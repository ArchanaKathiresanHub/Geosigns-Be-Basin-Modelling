//
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "ExportToXML.h"
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

using namespace CauldronIO;

bool ExportToXML::exportToXML(std::shared_ptr<Project>& project, const std::shared_ptr<Project>& projectExisting,
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

	ExportToXML newExport(outputPath.filePath(), filenameNoExtension, numThreads, center);

    // Create xml property tree and write datastores
    newExport.addProject(pt, project, projectExisting);

    // Write property tree to XML file
    ibs::FilePath xmlFileName(outputPath.filePath());
    xmlFileName << outputPath.fileNameNoExtension() + ".xml";

    return doc.save_file(xmlFileName.cpath());
}

void ExportToXML::addProject(pugi::xml_node pt, std::shared_ptr<Project>& project, const std::shared_ptr<Project>& projectExisting)
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

CauldronIO::ExportToXML::ExportToXML(const ibs::FilePath& absPath, const ibs::FilePath& relPath, size_t numThreads, bool center)
    : m_absPath(absPath), m_relPath(relPath), m_numThreads(numThreads), m_center(center)
{
}

void CauldronIO::ExportToXML::addProperty(pugi::xml_node node, const std::shared_ptr<const Property>& property) const
{
    pugi::xml_node propNode = node.append_child("property");
    propNode.append_attribute("name") = property->getName().c_str();
    propNode.append_attribute("cauldronname") = property->getCauldronName().c_str();
    propNode.append_attribute("username") = property->getUserName().c_str();
    propNode.append_attribute("unit") = property->getUnit().c_str();
    propNode.append_attribute("attribute") = property->getAttribute();
    propNode.append_attribute("type") = property->getType();
}

void CauldronIO::ExportToXML::addFormation(pugi::xml_node node, const std::shared_ptr<const Formation>& formation) const
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

void CauldronIO::ExportToXML::addSurface(DataStoreSave& dataStore, const std::shared_ptr<Surface>& surfaceIO, pugi::xml_node ptree)
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

void CauldronIO::ExportToXML::addVolume(DataStoreSave& dataStore, const std::shared_ptr<Volume>& volume, pugi::xml_node volNode)
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

void CauldronIO::ExportToXML::addReferenceData(pugi::xml_node &node, const DataStoreParams* params, bool dataIJK, bool dataKIJ) const
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

void CauldronIO::ExportToXML::addGeometryInfo2D(pugi::xml_node node, const std::shared_ptr<const Geometry2D>& geometry) const
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

void CauldronIO::ExportToXML::addSnapShot(const std::shared_ptr<SnapShot>& snapShot, ibs::FilePath fullPath, pugi::xml_node node)
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
        threads.add_thread(new boost::thread(CauldronIO::ExportToXML::compressDataQueue, allData, &queue));
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

void CauldronIO::ExportToXML::compressDataQueue(std::vector< std::shared_ptr < DataToCompress > > allData, boost::lockfree::queue<int>* queue)
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
bool CauldronIO::ExportToXML::detectAppend(std::shared_ptr<Project>& project)
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
