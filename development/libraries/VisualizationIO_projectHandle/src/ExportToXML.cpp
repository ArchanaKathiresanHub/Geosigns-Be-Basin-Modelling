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

CauldronIO::ExportToXML::ExportToXML(const ibs::FilePath& absPath, const ibs::FilePath& relPath, size_t numThreads, bool center)
   : m_fullPath(absPath), m_relPath(relPath), m_numThreads(numThreads), m_center(center)
{
    if( absPath.path() == "." ) {
       m_fullPath = relPath.path();
    } else {
       m_fullPath << relPath.path(); 
    }
}

void ExportToXML::addProject(pugi::xml_node pt, std::shared_ptr<Project>& project, const std::shared_ptr<Project>& projectExisting)
{
   addProjectDescription(pt, project, projectExisting);
   addProjectData(pt, project, true);
}

void ExportToXML::addProjectDescription(pugi::xml_node pt, std::shared_ptr<Project>& project, const std::shared_ptr<Project>& projectExisting)
{
   m_project = project;
   
   // If there is an existing project, it is checked if data to be stored isn't already stored in the existing project
   // if true, when such data is encountered, we add references to the existing project and not duplicating the data
   m_projectExisting = projectExisting;
   
   // Add general project description
   pt.append_child("name").text() = project->getName().c_str();
   pt.append_child("description").text() = project->getDescription().c_str();
   pt.append_child("modelingmode").text() = (int)project->getModelingMode();
   pt.append_child("programversion").text() = project->getProgramVersion().c_str();
   pt.append_child("outputpath").text() = m_relPath.cpath();
   
   pugi::xml_node ptxml = pt.append_child("xml-version");
   ptxml.append_attribute("major") = xml_version_major;
   ptxml.append_attribute("minor") = xml_version_minor;

   // Append is true if we're exporting new (native) data to an existing vizIO project - 
   // this is detected by inspecting the offered project and checking if the implementation is native, and not coming from project3D
   // Note - this mode is unused afaik, and probably untested. The preferred way would be to have an existing project instead
   m_append = detectAppend(project);
}

void ExportToXML::addProjectData(pugi::xml_node pt, std::shared_ptr<Project>& project, const bool addSnapshots)
{
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
   
   // If there is an existing project, find references to replace data 
   if (m_projectExisting)
   {
      VisualizationUtils::replaceStratigraphyTable(m_project, m_projectExisting);
   }
    
   if(addSnapshots) {
      // Write all snapshots
      const SnapShotList snapShotList = project->getSnapShots();
      pugi::xml_node snapShotNodes = pt.append_child("snapshots");
      
      for (auto& snapShot : snapShotList)
      {
         pugi::xml_node node = snapShotNodes.append_child("snapshot");
         addSnapShot(snapShot, node);
      }
   }
  // Write all geometries
   if (project->getGeometries().size() > 0)
   {
      pugi::xml_node geometryNode = pt.append_child("geometries");
      for (auto& geometry : project->getGeometries())
      {
         addGeometryInfo2D(geometryNode, geometry);
      }
   }
   
   // Write stratigraphy table & formations
   ///////////////////////////////////////////////
   {
      // Create datastore	for input surfaces
      ibs::FilePath inputSurfaceStorePath(m_fullPath);
      inputSurfaceStorePath << "Input_surfaces.cldrn";
      DataStoreSave inputSurfaceDataStore(inputSurfaceStorePath.path(), m_append);
      
      // Collect all data
      std::vector<VisualizationIOData*> allSurfaceData;
      for (const StratigraphyTableEntry& entry : project->getStratigraphyTable())
      {
         if (entry.getSurface())
         {
            for (const PropertySurfaceData& propertySurfaceData : entry.getSurface()->getPropertySurfaceDataList())
            {
               VisualizationIOData* surfaceData = propertySurfaceData.second.get();
               if (!surfaceData->isRetrieved())
                  allSurfaceData.push_back(surfaceData);
            }
         }
      }
      for (auto& formation : project->getFormations())
      {
         for (auto& propSurfData : formation->getPropertySurfaceDataList())
         {
            VisualizationIOData* surfaceData = propSurfData.second.get();
            if (!surfaceData->isRetrieved())
               allSurfaceData.push_back(surfaceData);
         }
      }
      
      // Retrieve it
      CauldronIO::VisualizationUtils::retrieveAllData(allSurfaceData, m_numThreads);
      
      // Add the entries
      pugi::xml_node stratTableNode = pt.append_child("stratigraphytable");
      for (const StratigraphyTableEntry& entry : project->getStratigraphyTable())
      {
         addStratTableNode(stratTableNode, entry, inputSurfaceDataStore);
      }
      
      // Add formations
      pugi::xml_node formationNode = pt.append_child("formations");
      for (auto& formation :project->getFormations())
      {
         addFormation(inputSurfaceDataStore, formationNode, formation);
      }
      
      // Compress the data
      std::vector<std::shared_ptr<DataToCompress> > allData;
      for (int i = 0; i < inputSurfaceDataStore.getDataToCompressList().size(); i++)
         allData.push_back(inputSurfaceDataStore.getDataToCompressList().at(i));
      
      boost::lockfree::queue<int> queue(allData.size());
      boost::thread_group threads;
      
      // Add to queue
      for (int i = 0; i < allData.size(); i++)
         queue.push(i);
      
      // Compress it in separate threads
      for (int i = 0; i < m_numThreads; ++i)
         threads.add_thread(new boost::thread(CauldronIO::ExportToXML::compressDataQueue, allData, &queue));
      threads.join_all();
      
      // Write to disk
      inputSurfaceDataStore.flush();
   }
   
   // Write migrationIO table
   addMigrationEventList(pt);
   
   // Write TrapperIO table
   addTrapperList(pt);
   
   // Write TrapIO table
   addTrapList(pt);
   
   // Add Genex history files references
   addGenexHistory(pt);
   
   // Add Burial history files references
   addBurialHistory(pt);
   
   // Add MassBalance file reference
   addMassBalance(pt);
   
   // Add 1D data
   add1Ddata(pt);
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

void CauldronIO::ExportToXML::addFormation(DataStoreSave& dataStore, pugi::xml_node node, const std::shared_ptr<Formation>& formation) const
{
    pugi::xml_node subNode = node.append_child("formation");
    subNode.append_attribute("name") = formation->getName().c_str();
    int start, end;
    formation->getK_Range(start, end);
    
    subNode.append_attribute("kstart") = start;
    subNode.append_attribute("kend") = end;
    
	if (formation->isSourceRock())
	{
		subNode.append_attribute("isSR") = true;
	}
	if (formation->isMobileLayer())
	{
		subNode.append_attribute("isML") = true;
	}

	// Add thicknessmap if present
	pugi::xml_node subsubNode;
	if (formation->hasThicknessMap())
	{
		subsubNode = subNode.append_child("thicknessmap");
		addPropertySurfaceData(subsubNode, dataStore, formation->getThicknessMap());
	}
	if (formation->hasSourceRockMixingHIMap())
	{
		subsubNode = subNode.append_child("mixingHImap");
		addPropertySurfaceData(subsubNode, dataStore, formation->getSourceRockMixingHIMap());
	}
	if (formation->hasLithoType1PercentageMap())
	{
		subsubNode = subNode.append_child("lithoType1Perc1map");
		addPropertySurfaceData(subsubNode, dataStore, formation->getLithoType1PercentageMap());
	}
	if (formation->hasLithoType2PercentageMap())
	{
		subsubNode = subNode.append_child("lithoType1Perc2map");
		addPropertySurfaceData(subsubNode, dataStore, formation->getLithoType2PercentageMap());
	}
	if (formation->hasLithoType3PercentageMap())
	{
		subsubNode = subNode.append_child("lithoType1Perc3map");
		addPropertySurfaceData(subsubNode, dataStore, formation->getLithoType3PercentageMap());
	}

	// Add other properties
	if (formation->getSourceRock1Name().length() > 0)
	{
		subNode.append_attribute("sr1name") = formation->getSourceRock1Name().c_str();
	}
	if (formation->getSourceRock2Name().length() > 0)
	{
		subNode.append_attribute("sr2name") = formation->getSourceRock2Name().c_str();
	}
	if (formation->getFluidType().length() > 0)
	{
		subNode.append_attribute("fluid_type") = formation->getFluidType().c_str();
	}
	if (formation->getEnableSourceRockMixing())
	{
		subNode.append_attribute("sr_mixing") = true;
	}
	if (formation->hasAllochthonousLithology())
	{
		subNode.append_attribute("allocht_lith") = true;
		subNode.append_attribute("allocht_lith_name") = formation->getAllochthonousLithologyName().c_str();
	}
	if (formation->hasConstrainedOverpressure())
	{
		subNode.append_attribute("constrained_op") = true;
	}
	if (formation->hasChemicalCompaction())
	{
		subNode.append_attribute("chem_compact") = true;
	}
	if (formation->isIgneousIntrusion())
	{
		subNode.append_attribute("ignious_intr") = true;
		subNode.append_attribute("ignious_intr_age") = formation->getIgneousIntrusionAge();
	}
	subNode.append_attribute("depo_sequence") = formation->getDepoSequence();
	subNode.append_attribute("elem_refinement") = formation->getElementRefinement();
	subNode.append_attribute("mixing_model") = formation->getMixingModel().c_str();
	if (formation->getLithoType1Name().length() > 0)
	{
		subNode.append_attribute("litho1_name") = formation->getLithoType1Name().c_str();
	}
	if (formation->getLithoType2Name().length() > 0)
	{
		subNode.append_attribute("litho2_name") = formation->getLithoType2Name().c_str();
	}
	if (formation->getLithoType3Name().length() > 0)
	{
		subNode.append_attribute("litho3_name") = formation->getLithoType3Name().c_str();
	}
}

void CauldronIO::ExportToXML::addSurface(DataStoreSave& dataStore, pugi::xml_node& surfacesNode, const std::shared_ptr<Surface>& surfaceIO) const
{
	// General properties
	pugi::xml_node ptree = surfacesNode.append_child("surface");
	
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
	if (surfaceIO->isAgeDefined())
		ptree.append_attribute("age") = surfaceIO->getAge();

    // Iterate over all contained valuemaps
    const PropertySurfaceDataList valueMaps = surfaceIO->getPropertySurfaceDataList();

    if (valueMaps.size() > 0)
    {
        pugi::xml_node valueMapsNode = ptree.append_child("propertymaps");
        BOOST_FOREACH(const PropertySurfaceData& propertySurfaceData, valueMaps)
        {
			pugi::xml_node node = valueMapsNode.append_child("propertymap");
			addPropertySurfaceData(node, dataStore, propertySurfaceData);
		}
    }
}


void CauldronIO::ExportToXML::addPropertySurfaceData(pugi::xml_node &node, DataStoreSave &dataStore, const PropertySurfaceData &propertySurfaceData) const
{
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
        if(m_project-> getModelingMode() ==  MODE1D) {
           node.append_attribute("depo_sequence") = surfaceData->getDepoSequence();
         }
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
            node.append_attribute("geom-index") = (int)m_project->getGeometryIndex(thisGeometry, true);

            size_t numBytes = thisGeometry->getSize() * sizeof(float);

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

void CauldronIO::ExportToXML::addSnapShot(const std::shared_ptr<SnapShot>& snapShot, pugi::xml_node node)
{
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(6);
    ss << snapShot->getAge();
    std::string snapshotString = ss.str();
    std::cout << "Writing snapshot Age=" << snapshotString << std::endl;

    ibs::FilePath volumeStorePath(m_fullPath);
    volumeStorePath << "Snapshot_" + snapshotString + "_volumes.cldrn";

    DataStoreSave volumeStore(volumeStorePath.path(), m_append);

    ibs::FilePath surfaceStorePath(m_fullPath);
    surfaceStorePath << "Snapshot_" + snapshotString + "_surfaces.cldrn";

    DataStoreSave surfaceDataStore(surfaceStorePath.path(), m_append);

    node.append_attribute("age") = snapShot->getAge();
    node.append_attribute("kind") = snapShot->getKind();
    node.append_attribute("isminor") = snapShot->isMinorShapshot();

	// If there is an existing project, find references to replace data 
	if (m_projectExisting)
	{
		VisualizationUtils::replaceExistingProperties(snapShot, m_projectExisting);
	}

    // Read all data into memory: if there is an existing project, this is too late...
	std::vector < VisualizationIOData* > data = snapShot->getAllRetrievableData();
    CauldronIO::VisualizationUtils::retrieveAllData(data, m_numThreads);
    
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
            // Data storage
            addSurface(surfaceDataStore, surfacesNode, surfaceIO);
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

    // Compress on main thread or separately
    if (m_numThreads == 1)
    {
        for (int i = 0; i < allData.size(); i++)
        {
            auto& dataToCompress = allData.at(i);
            dataToCompress->compress();
        }
    }
    else
    {
        boost::lockfree::queue<int> queue(allData.size());
        boost::thread_group threads;

        // Add to queue
        for (int i = 0; i < allData.size(); i++)
            queue.push(i);

        // Compress it in separate threads
        for (int i = 0; i < m_numThreads; ++i)
            threads.add_thread(new boost::thread(CauldronIO::ExportToXML::compressDataQueue, allData, &queue));
        threads.join_all();
    }

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


void CauldronIO::ExportToXML::addStratTableNode(pugi::xml_node& stratTableNode, const StratigraphyTableEntry& entry, DataStoreSave& dataStoreSave)
{
	if (entry.getFormation())
	{
		// Do not store the actual formation, it is in the formationlist
		pugi::xml_node formationNode = stratTableNode.append_child("formation");
		formationNode.append_attribute("name") = entry.getFormation()->getName().c_str();
	}
	else if (entry.getSurface())
	{
		addSurface(dataStoreSave, stratTableNode, entry.getSurface());
	}
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
            if (dynamic_cast<MapNative*>(surfaceIO->getPropertySurfaceDataList().at(0).second.get()) != nullptr) return true;
            return false;
        }

        const std::shared_ptr<Volume>& volume = snapShot->getVolume();
		if (volume)
		{
			BOOST_FOREACH(const PropertyVolumeData& volumeData, volume->getPropertyVolumeDataList())
			{
				if (dynamic_cast<VolumeDataNative*>(volumeData.second.get()) != nullptr) return true;
				return false;
			}
		}
    }

    // This should not happen
    return false;
}

void CauldronIO::ExportToXML::addMigrationEventList(pugi::xml_node pt)
{
    size_t nr_events = m_project->getMigrationEventsTable().size();
    if (nr_events == 0) return;

    MigrationEventList events = m_project->getMigrationEventsTable();

    pugi::xml_node node = pt.append_child("migrationEvents");
    node.append_attribute("number") = (unsigned int)nr_events;

    size_t record_size = sizeof(*events[0]);
    node.append_attribute("record_size") = (unsigned int)record_size;

    ibs::FilePath migrationDataPath(m_fullPath);
    migrationDataPath << "migration_events.cldrn";
    DataStoreSave migrationDataStore(migrationDataPath.path(), m_append);

    char* data = new char[record_size * nr_events];
    assert(sizeof(char) == 1);

    size_t dataIndex = 0;

    for (size_t index = 0; index < nr_events; ++index, dataIndex += record_size)
    {
        void* source = (void*)(events[index].get());
        void* dest = (void*)(&data[dataIndex]);
        memcpy(dest, source, record_size);
    }

    // Add all data
    migrationDataStore.addData((void*)data, node, record_size * nr_events);
    // Compress it and write to disk
    migrationDataStore.flush();

    delete[] data;
}


void CauldronIO::ExportToXML::addTrapperList(pugi::xml_node pt)
{
    size_t nr_events = m_project->getTrapperTable().size();
    if (nr_events == 0) return;

    TrapperList events = m_project->getTrapperTable();

    pugi::xml_node node = pt.append_child("trapper");
    node.append_attribute("number") = (unsigned int)nr_events;

    size_t record_size = sizeof(*events[0]);
    node.append_attribute("record_size") = (unsigned int)record_size;

    ibs::FilePath trapperDataPath(m_fullPath);
    trapperDataPath << "trapper_table.cldrn";
    DataStoreSave trapperDataStore(trapperDataPath.path(), m_append);

    char* data = new char[record_size * nr_events];
    assert(sizeof(char) == 1);

    size_t dataIndex = 0;

    for (size_t index = 0; index < nr_events; ++index, dataIndex += record_size)
    {
        void* source = (void*)(events[index].get());
        void* dest = (void*)(&data[dataIndex]);
        memcpy(dest, source, record_size);
    }

    // Add all data
    trapperDataStore.addData((void*)data, node, record_size * nr_events);
    // Compress it and write to disk
    trapperDataStore.flush();

    delete[] data;
}

void CauldronIO::ExportToXML::addTrapList(pugi::xml_node pt)
{
    size_t nr_events = m_project->getTrapTable().size();
    if (nr_events == 0) return;

    TrapList events = m_project->getTrapTable();

    pugi::xml_node node = pt.append_child("trap");
    node.append_attribute("number") = (unsigned int)nr_events;

    size_t record_size = sizeof(*events[0]);
    node.append_attribute("record_size") = (unsigned int)record_size;

    ibs::FilePath trapDataPath(m_fullPath);
    trapDataPath << "trap_table.cldrn";
    DataStoreSave trapDataStore(trapDataPath.path(), m_append);

    char* data = new char[record_size * nr_events];
    assert(sizeof(char) == 1);

    size_t dataIndex = 0;

    for (size_t index = 0; index < nr_events; ++index, dataIndex += record_size)
    {
        void* source = (void*)(events[index].get());
        void* dest = (void*)(&data[dataIndex]);
        memcpy(dest, source, record_size);
    }

    // Add all data
    trapDataStore.addData((void*)data, node, record_size * nr_events);
    // Compress it and write to disk
    trapDataStore.flush();

    delete[] data;
}

void CauldronIO::ExportToXML::addGenexHistory(pugi::xml_node pt)
{
   size_t nr_events = m_project->getGenexHistoryList().size();
   if (nr_events == 0) return;
   
   const std::vector<std::string> historyFiles = m_project->getGenexHistoryList();
   
   pugi::xml_node node = pt.append_child("genexHistoryFiles");
   
   std::vector<std::string>::const_iterator fit;
   for( fit = historyFiles.begin(); fit < historyFiles.end(); ++ fit ) {
      pugi::xml_node propNode = node.append_child("filepath");
      propNode.append_attribute("file") = (* fit).c_str();
   }
}

void CauldronIO::ExportToXML::addMassBalance(pugi::xml_node pt)
{
   const std::string & massBalance = m_project->getMassBalance();
   if (massBalance != "" ) {
      pugi::xml_node node = pt.append_child("massBalance");
      node.append_attribute("file") = massBalance.c_str();
   }
}

void CauldronIO::ExportToXML::addBurialHistory(pugi::xml_node pt)
{
   size_t nr_events = m_project->getBurialHistoryList().size();
   if (nr_events == 0) return;
   
   const std::vector<std::string> historyFiles = m_project->getBurialHistoryList();
   
   pugi::xml_node node = pt.append_child("burialHistoryFiles");
   
   std::vector<std::string>::const_iterator fit;
   for( fit = historyFiles.begin(); fit < historyFiles.end(); ++ fit ) {
      pugi::xml_node propNode = node.append_child("filepath");
      propNode.append_attribute("file") = (* fit).c_str();
   }
}

void CauldronIO::ExportToXML::add1Ddata(pugi::xml_node pt)
{
   // DisplayContour
   DisplayContourList displayRecords = m_project->getDisplayContourTable();
   size_t nr_events = displayRecords.size();

   pugi::xml_node node;
   
   if (nr_events != 0) {

      node = pt.append_child("displayContour");
      node.append_attribute("number") = (unsigned int)nr_events;
      BOOST_FOREACH(const std::shared_ptr<const DisplayContour>& entry, displayRecords)
      {
         pugi::xml_node recordNode = node.append_child("record");
         
         recordNode.append_attribute("propertyName") = entry->getPropertyName().c_str();
         recordNode.append_attribute("colour") = entry->getContourColour().c_str();
         recordNode.append_attribute("value") = entry->getContourValue();
      }
   }
   // TemperatureIsoIoTbl
   TemperatureIsoList tempIsoRecords = m_project->getTemperatureIsoTable();
   nr_events = tempIsoRecords.size();

   if (nr_events != 0) {

      node = pt.append_child("temperatureIso");
      node.append_attribute("number") = (unsigned int)nr_events;
      BOOST_FOREACH(const std::shared_ptr<const IsoEntry>& entry, tempIsoRecords)
      {
         pugi::xml_node recordNode = node.append_child("record");
         
         recordNode.append_attribute("age") = entry->getAge();
         recordNode.append_attribute("value") = entry->getContourValue();
         recordNode.append_attribute("sum") = entry->getSum();
         recordNode.append_attribute("NP") = entry->getNP();
      }
   }
   // VrIsoIoTbl
   VrIsoList vrIsoRecords = m_project->getVrIsoTable();
   nr_events = vrIsoRecords.size();

   if (nr_events != 0) {

      node = pt.append_child("vrIso"); 
      node.append_attribute("number") = (unsigned int)nr_events;
      BOOST_FOREACH(const std::shared_ptr<const IsoEntry>& entry, vrIsoRecords)
      {
         pugi::xml_node recordNode = node.append_child("record");
         
         recordNode.append_attribute("age") = entry->getAge();
         recordNode.append_attribute("value") = entry->getContourValue();
         recordNode.append_attribute("sum") = entry->getSum();
         recordNode.append_attribute("NP") = entry->getNP();
      }
      
   }
   // FtSampleIoTbl
   FtSampleList fsampRecords = m_project->getFtSampleTable();
   nr_events = fsampRecords.size();
      
   if (nr_events != 0) {

      node = pt.append_child("ftSample"); 
      node.append_attribute("number") = (unsigned int)nr_events;
      BOOST_FOREACH(const std::shared_ptr<const FtSample>& entry, fsampRecords)
      {
         pugi::xml_node recordNode = node.append_child("record");
         
         recordNode.append_attribute("id") = entry->getFtSampleId().c_str();
         recordNode.append_attribute("depthInd") = entry->getDepthIndex();
         recordNode.append_attribute("zeta") = entry->getFtZeta();
         recordNode.append_attribute("ustglTrackDensity") = entry->getFtUstglTrackDensity();
         recordNode.append_attribute("predictedAge") = entry->getFtPredictedAge();
         recordNode.append_attribute("pooledAge") = entry->getFtPooledAge();
         recordNode.append_attribute("pooledAgeErr") = entry->getFtPooledAgeErr();
         recordNode.append_attribute("ageChi2") = entry->getFtAgeChi2();
         recordNode.append_attribute("degreeOfFreedom") = entry->getFtDegreeOfFreedom();
         recordNode.append_attribute("pAgeChi2") = entry->getFtPAgeChi2();
         recordNode.append_attribute("corrCoeff") = entry->getFtCorrCoeff();
         recordNode.append_attribute("varianceSqrtNs") = entry->getFtVarianceSqrtNs();
         recordNode.append_attribute("varianceSqrtNi") = entry->getFtVarianceSqrtNi();
         recordNode.append_attribute("nsDivNi") = entry->getFtNsDivNi();
         recordNode.append_attribute("nsDivNiErr") = entry->getFtNsDivNiErr();
         recordNode.append_attribute("meanRatio") = entry->getFtMeanRatio();
         recordNode.append_attribute("meanRatioErr") = entry->getFtMeanRatioErr();
         recordNode.append_attribute("centralAge") = entry->getFtCentralAge();
         recordNode.append_attribute("centralAgeErr") = entry->getFtCentralAgeErr();
         recordNode.append_attribute("meanAge") = entry->getFtMeanAge();
         recordNode.append_attribute("meanAgeErr") = entry->getFtMeanAgeErr();
         recordNode.append_attribute("lengthChi2") = entry->getFtLengthChi2();
         recordNode.append_attribute("apatiteYield") = entry->getFtApatiteYield().c_str();
         
      }
   }
    // FtGrainIoTbl
    FtGrainList fgRecords = m_project->getFtGrainTable();
    nr_events = fgRecords.size();
    
    if (nr_events != 0) {
    
       node = pt.append_child("ftGrain"); 
       node.append_attribute("number") = (unsigned int)nr_events;
       BOOST_FOREACH(const std::shared_ptr<const FtGrain>& entry, fgRecords)
       {
          pugi::xml_node recordNode = node.append_child("record");
          
          recordNode.append_attribute("sampleId") = entry->getFtSampleId().c_str();
          recordNode.append_attribute("grainId") = entry->getFtGrainId();
          recordNode.append_attribute("spontTrackNo") = entry->getFtSpontTrackNo();
          recordNode.append_attribute("inducedTrackNo") = entry->getFtInducedTrackNo();
          recordNode.append_attribute("clWeightPerc") = entry->getFtClWeightPerc();
          recordNode.append_attribute("grainAge") = entry->getFtGrainAge();
       }
    }
    // FtPredLengthCountsHistIoTbl
    FtPredLengthCountsHistList fhRecords = m_project->getFtPredLengthCountsHistTable();
    nr_events = fhRecords.size();
    
    if (nr_events != 0) {
    
       node = pt.append_child("ftPredLengthCountsHist"); 
       node.append_attribute("number") = (unsigned int)nr_events;
       BOOST_FOREACH(const std::shared_ptr<const FtPredLengthCountsHist>& entry, fhRecords)
       {
          pugi::xml_node recordNode = node.append_child("record");
          
          recordNode.append_attribute("id") = entry->getFtPredLengthHistId();
          recordNode.append_attribute("sampleId") = entry->getFtSampleId().c_str();
          recordNode.append_attribute("clWeightPerc") = entry->getFtClWeightPerc();
          recordNode.append_attribute("binStart") = entry->getFtPredLengthBinStart();
          recordNode.append_attribute("binWidth") = entry->getFtPredLengthBinWidth();
          recordNode.append_attribute("binNum") = entry->getFtPredLengthBinNum();
       }
    }
    // FtPredLengthCountsHistDataIoTbl
    FtPredLengthCountsHistDataList fdRecords = m_project->getFtPredLengthCountsHistDataTable();
    nr_events = fdRecords.size();
    
    if (nr_events != 0) {
    
       node = pt.append_child("ftPredLengthCountsHistData"); 
       node.append_attribute("number") = (unsigned int)nr_events;
       BOOST_FOREACH(const std::shared_ptr<const FtPredLengthCountsHistData>& entry, fdRecords)
       {
          pugi::xml_node recordNode = node.append_child("record");
          
          recordNode.append_attribute("id") = entry->getFtPredLengthHistId();
          recordNode.append_attribute("binIndex") = entry->getFtPredLengthBinIndex();
          recordNode.append_attribute("binCount") = entry->getFtPredLengthBinCount();
       }
    }
    // FtClWeightPercBinsTbl
    FtClWeightPercBinsList ftClRecords = m_project->getFtClWeightPercBinsTable();
    nr_events = ftClRecords.size();
    
    if (nr_events != 0) {
    
       node = pt.append_child("ftClWeightPercBins"); 
       node.append_attribute("number") = (unsigned int)nr_events;
       BOOST_FOREACH(const std::shared_ptr<const FtClWeightPercBins>&entry, ftClRecords)
       {
          pugi::xml_node recordNode = node.append_child("record");
          
          recordNode.append_attribute("start") = entry->getFtClWeightBinStart();
          recordNode.append_attribute("width") = entry->getFtClWeightBinWidth();
       }
    }
    // 1DTimeIoTbl
    nr_events = m_project->get1DTimeIoTable().size();
	char * data = 0;
	size_t record_size = 0;
	size_t dataIndex = 0;
    if (nr_events != 0) {

       TimeIo1DList timeio1d_events = m_project->get1DTimeIoTable();
       
       node = pt.append_child("timeIo1D");
       node.append_attribute("number") = (unsigned int)nr_events;
       
       record_size = sizeof(*timeio1d_events[0]);
       node.append_attribute("record_size") = (unsigned int)record_size;
       
       ibs::FilePath time1DDataPath(m_fullPath);
       time1DDataPath << "timeIo1D_table.cldrn";
       DataStoreSave time1DDataStore(time1DDataPath.path(), m_append);
       
       data = new char[record_size * nr_events];
       assert(sizeof(char) == 1);
       
       dataIndex = 0;
       
       for (size_t index = 0; index < nr_events; ++index, dataIndex += record_size)
       {
          void* source = (void*)(timeio1d_events[index].get());
          void* dest = (void*)(&data[dataIndex]);
          memcpy(dest, source, record_size);
       }
       
       // Add all data
       time1DDataStore.addData((void*)data, node, record_size * nr_events);
       // Compress it and write to disk
       time1DDataStore.flush();
       
       delete[] data;
    }
}
