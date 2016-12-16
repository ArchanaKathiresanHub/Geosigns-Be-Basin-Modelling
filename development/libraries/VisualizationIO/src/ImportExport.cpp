//
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "ImportExport.h"
#include "VisualizationIO_native.h"
#include "VisualizationUtils.h"
#include "DataStore.h"
#include "FilePath.h"
#include "FolderPath.h"

#include <boost/foreach.hpp>
#include <boost/thread.hpp>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <cstring>
#include "hdf5.h"

using namespace CauldronIO;

bool ImportExport::exportToXML(std::shared_ptr<Project>& project, const std::shared_ptr<Project>& projectExisting,
	const std::string& absPath, size_t numThreads, bool center)
{
   // Create empty property tree object
   ibs::FilePath outputPath(absPath);
   ibs::FilePath folderPath = outputPath.filePath();
   std::string filename = outputPath.fileName();
   std::string filenameNoExtension = outputPath.fileNameNoExtension();
   filenameNoExtension += "_vizIO_output";
   folderPath << filenameNoExtension;
      
   // Create output directory if not existing
    if (!folderPath.exists())
    {
        ibs::FolderPath(folderPath.path()).create();
    }

    pugi::xml_document doc;
    pugi::xml_node pt = doc.append_child("project");

    ImportExport newExport(outputPath.filePath(), filenameNoExtension, numThreads, center);

    // Create xml property tree and write datastores
    newExport.addProject(pt, project, projectExisting);

    // Write property tree to XML file
    ibs::FilePath xmlFileName(outputPath.filePath());
    xmlFileName << outputPath.fileNameNoExtension() + ".xml";

    return doc.save_file(xmlFileName.cpath());
}

void ImportExport::addProject(pugi::xml_node pt, std::shared_ptr<Project>& project, const std::shared_ptr<Project>& projectExisting)
{
    ibs::FilePath fullPath(m_absPath);
    fullPath << m_relPath.path();
    m_project = project;
	m_projectExisting = projectExisting;

    // Add general project description
    pt.append_child("name").text() = project->getName().c_str();
    pt.append_child("description").text() = project->getDescription().c_str();
    pt.append_child("modelingmode").text() = (int)project->getModelingMode();
    pt.append_child("team").text() = project->getTeam().c_str();
    pt.append_child("programversion").text() = project->getProgramVersion().c_str();
	pt.append_child("outputpath").text() = m_relPath.cpath();
    
    pugi::xml_node ptxml = pt.append_child("xml-version");
    ptxml.append_attribute("major") = xml_version_major;
    ptxml.append_attribute("minor") = xml_version_minor;

    // Write all formations
    pugi::xml_node formationNode = pt.append_child("formations");
    BOOST_FOREACH(const std::shared_ptr<const Formation>& formation, project->getFormations())
    {
        addFormation(formationNode, formation);
    }

    // Write all properties
    pugi::xml_node propertyNode = pt.append_child("properties");
    BOOST_FOREACH(const std::shared_ptr<const Property>& property, project->getProperties())
    {
        addProperty(propertyNode, property);
    }

    // Write all reservoirs
    if (project->getReservoirs().size() > 0)
    {
        pugi::xml_node reservoirNodes = pt.append_child("reservoirs");
        BOOST_FOREACH(const std::shared_ptr<const Reservoir>& reservoir, project->getReservoirs())
        {
            pugi::xml_node reservoirNode = reservoirNodes.append_child("reservoir");
            reservoirNode.append_attribute("name") = reservoir->getName().c_str();
            reservoirNode.append_attribute("formation") = reservoir->getFormation()->getName().c_str();
        }
    }

    // Write all snapshots
    const SnapShotList snapShotList = project->getSnapShots();
    m_append = detectAppend(project);
    pugi::xml_node snapShotNodes = pt.append_child("snapshots");

    BOOST_FOREACH(const std::shared_ptr<SnapShot>& snapShot, snapShotList)
    {
        pugi::xml_node node = snapShotNodes.append_child("snapshot");
        addSnapShot(snapShot, fullPath, node);
    }

    // Write all geometries
    if (project->getGeometries().size() > 0)
    {
        pugi::xml_node geometryNode = pt.append_child("geometries");
        BOOST_FOREACH(const std::shared_ptr<const Geometry2D>& geometry, project->getGeometries())
        {
            addGeometryInfo2D(geometryNode, geometry);
        }
    }
}

CauldronIO::ImportExport::ImportExport(const ibs::FilePath& absPath, const ibs::FilePath& relPath, size_t numThreads, bool center) 
    : m_absPath(absPath), m_relPath(relPath), m_numThreads(numThreads), m_center(center)
{
}

void CauldronIO::ImportExport::addProperty(pugi::xml_node node, const std::shared_ptr<const Property>& property) const
{
    pugi::xml_node propNode = node.append_child("property");
    propNode.append_attribute("name") = property->getName().c_str();
    propNode.append_attribute("cauldronname") = property->getCauldronName().c_str();
    propNode.append_attribute("username") = property->getUserName().c_str();
    propNode.append_attribute("unit") = property->getUnit().c_str();
    propNode.append_attribute("attribute") = property->getAttribute();
    propNode.append_attribute("type") = property->getType();
}

void CauldronIO::ImportExport::addFormation(pugi::xml_node node, const std::shared_ptr<const Formation>& formation) const
{
    pugi::xml_node subNode = node.append_child("formation");
    subNode.append_attribute("name") = formation->getName().c_str();
    unsigned int start, end;
    formation->getK_Range(start, end);
    
    subNode.append_attribute("kstart") = start;
    subNode.append_attribute("kend") = end;
    subNode.append_attribute("isSR") = formation->isSourceRock();
    subNode.append_attribute("isML") =formation->isMobileLayer();
}

void CauldronIO::ImportExport::addSurface(DataStoreSave& dataStore, const std::shared_ptr<Surface>& surfaceIO, pugi::xml_node ptree)
{
    // Retrieve data if necessary: if the getDataStoreParams is unequal to zero this means data is saved and does not need to be saved again
    if (!surfaceIO->isRetrieved() && !m_append)
        surfaceIO->retrieve();

    // Write general info
    ptree.append_attribute("name") = surfaceIO->getName().c_str();
    ptree.append_attribute("subsurfacekind") = surfaceIO->getSubSurfaceKind();
    if (surfaceIO->getTopFormation())
        ptree.append_attribute("top-formation") = surfaceIO->getTopFormation()->getName().c_str();
    if (surfaceIO->getBottomFormation())
        ptree.append_attribute("bottom-formation") = surfaceIO->getBottomFormation()->getName().c_str();

    // Iterate over all contained valuemaps
    const PropertySurfaceDataList valueMaps = surfaceIO->getPropertySurfaceDataList();

    if (valueMaps.size() > 0)
    {
        pugi::xml_node valueMapsNode = ptree.append_child("propertymaps");
        BOOST_FOREACH(const PropertySurfaceData& propertySurfaceData, valueMaps)
        {
            pugi::xml_node node = valueMapsNode.append_child("propertymap");
            node.append_attribute("property") = propertySurfaceData.first->getName().c_str();

            const std::shared_ptr<SurfaceData>& surfaceData = propertySurfaceData.second;
            if (surfaceData->getFormation())
                node.append_attribute("formation") = surfaceData->getFormation()->getName().c_str();
            if (surfaceData->getReservoir())
                node.append_attribute("reservoir") = surfaceData->getReservoir()->getName().c_str();

            // Write the geometry
            node.append_attribute("geom-index") = (int)m_project->getGeometryIndex(surfaceData->getGeometry());

            // Min/max values
            node.append_attribute("min") = surfaceData->getMinValue();
            node.append_attribute("max") = surfaceData->getMaxValue();

			// Check for reference volume
			ReferenceMap* refMap = dynamic_cast<ReferenceMap*>(surfaceData.get());

			if (surfaceData->isConstant())
			{
				node.append_attribute("constantvalue") = surfaceData->getConstantValue();
			}
			else if (refMap != nullptr)
			{
				addReferenceData(node, refMap->getDataStoreParams(), false, false);
			}
			else
			{
				dataStore.addSurface(surfaceData, node);
			}
        }
    }
}

void CauldronIO::ImportExport::addVolume(DataStoreSave& dataStore, const std::shared_ptr<Volume>& volume, pugi::xml_node volNode)
{
    if (!volume->isRetrieved() && !m_append)
        volume->retrieve();

    volNode.append_attribute("subsurfacekind") = volume->getSubSurfaceKind();

    if (volume->getPropertyVolumeDataList().size() > 0)
    {
       
        pugi::xml_node propVolNodes = volNode.append_child("propertyvols");
        BOOST_FOREACH(const PropertyVolumeData& propVolume, volume->getPropertyVolumeDataList())
        {
            const std::shared_ptr<const Property>& prop = propVolume.first;
            const std::shared_ptr<VolumeData>& data = propVolume.second;
            const std::shared_ptr<Geometry3D>& thisGeometry = data->getGeometry();

            pugi::xml_node node = propVolNodes.append_child("propertyvol");
            node.append_attribute("property") = prop->getName().c_str();
            node.append_attribute("firstK") = (unsigned int)thisGeometry->getFirstK();
            node.append_attribute("numK") = (unsigned int)thisGeometry->getNumK();

            // Write the geometry
            node.append_attribute("geom-index") = (int)m_project->getGeometryIndex(thisGeometry);

            size_t numBytes = thisGeometry->getNumI()*thisGeometry->getNumJ()*thisGeometry->getNumK()*sizeof(float);

            // Min/max values
            node.append_attribute("min") = data->getMinValue();
            node.append_attribute("max") = data->getMaxValue();

			// Check for reference volume
			ReferenceVolume* refVolume = dynamic_cast<ReferenceVolume*>(data.get());

            if (data->isConstant())
            {
                node.append_attribute("constantvalue") = data->getConstantValue();
            }
			else if (refVolume) //TODO: relative path of existing project needs to be added to filename
			{
				if (refVolume->getDataStoreParamsIJK())
				{
					const DataStoreParams* params = refVolume->getDataStoreParamsIJK();
					addReferenceData(node, params, true, false);
				}
				if (refVolume->getDataStoreParamsKIJ())
				{
					const DataStoreParams* params = refVolume->getDataStoreParamsKIJ();
					addReferenceData(node, params, false, true);
				}
			}
			else
			{
				dataStore.addVolume(data, node, numBytes);
			}
        }
    }
}

void CauldronIO::ImportExport::addReferenceData(pugi::xml_node &node, const DataStoreParams* params, bool dataIJK, bool dataKIJ) const
{
	pugi::xml_node subNode = node.append_child("datastore");
	subNode.append_attribute("file") = params->fileName.path().c_str();

	if (params->compressed)
		subNode.append_attribute("compression") = params->compressed_lz4 ? "lz4" : "gzip";
	else
		subNode.append_attribute("compression") = "none";

	subNode.append_attribute("partialpath") = false;
	
	if (dataIJK || dataKIJ)
	{
		subNode.append_attribute("dataIJK") = dataIJK;
	}
	subNode.append_attribute("size") = (unsigned int)params->size;
	subNode.append_attribute("offset") = (unsigned int)params->offset;
}

void CauldronIO::ImportExport::addGeometryInfo2D(pugi::xml_node node, const std::shared_ptr<const Geometry2D>& geometry) const
{
    pugi::xml_node subNode = node.append_child("geometry");
    subNode.append_attribute("numI"  ) = (unsigned int)geometry->getNumI();
    subNode.append_attribute("numJ"  ) = (unsigned int)geometry->getNumJ();
    subNode.append_attribute("minI"  ) = geometry->getMinI();
    subNode.append_attribute("minJ"  ) = geometry->getMinJ();
    subNode.append_attribute("deltaI") = geometry->getDeltaI();
    subNode.append_attribute("deltaJ") = geometry->getDeltaJ();

    if (geometry->isCellCentered())
        subNode.append_attribute("cell-centered") = true;
}

void CauldronIO::ImportExport::addSnapShot(const std::shared_ptr<SnapShot>& snapShot, ibs::FilePath fullPath, pugi::xml_node node)
{
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(6);
    ss << snapShot->getAge();
    std::string snapshotString = ss.str();
    std::cout << "Writing snapshot Age=" << snapshotString << std::endl;

    ibs::FilePath volumeStorePath(fullPath);
    volumeStorePath << "Snapshot_" + snapshotString + "_volumes.cldrn";
    DataStoreSave volumeStore(volumeStorePath.path(), m_append);

    ibs::FilePath surfaceStorePath(fullPath);
    surfaceStorePath << "Snapshot_" + snapshotString + "_surfaces.cldrn";
    DataStoreSave surfaceDataStore(surfaceStorePath.path(), m_append);

    node.append_attribute("age") = snapShot->getAge();
    node.append_attribute("kind") = snapShot->getKind();
    node.append_attribute("isminor") = snapShot->isMinorShapshot();

	// If there is an existing project
	if (m_projectExisting)
	{
		VisualizationUtils::replaceExistingProperties(snapShot, m_projectExisting);
	}

    // Read all data into memory: if there is an existing project, this is too late...
    CauldronIO::VisualizationUtils::retrieveAllData(snapShot, m_numThreads);
    
    // Cell center data if necessary
    if (m_center)
    {
        CauldronIO::VisualizationUtils::cellCenterAllMaps(snapShot, m_project);
        CauldronIO::VisualizationUtils::cellCenterVolume(snapShot->getVolume(), m_project);
        
		// This should be the slowest operation: parallellize
		CauldronIO::VisualizationUtils::cellCenterFormationVolumes(snapShot, m_project, m_numThreads);
    }

    // Add surfaces
    //////////////////////////////////////////////////////////////////////////
    const SurfaceList surfaces = snapShot->getSurfaceList();
    if (surfaces.size() > 0)
    {
        pugi::xml_node surfacesNode = node.append_child("surfaces");
        BOOST_FOREACH(const std::shared_ptr<Surface>& surfaceIO, surfaces)
        {
            // General properties
            pugi::xml_node surfaceNode = surfacesNode.append_child("surface");

            // Data storage
            addSurface(surfaceDataStore, surfaceIO, surfaceNode);
        }
    }

    // Add the continuous volume
    //////////////////////////////////////////////////////////////////////////
    const std::shared_ptr<Volume> volume = snapShot->getVolume();
    if (volume)
    {
        pugi::xml_node volNode = node.append_child("volume");
        addVolume(volumeStore, volume, volNode);
    }

    // Add a volume per formation, with discontinuous properties
    //////////////////////////////////////////////////////////////////////////
    FormationVolumeList formVolumes = snapShot->getFormationVolumeList();
    if (formVolumes.size() > 0)
    {
        pugi::xml_node formVolumesNode = node.append_child("formvols");
        BOOST_FOREACH(FormationVolume& formVolume, formVolumes)
        {
            // Only add a volume if it contains something
            if (formVolume.second->getPropertyVolumeDataList().size() > 0)
            {
                // General properties
                pugi::xml_node volNode = formVolumesNode.append_child("formvol");

                const std::shared_ptr<Volume> subVolume = formVolume.second;
                const std::shared_ptr<const Formation> subFormation = formVolume.first;

                // Add formation name
                volNode.append_attribute("formation") = subFormation->getName().c_str();

                // Add volume 
                pugi::xml_node subvolNode = volNode.append_child("volume");
                addVolume(volumeStore, subVolume, subvolNode);
            }
        }
    }

    // Compress all data
    ////////////////////////////////

    // Collect all data
    std::vector<std::shared_ptr<DataToCompress> > allData;
    for (int i = 0; i < surfaceDataStore.getDataToCompressList().size(); i++)
        allData.push_back(surfaceDataStore.getDataToCompressList().at(i));
    for (int i = 0; i < volumeStore.getDataToCompressList().size(); i++)
        allData.push_back(volumeStore.getDataToCompressList().at(i));

    boost::lockfree::queue<int> queue(1024);
    boost::thread_group threads;

    // Add to queue
    for (int i = 0; i < allData.size(); i++)
        queue.push(i);

    // Compress it in separate threads
    for (int i = 0; i < m_numThreads; ++i)
        threads.add_thread(new boost::thread(CauldronIO::ImportExport::compressDataQueue, allData, &queue));
    threads.join_all();

    surfaceDataStore.flush();
    volumeStore.flush();

    // Add trappers
    /////////////////////////////
    const TrapperList trappers = snapShot->getTrapperList();
    if (trappers.size() > 0)
    {
        pugi::xml_node trappersNode = node.append_child("trappers");

        int maxPersistentTrapperID = -1;
        BOOST_FOREACH(const std::shared_ptr<const Trapper>& trapper, trappers)
        {
            // General properties
            pugi::xml_node trapperNode = trappersNode.append_child("trapper");

            trapperNode.append_attribute("id") = trapper->getID();
            trapperNode.append_attribute("persistentID") = trapper->getPersistentID();
            trapperNode.append_attribute("reservoirname") = trapper->getReservoirName().c_str();
            trapperNode.append_attribute("depth") = trapper->getDepth();
            trapperNode.append_attribute("spillDepth") = trapper->getSpillDepth();

            float x, y;
            trapper->getPosition(x, y);
            trapperNode.append_attribute("posX") = x;
            trapperNode.append_attribute("posY") = y;

            trapper->getSpillPointPosition(x, y);
            trapperNode.append_attribute("spillPosX") = x;
            trapperNode.append_attribute("spillPosY") = y;

            trapperNode.append_attribute("goc") = trapper->getGOC();
            trapperNode.append_attribute("owc") = trapper->getOWC();

            int downstreamTrapperID = trapper->getDownStreamTrapperID();
            trapperNode.append_attribute("downstreamtrapper") = downstreamTrapperID;
            maxPersistentTrapperID = std::max(maxPersistentTrapperID, trapper->getPersistentID());
        }

        if (maxPersistentTrapperID != -1)
            trappersNode.append_child("maxPersistentTrapperID").text() = maxPersistentTrapperID;
    }

    // Release all data
    snapShot->release();
}

void CauldronIO::ImportExport::compressDataQueue(std::vector< std::shared_ptr < DataToCompress > > allData, boost::lockfree::queue<int>* queue)
{
    int value;
    while (queue->pop(value))
    {
        std::shared_ptr<DataToCompress> data = allData.at(value);
        data->compress();
    }
}

// If the objects are 'native' implementation, we should append the output files, otherwise
// we should start from scratch
bool CauldronIO::ImportExport::detectAppend(std::shared_ptr<Project>& project)
{
    const SnapShotList snapShotList = project->getSnapShots();
    BOOST_FOREACH(std::shared_ptr<const SnapShot> snapShot, snapShotList)
    {
        const SurfaceList surfaces = snapShot->getSurfaceList();
        BOOST_FOREACH(const std::shared_ptr<Surface>& surfaceIO, surfaces)
        {
            if (dynamic_cast<MapNative*>(surfaceIO->getPropertySurfaceDataList().at(0).second.get()) != NULL) return true;
            return false;
        }

        const std::shared_ptr<Volume>& volume = snapShot->getVolume();
		if (volume)
		{
			BOOST_FOREACH(const PropertyVolumeData& volumeData, volume->getPropertyVolumeDataList())
			{
				if (dynamic_cast<VolumeDataNative*>(volumeData.second.get()) != NULL) return true;
				return false;
			}
		}
    }

    // This should not happen
    return false;
}

//////////////////////////////////////////////////////////////////////////
/// Importing from native format
//////////////////////////////////////////////////////////////////////////

std::shared_ptr<Project> CauldronIO::ImportExport::importFromXML(const std::string& filename)
{
    if (!ibs::FilePath(filename).exists())
        throw CauldronIOException("Cannot open file");
    
    pugi::xml_document doc;
    std::cout << "Reading XML into pugiXML DOM" << std::endl;
    pugi::xml_parse_result result = doc.load_file(filename.c_str());

    if (!result)
        throw CauldronIOException("Error during parsing xml file");

        
    ImportExport importExport(ibs::FilePath(filename).filePath(), ibs::FilePath(""), 1, false);
    std::shared_ptr<Project> project;
    
    try
    {
        project = importExport.getProject(doc);
    }
    catch (CauldronIOException& excp)
    {
        // Just rethrow it
        throw excp;
    }
    catch (...)
    {
        throw CauldronIOException("unspecified error during xml parse");
    }
    
    return project;
}

std::string CauldronIO::ImportExport::getXMLIndexingFileName(const std::string& project3Dfilename)
{
    ibs::FilePath path(project3Dfilename);
    return path.fileNameNoExtension() + ".xml";
}

std::shared_ptr<Project> CauldronIO::ImportExport::getProject(const pugi::xml_document& ptDoc)
{
    pugi::xml_node pt = ptDoc.child("project");
    if (!pt) 
        throw CauldronIOException("Invalid xml file or format");

    std::string projectName = pt.child_value("name");
    std::string projectDescript = pt.child_value("description");
    std::string projectTeam = pt.child_value("team");
    std::string projectVersion = pt.child_value("programversion");
	std::string outputPath = pt.child_value("outputpath");
    
	ModellingMode mode = (ModellingMode)pt.child("modelingmode").text().as_int();

    // Check XML versions
    // dataXmlVersion > xml_version : actual data generated with code newer than this code, requires forward compatibility
    // dataXmlVersion < xml_version : actual data generated with code older than this code, requires backward compatibility
    //////////////////////////////////////////////////////////////////////////

    int dataXmlVersionMinor = 0;
    int dataXmlVersionMajor = 0;
    pugi::xml_node xmlOptional = pt.child("xml-version");
    if (xmlOptional)
    {
        dataXmlVersionMinor = xmlOptional.attribute("minor").as_int();
        dataXmlVersionMajor = xmlOptional.attribute("major").as_int();
    }

    bool forwardCompatible = dataXmlVersionMajor > xml_version_major || (dataXmlVersionMajor == xml_version_major && dataXmlVersionMinor > xml_version_minor);
    bool backwardCompatible = dataXmlVersionMajor < xml_version_major || (dataXmlVersionMajor == xml_version_major && dataXmlVersionMinor < xml_version_minor);

    if (forwardCompatible)
        throw CauldronIOException("Xml format not forward compatible");
    if (backwardCompatible)
        throw CauldronIOException("Xml format not backward compatible"); // we should try to fix that when the time comes

    ibs::FilePath fullOutputPath(m_absPath);
	if (m_absPath.path() != ".")
	{
		fullOutputPath << outputPath;
	}
	else
	{
		fullOutputPath = ibs::FilePath(outputPath);
	}

    // Create the project
    m_project.reset(new Project(projectName, projectDescript, projectTeam, projectVersion, mode, dataXmlVersionMajor, dataXmlVersionMinor));

    // Read all formations
    pugi::xml_node formationsNode = pt.child("formations");
    for (pugi::xml_node formationNode = formationsNode.child("formation"); formationNode; formationNode = formationNode.next_sibling("formation"))
    {
        std::shared_ptr<const Formation> formation = getFormation(formationNode);
        m_project->addFormation(formation);
    }

    // Read all properties
    pugi::xml_node propertiesNode = pt.child("properties");
    for (pugi::xml_node propertyNode = propertiesNode.child("property"); propertyNode; propertyNode = propertyNode.next_sibling("property"))
    {
        std::shared_ptr<const Property> property = getProperty(propertyNode);
        m_project->addProperty(property);
    }

    // Read all reservoirs
    pugi::xml_node hasReservoirs = pt.child("reservoirs");
    if (hasReservoirs)
    {
        for (pugi::xml_node reservoirNode = hasReservoirs.child("reservoir"); reservoirNode; reservoirNode = reservoirNode.next_sibling("reservoir"))
        {
            std::shared_ptr<const Reservoir> reservoir = getReservoir(reservoirNode);
            m_project->addReservoir(reservoir);
        }
    }

    // Read all geometries
    pugi::xml_node geometriesNode = pt.child("geometries");
    if (!geometriesNode)
    {
        throw CauldronIOException("Cannot find any geometries, please reconvert your project to reflect latest xml format");
    }
    for (pugi::xml_node geometryNode = geometriesNode.child("geometry"); geometryNode; geometryNode = geometryNode.next_sibling("geometry"))
    {
        std::shared_ptr<const Geometry2D> geometry = getGeometry2D(geometryNode);
        m_project->addGeometry(geometry);
    }
    
    // Read all snapshots
    pugi::xml_node snapshotsNode = pt.child("snapshots");
    for (pugi::xml_node snapShotNode = snapshotsNode.child("snapshot"); snapShotNode; snapShotNode = snapShotNode.next_sibling("snapshot"))
    {
        double age = snapShotNode.attribute("age").as_double();
        SnapShotKind kind = (SnapShotKind)snapShotNode.attribute("kind").as_int();
        bool isminor = snapShotNode.attribute("isminor").as_bool();

        // Create the snapshot
        std::shared_ptr<SnapShot> snapShot(new SnapShot(age, kind, isminor));

        // Find all surfaces
        /////////////////////////////////////////
        pugi::xml_node hasSurfaces = snapShotNode.child("surfaces");
        if (hasSurfaces)
        {
            for (pugi::xml_node surfaceNode = hasSurfaces.child("surface"); surfaceNode; surfaceNode = surfaceNode.next_sibling("surface"))
            {
                // Read some xml attributes
                std::string surfaceName = surfaceNode.attribute("name").value();
                SubsurfaceKind surfaceKind = (SubsurfaceKind)surfaceNode.attribute("subsurfacekind").as_int();

                // Find formations, if present
                std::shared_ptr<const Formation> topFormationIO;
                pugi::xml_attribute topFormationName = surfaceNode.attribute("top-formation");
                if (topFormationName)
                    topFormationIO = m_project->findFormation(topFormationName.value());
                std::shared_ptr<const Formation> bottomFormationIO;
                pugi::xml_attribute bottomFormationName = surfaceNode.attribute("bottom-formation");
                if (bottomFormationName)
                    bottomFormationIO = m_project->findFormation(bottomFormationName.value());

                // Construct the surface
                std::shared_ptr<Surface> surface(new Surface(surfaceName, surfaceKind)); 
                if (topFormationIO) surface->setFormation(topFormationIO, true);
                if (bottomFormationIO) surface->setFormation(bottomFormationIO, false);

                // Get all property surface data
                pugi::xml_node propertyMapNodes = surfaceNode.child("propertymaps");
                for (pugi::xml_node propertyMapNode = propertyMapNodes.child("propertymap"); propertyMapNode; propertyMapNode = propertyMapNode.next_sibling("propertymap"))
                {
                    std::string propertyName = propertyMapNode.attribute("property").value();
                    std::shared_ptr<const Property> property = m_project->findProperty(propertyName);
                    assert(property);

                    // Get the geometry
                    size_t geometryIndex = (size_t)propertyMapNode.attribute("geom-index").as_int();
                    std::shared_ptr<const Geometry2D> geometry = m_project->getGeometries().at(geometryIndex);

                    // Get min/max value
                    float minValue = DefaultUndefinedValue;
                    float maxValue = DefaultUndefinedValue;

                    pugi::xml_attribute minValueAttr = propertyMapNode.attribute("min");
                    if (minValueAttr)
                        minValue = minValueAttr.as_float();

                    pugi::xml_attribute maxValueAttr = propertyMapNode.attribute("max");
                    if (maxValueAttr)
                        maxValue = maxValueAttr.as_float();

                    // Create the surface data
                    std::shared_ptr<SurfaceData> surfaceData(new MapNative(geometry, minValue, maxValue));

                    // Contains formation?
                    pugi::xml_attribute formationName = propertyMapNode.attribute("formation");
                    if (formationName)
                    {
                        std::shared_ptr<const Formation> formationIO = m_project->findFormation(formationName.value());
                        surfaceData->setFormation(formationIO);
                    }

                    // Find the reservoir object, if name is present
                    std::shared_ptr<const Reservoir> reservoirIO;
                    pugi::xml_attribute reservoirName = propertyMapNode.attribute("reservoir");
                    if (reservoirName)
                    {
                        reservoirIO = m_project->findReservoir(reservoirName.value());
                        assert(reservoirIO);
                        surfaceData->setReservoir(reservoirIO);
                    }

                    // Get the datastore xml node or constantvalue
                    pugi::xml_attribute constantVal = propertyMapNode.attribute("constantvalue");
                    if (constantVal)
                        surfaceData->setConstantValue(constantVal.as_float());
                    else
                        DataStoreLoad::getSurface(propertyMapNode, surfaceData, fullOutputPath);

                    PropertySurfaceData propSurfaceData(property, surfaceData);
                    surface->addPropertySurfaceData(propSurfaceData);
                }

                // Add to snapshot
                snapShot->addSurface(surface);
            }
        }
        
        // Find all (continuous) volumes
        /////////////////////////////////////////
        pugi::xml_node hasVolumes = snapShotNode.child("volume");
        if (hasVolumes)
        {
            std::shared_ptr<Volume> volume = getVolume(hasVolumes, fullOutputPath);
            snapShot->setVolume(volume);
        }

        // Get formation volumes
        //////////////////////////////////////////////////////////////////////////
        pugi::xml_node hasFormationVolumes = snapShotNode.child("formvols");

        // Get all property volume data
        if (hasFormationVolumes)
        {
            for (pugi::xml_node formVolNode = hasFormationVolumes.child("formvol"); formVolNode; formVolNode = formVolNode.next_sibling("formvol"))
            {
                std::string formationName = formVolNode.attribute("formation").value();
                std::shared_ptr<const Formation> formationIO = m_project->findFormation(formationName);
                assert(formationIO);

                // There should be a volume
                pugi::xml_node volumeNode = formVolNode.child("volume");
                std::shared_ptr<Volume> volume = getVolume(volumeNode, fullOutputPath);

                // Add it to the list
                FormationVolume formVolume(formationIO, volume);
                snapShot->addFormationVolume(formVolume);
            }
        }

        // Get trappers
        //////////////////////////////////////////////////////////////////////////

        pugi::xml_node hasTrappers = snapShotNode.child("trappers");
        if (hasTrappers)
        {
            int maxPersistentTrapperID = hasTrappers.child("maxPersistentTrapperID").text().as_int();
            assert(maxPersistentTrapperID > -1);
                
            std::vector<std::shared_ptr<Trapper>* > persistentIDs(maxPersistentTrapperID + 1);

            for (pugi::xml_node trapperNode = hasTrappers.child("trapper"); trapperNode; trapperNode = trapperNode.next_sibling("trapper"))
            {
                int ID = trapperNode.attribute("id").as_int();
                int persistentID = trapperNode.attribute("persistentID").as_int(); 
                int downstreamTrapperID = trapperNode.attribute("downstreamtrapper").as_int(); 
                float depth = trapperNode.attribute("depth").as_float();
                float spillDepth = trapperNode.attribute("spillDepth").as_float(); 
                std::string reservoirname = trapperNode.attribute("reservoirname").value();
                        
                float x      = trapperNode.attribute("posX").as_float();
                float y      = trapperNode.attribute("posY").as_float();
                float spillX = trapperNode.attribute("spillPosX").as_float();
                float spillY = trapperNode.attribute("spillPosY").as_float();
                float goc    = trapperNode.attribute("goc").as_float();
                float owc    = trapperNode.attribute("owc").as_float();

                std::shared_ptr<Trapper> trapperIO(new Trapper(ID, persistentID));
                trapperIO->setDownStreamTrapperID(downstreamTrapperID);
                trapperIO->setReservoirName(reservoirname);
                trapperIO->setSpillDepth(spillDepth);
                trapperIO->setSpillPointPosition(spillX, spillY);
                trapperIO->setDepth(depth);
                trapperIO->setPosition(x, y);
                trapperIO->setOWC(owc);
                trapperIO->setGOC(goc);

                snapShot->addTrapper(trapperIO);

                assert(persistentID <= maxPersistentTrapperID);
                persistentIDs[persistentID] = &trapperIO;
            }

            // Assign downstream trappers
            const TrapperList& trappers = snapShot->getTrapperList();
            BOOST_FOREACH(const std::shared_ptr<const Trapper>& trapper, trappers)
            {
                int downstreamTrapperID = trapper->getDownStreamTrapperID();
                if (downstreamTrapperID > -1)
                {
                    std::shared_ptr<Trapper> downstreamTrapper = *persistentIDs[downstreamTrapperID];
                    std::shared_ptr<Trapper> thisTrapper = *persistentIDs[trapper->getPersistentID()];
                    thisTrapper->setDownStreamTrapper(downstreamTrapper);
                }
            }
        }

        m_project->addSnapShot(snapShot);
    }
    
    return m_project;
}

std::shared_ptr<const Reservoir> CauldronIO::ImportExport::getReservoir(pugi::xml_node reservoirNode) const
{
    std::shared_ptr<Reservoir> reservoir;

    std::string name = reservoirNode.attribute("name").value();
    std::string formation = reservoirNode.attribute("formation").value(); 

    std::shared_ptr<const Formation> formationIO = m_project->findFormation(formation);
    assert(formationIO);

    reservoir.reset(new Reservoir(name, formationIO));
    return reservoir;
}

std::shared_ptr<const Geometry2D> CauldronIO::ImportExport::getGeometry2D(pugi::xml_node geometryNode) const
{
    std::shared_ptr<const Geometry2D> geometry;

    double minI, minJ, deltaI, deltaJ;
    size_t numI, numJ;

    numI = (size_t)geometryNode.attribute("numI").as_uint();
    numJ = (size_t)geometryNode.attribute("numJ").as_uint();
    minI =   geometryNode.attribute("minI").as_double();
    minJ =   geometryNode.attribute("minJ").as_double();
    deltaI = geometryNode.attribute("deltaI").as_double();
    deltaJ = geometryNode.attribute("deltaJ").as_double();

    // Cell-centered?
    bool cellCentered = false;
    pugi::xml_attribute isCellCenteredNode = geometryNode.attribute("cell-centered");
    if (isCellCenteredNode)
    {
        cellCentered = isCellCenteredNode.as_bool();
    }

    return std::shared_ptr<const Geometry2D>(new Geometry2D(numI, numJ, deltaI, deltaJ, minI, minJ, cellCentered));
}

std::shared_ptr<Volume> CauldronIO::ImportExport::getVolume(pugi::xml_node volumeNode, const ibs::FilePath& path)
{
    SubsurfaceKind surfaceKind = (SubsurfaceKind)volumeNode.attribute("subsurfacekind").as_int();

    // Create the volume
    std::shared_ptr<Volume> volume(new Volume(surfaceKind));

    // Get all property volume data
    pugi::xml_node propertyVolNodes = volumeNode.child("propertyvols");
    assert(propertyVolNodes);

    for (pugi::xml_node propertyVolNode = propertyVolNodes.child("propertyvol"); propertyVolNode; propertyVolNode = propertyVolNode.next_sibling("propertyvol"))
    {
        std::string propertyName = propertyVolNode.attribute("property").value();
        std::shared_ptr<const Property> property = m_project->findProperty(propertyName);
        assert(property);

        // Get the geometry
        size_t geometryIndex = (size_t)propertyVolNode.attribute("geom-index").as_int();
        std::shared_ptr<const Geometry2D> geometry = m_project->getGeometries().at(geometryIndex);

        // Get K range
        pugi::xml_attribute firstK_attrib = propertyVolNode.attribute("firstK");
        if (!firstK_attrib) throw CauldronIOException("Cannot find firstK attribute in property-volume");
        size_t firstK = firstK_attrib.as_uint();
        pugi::xml_attribute numK_attrib = propertyVolNode.attribute("numK");
        if (!numK_attrib) throw CauldronIOException("Cannot find numK attribute in property-volume");
        size_t numK = numK_attrib.as_uint();

        // Get min/max value
        float minValue = DefaultUndefinedValue;
        float maxValue = DefaultUndefinedValue;

        pugi::xml_attribute minValueAttr = propertyVolNode.attribute("min");
        if (minValueAttr)
            minValue = minValueAttr.as_float();

        pugi::xml_attribute maxValueAttr = propertyVolNode.attribute("max");
        if (maxValueAttr)
            maxValue = maxValueAttr.as_float();

        // Create the VolumeData
        std::shared_ptr<Geometry3D> geometry3D(new Geometry3D(geometry->getNumI(), geometry->getNumJ(), numK, firstK, geometry->getDeltaI(),
            geometry->getDeltaJ(), geometry->getMinI(), geometry->getMinJ()));
        std::shared_ptr<VolumeData> volData(new VolumeDataNative(geometry3D, minValue, maxValue));

        // Get the datastore xml node or constantvalue
        pugi::xml_attribute constantVal = propertyVolNode.attribute("constantvalue");
        if (constantVal)
            volData->setConstantValue(constantVal.as_float());
        else
            DataStoreLoad::getVolume(propertyVolNode, volData, path);

        PropertyVolumeData propVolData(property, volData);
        volume->addPropertyVolumeData(propVolData);
    }
    
    return volume;
}

std::shared_ptr<Property> CauldronIO::ImportExport::getProperty(pugi::xml_node propertyNode) const
{
    std::shared_ptr<Property> property;

    std::string name = propertyNode.attribute("name").value();
    std::string cauldronname = propertyNode.attribute("cauldronname").value(); 
    std::string username = propertyNode.attribute("username").value(); 
    std::string unit = propertyNode.attribute("unit").value();  
    PropertyType type = (PropertyType)propertyNode.attribute("type").as_int();
    PropertyAttribute attrib = (PropertyAttribute)propertyNode.attribute("attribute").as_int();

    property.reset(new Property(name, username, cauldronname, unit, type, attrib));
    return property;
}

std::shared_ptr<Formation> CauldronIO::ImportExport::getFormation(pugi::xml_node formationNode) const
{
    std::shared_ptr<Formation> formation;

    unsigned int start, end;
    std::string name = formationNode.attribute("name").value();
    start = formationNode.attribute("kstart").as_uint();
    end = formationNode.attribute("kend").as_uint();
    bool isSR = formationNode.attribute("isSR").as_bool();
    bool isML = formationNode.attribute("isML").as_bool(); 

    formation.reset(new Formation(start, end, name, isSR, isML));
    return formation;
}