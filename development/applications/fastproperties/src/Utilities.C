//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include <petsc.h>
#include "Utilities.h"
#include "VisualizationAPI.h"
#include "DataStore.h"

//------------------------------------------------------------//

std::shared_ptr<CauldronIO::FormationInfoList> DerivedProperties::getDepthFormations( GeoPhysics::ProjectHandle* projectHandle, const Snapshot* snapShot) {

    std::shared_ptr<CauldronIO::FormationInfoList> depthFormations(new CauldronIO::FormationInfoList());

    // Find the depth property
    const Interface::Property* depthProp = projectHandle->findProperty("Depth");
    if (!depthProp) return depthFormations;

    // Find the depth property formations for this snapshot
    std::shared_ptr<PropertyValueList> propValues( projectHandle->getModellingMode() == Interface::MODE1D ?
                                                   projectHandle->getPropertyValues(SURFACE, depthProp, snapShot, 0, 0, 0, MAP| VOLUME) :
                                                   projectHandle->getPropertyValues(FORMATION, depthProp, snapShot, 0, 0, 0, VOLUME));

    if (propValues->size() == 0) return depthFormations;

    // Find all depth formations and add these
    for (size_t i = 0; i < propValues->size(); ++i)
    {
        GridMap* map = propValues->at(i)->getGridMap();

        if (!map) throw CauldronIO::CauldronIOException("Could not open project3D HDF file!");
        std::shared_ptr<CauldronIO::FormationInfo> info(new CauldronIO::FormationInfo());

        map->retrieveData();

        info->formation = propValues->at(i)->getFormation();
        info->kStart = map->firstK();
        info->kEnd = map->lastK();
        info->numI = map->numI();
        info->numJ = map->numJ();
        info->deltaI = map->deltaI();
        info->deltaJ = map->deltaJ();
        info->minI = map->minI();
        info->minJ = map->minJ();

        /// in a SerialGridMap gridmap, depth is aligned (=increasing) with k index
        /// in a DistributedGridmap, depth is inverse to k index
        //   SerialGridMap* sGridmap = static_cast<SerialGridMap*>(map);
        info->reverseDepth = true; //sGridmap == nullptr;
        
        depthFormations->push_back(info);

        map->restoreData();
    }

    // Capture global k-range
    size_t currentK = depthFormations->at(0)->kEnd;
    for (unsigned int i = 1; i < depthFormations->size(); ++i)
    {
        std::shared_ptr<CauldronIO::FormationInfo>& info = depthFormations->at(i);
        info->kStart += currentK;
        info->kEnd += currentK;
        currentK = info->kEnd;
    }
    return depthFormations;
}

//------------------------------------------------------------//
void DerivedProperties::updateVolumeDataConstantValue( shared_ptr<CauldronIO::VolumeData>& volDataNew )
{
   // to do: we  can set min and max value here
   volDataNew->retrieve();    
   
   const std::shared_ptr<Geometry3D>& geometry = volDataNew->getGeometry();
   
   size_t allElements = geometry->getNumI() * geometry->getNumJ() * geometry->getNumK();
   float minValue = CauldronIO::DefaultUndefinedValue;
   float maxValue = CauldronIO::DefaultUndefinedValue;
   const float* internaldata = volDataNew->getVolumeValues_IJK();
   if( internaldata == NULL ) {
      internaldata = volDataNew->getVolumeValues_KIJ();
   }
   
   for (size_t i = 0; i < allElements; i++)
   {
      float val = internaldata[i];
      if (val != DefaultUndefinedValue)
      {
         minValue = minValue == DefaultUndefinedValue ? val : min(minValue, val);
         maxValue = maxValue == DefaultUndefinedValue ? val : max(maxValue, val);
      }
   }
   
   if( minValue == maxValue ) {
      volDataNew->setConstantValue( minValue );
   } 
}

//------------------------------------------------------------//

CauldronIO::SnapShotKind DerivedProperties::getSnapShotKind(const Interface::Snapshot* snapShot)
{
    // Get snapshot kind
    const string snapShotKind = snapShot->getKind();
    CauldronIO::SnapShotKind kind = CauldronIO::NONE;
    if (snapShotKind == "System Generated")
        kind = CauldronIO::SYSTEM;
    else if (snapShotKind == "User Defined")
        kind = CauldronIO::USERDEFINED;
    else
       kind = CauldronIO::NONE;
    return kind;
}

//------------------------------------------------------------//

std::shared_ptr<SnapShot> DerivedProperties::getSnapShot( std::shared_ptr< CauldronIO::Project>& project, const double age ) 
{

   for(auto& snapShot : project->getSnapShots())
      if (snapShot->getAge() == age) return snapShot;
   
   return std::shared_ptr<SnapShot>();
}

//------------------------------------------------------------//

std::shared_ptr< CauldronIO::Surface > DerivedProperties::getSurface( std::shared_ptr< CauldronIO::SnapShot>& snapshot, const std::string& surfaceName )
{
   for(auto& surface : snapshot->getSurfaceList() )
      if (surface->getName() == surfaceName) return surface;

   return std::shared_ptr< CauldronIO::Surface >();
}

//------------------------------------------------------------//

std::shared_ptr< CauldronIO::Surface > DerivedProperties::getSurface( std::shared_ptr< CauldronIO::SnapShot>& snapshot, std::shared_ptr< CauldronIO::Formation>& formation )
{

   for(auto& surface : snapshot->getSurfaceList())
      if (surface->getBottomFormation() == formation.get() && surface->getTopFormation() == formation.get()) return surface;

   return std::shared_ptr< CauldronIO::Surface >();
}


//------------------------------------------------------------//
void DerivedProperties::listProperties(const std::shared_ptr<SnapShot>& snapShot, std::shared_ptr<Project> &project) {

   std::cout << "Snapshot: " << snapShot->getAge() << endl;

   const StratigraphyTableEntryList& list = project->getStratigraphyTable();
   for (auto& entry : list ) {
      const std::shared_ptr<CauldronIO::Surface>& surfaceIO = entry.getSurface();
      if( surfaceIO ) {
         std::cout << "Strat Surface: " << surfaceIO->getName() << std::endl;
         if(  surfaceIO->getPropertySurfaceDataList().size() > 0 ) {
            const PropertySurfaceDataList valueMaps = surfaceIO->getPropertySurfaceDataList();
            
            if (valueMaps.size() > 0)
            {
               for(auto& propertySurfaceData : valueMaps)
               {
                  std::cout << "     " << propertySurfaceData.first->getName() << endl;
               }
            }
         }
      }
      const std::shared_ptr<CauldronIO::Formation>& formationIO = entry.getFormation();
      if(formationIO) {
         std::cout << "Strat Formation: " << formationIO->getName() << std::endl;
         if(  formationIO->getPropertySurfaceDataList().size() > 0 ) {
            const PropertySurfaceDataList valueMaps = formationIO->getPropertySurfaceDataList();
            
            if (valueMaps.size() > 0)
            {
               for(auto& propertySurfaceData : valueMaps)
               {
                  std::cout << "     " << propertySurfaceData.first->getName() << endl;
               }
            }
         }
      }
 
   }

   const CauldronIO::SurfaceList surfaces = snapShot->getSurfaceList();
   if (surfaces.size() > 0)
   {
      for(auto& surfaceIO : surfaces)
      {
         std::cout << "Surface: " << surfaceIO->getName() << std::endl;
         const PropertySurfaceDataList valueMaps = surfaceIO->getPropertySurfaceDataList();
         
         if (valueMaps.size() > 0)
         {
            for(auto& propertySurfaceData : valueMaps)
            {
               std::cout << "     " << propertySurfaceData.first->getName() << " " << ( propertySurfaceData.second->getFormation() != 0 ? 
                                                                                        propertySurfaceData.second->getFormation()->getName() : "" ) << endl;
            }
         }
      }
   }
   const std::shared_ptr<Volume> volume = snapShot->getVolume();
   if (volume)
   {
     if (volume->getPropertyVolumeDataList().size() > 0)
      {
         for(auto& propVolume : volume->getPropertyVolumeDataList())
        {
           std::cout << "Property volume: " << propVolume.first->getName() << std::endl;
        }
        
      }
   }
   FormationVolumeList formVolumes = snapShot->getFormationVolumeList();
   if (formVolumes.size() > 0)
   {
      for(auto& formVolume : formVolumes)
      {
         if (formVolume.second->getPropertyVolumeDataList().size() > 0)
         {
            
            const std::shared_ptr<Volume> subVolume = formVolume.second;
            const std::shared_ptr<const CauldronIO::Formation> subFormation = formVolume.first;
            std::cout << "Formation volume: " << formVolume.first->getName() << endl;
           
            if (subVolume->getPropertyVolumeDataList().size() > 0)
            {
               for(auto& propVolume : subVolume->getPropertyVolumeDataList())
               {
                  std::cout << "       Property " << propVolume.first->getName() << std::endl;
               }
            }
         }
      }
   }
}

//------------------------------------------------------------//
void DerivedProperties::saveVizSnapshot( const std::shared_ptr<SnapShot>& snapShot, const std::string& absPath) {

   std::ostringstream ss;
   ss << std::fixed << std::setprecision(6);
   ss << snapShot->getAge();
   std::string snapshotString = ss.str();

   ibs::FilePath outputPath(absPath);
   ibs::FilePath volumeStorePath(outputPath.filePath());
    volumeStorePath << "Snapshot_" + snapshotString + "_volumes.cldrn";
    DataStoreSave volumeStore(volumeStorePath.path(), true);

    ibs::FilePath surfaceStorePath(outputPath.filePath());
    surfaceStorePath << "Snapshot_" + snapshotString + "_surfaces.cldrn";
    DataStoreSave surfaceDataStore(surfaceStorePath.path(), true);

    const CauldronIO::SurfaceList surfaces = snapShot->getSurfaceList();
   if (surfaces.size() > 0)
    {
       for(auto& surfaceIO : surfaces)
       {
          // Data storage
          const PropertySurfaceDataList valueMaps = surfaceIO->getPropertySurfaceDataList();
          
          if (valueMaps.size() > 0)
          {
             for(auto& propertySurfaceData : valueMaps)
             {
                pugi::xml_node node;
                const std::shared_ptr<SurfaceData>& surfaceData = propertySurfaceData.second;
                surfaceDataStore.addSurface(surfaceData, node);
             }
          }
       }
    }
   const std::shared_ptr<Volume> volume = snapShot->getVolume();
   if (volume)
   {
      if (volume->getPropertyVolumeDataList().size() > 0)
      {
         pugi::xml_node node;
         for(auto& propVolume : volume->getPropertyVolumeDataList())
         {
           const std::shared_ptr<VolumeData>& data = propVolume.second;
           const std::shared_ptr<Geometry3D>& thisGeometry = data->getGeometry();
           size_t numBytes = thisGeometry->getSize() * sizeof(float);
           if ( not data->isConstant())
           {
              volumeStore.addVolume(data, node, numBytes);
           }
        }
      }
   }
   
   FormationVolumeList formVolumes = snapShot->getFormationVolumeList();
   if (formVolumes.size() > 0)
   {
      for(auto& formVolume : formVolumes)
      {
         // Only add a volume if it contains something
         if (formVolume.second->getPropertyVolumeDataList().size() > 0)
         {
            // General properties
            pugi::xml_node node;
            
            const std::shared_ptr<Volume> subVolume = formVolume.second;
            const std::shared_ptr<const CauldronIO::Formation> subFormation = formVolume.first;
            
            if (volume->getPropertyVolumeDataList().size() > 0)
            {
               pugi::xml_node node;
               for(auto& propVolume : volume->getPropertyVolumeDataList())
               {
                  const std::shared_ptr<VolumeData>& data = propVolume.second;
                  const std::shared_ptr<Geometry3D>& thisGeometry = data->getGeometry();
                  size_t numBytes = thisGeometry->getSize() * sizeof(float);
                  if (not data->isConstant())
                  {
                     volumeStore.addVolume(data, node, numBytes);
                  }
               }
            }
         }
      }
   }

    surfaceDataStore.flush();
    volumeStore.flush();
    snapShot->release();
 
 }

//------------------------------------------------------------//
void DerivedProperties::minmax_op( float *invec, float *inoutvec, int *len, MPI_Datatype *datatype ) {  
   
   // Find the minimum (0 index) and maximum (1 index)
   if( invec[0] != DefaultUndefinedValue and inoutvec[0] != DefaultUndefinedValue ) {
      inoutvec[0] = min(invec[0], inoutvec[0]);
   } else if ( invec[0] != DefaultUndefinedValue ) {
      inoutvec[0] = invec[0];
   }
   if( invec[1] != DefaultUndefinedValue and inoutvec[1] != DefaultUndefinedValue ) {
      inoutvec[1] = max(invec[1], inoutvec[1]);
   } else if ( invec[1] != DefaultUndefinedValue ) {
      inoutvec[1] = invec[1];
   }
}
