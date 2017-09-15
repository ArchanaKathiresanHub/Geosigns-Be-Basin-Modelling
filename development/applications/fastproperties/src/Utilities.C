//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include <boost/foreach.hpp>
#include "Utilities.h"
#include "VisualizationAPI.h"
#include "DataStore.h"

//------------------------------------------------------------//
bool DerivedProperties::createVizSnapshotResultPropertyValue (  std::shared_ptr<CauldronIO::Project> vizProject,
                                                                GeoPhysics::ProjectHandle* projectHandle, 
                                                                OutputPropertyValuePtr propertyValue, 
                                                                const Snapshot* snapshot, const Interface::Formation * formation,
                                                                const Interface::Surface * surface,
                                                                std::shared_ptr<CauldronIO::FormationInfoList> &formInfoList,
                                                                vector<float> & data ) {
   
   if( not propertyValue->hasMap () ) {
      return true;
   }

   unsigned int p_depth = propertyValue->getDepth();
   
   if( p_depth > 1 and surface == 0 ) {

      if( not propertyValue->isPrimary() ) {
         if(  propertyValue->getProperty ()->getPropertyAttribute () == DataModel::CONTINUOUS_3D_PROPERTY ) {
            createVizSnapshotResultPropertyValueContinuous ( vizProject, projectHandle,  propertyValue, snapshot, formation, formInfoList, data );
         } else if( propertyValue->getProperty ()->getPropertyAttribute () == DataModel::DISCONTINUOUS_3D_PROPERTY ) {
            createVizSnapshotResultPropertyValueDiscontinuous ( vizProject, projectHandle,  propertyValue, snapshot, formation, formInfoList, data );
         } else {
            return false;
         }
      } else {
         //  the property is already in the output file
      }
      
   } else if(not propertyValue->isPrimary() and not ( surface == 0 and propertyValue->isPrimary() )) {
      return createVizSnapshotResultPropertyValueMap ( vizProject, projectHandle,  propertyValue, snapshot, formation, surface, data );
   }

   return true;

}

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
        map->release();
    }

    // Capture global k-range
    size_t currentK = depthFormations->at(0)->kEnd;
    for (int i = 1; i < depthFormations->size(); ++i)
    {
        std::shared_ptr<CauldronIO::FormationInfo>& info = depthFormations->at(i);
        info->kStart += currentK;
        info->kEnd += currentK;
        currentK = info->kEnd;
    }
    return depthFormations;
}

//------------------------------------------------------------//

bool DerivedProperties::createVizSnapshotResultPropertyValueContinuous (  std::shared_ptr<CauldronIO::Project> vizProject,
                                                                          GeoPhysics::ProjectHandle* projectHandle, 
                                                                          OutputPropertyValuePtr propertyValue, 
                                                                          const Snapshot* snapshot, const Interface::Formation * formation,
                                                                          std::shared_ptr<CauldronIO::FormationInfoList> &formInfoList,
                                                                          vector<float> & inData ) {
   
   bool debug = false;

   const Interface::Formation* daFormation = dynamic_cast<const Interface::Formation *>(formation);
   //find info and geometry for the formation
   std::shared_ptr<CauldronIO::FormationInfo> info;
   size_t maxK = 0;
   size_t minK = std::numeric_limits<size_t>::max();
   
   for (size_t i = 0; i < formInfoList->size(); ++i)  {
      std::shared_ptr<CauldronIO::FormationInfo>& depthInfo = formInfoList->at(i);

      maxK = max(maxK, depthInfo->kEnd);
      minK = min(minK, depthInfo->kStart);
      if( depthInfo->formation == formation ) {
         info = formInfoList->at(i);
      }
   }
   size_t depthK = 1 + maxK - minK;
   if( not info ) {
      cout << "Cont: Not found formation " << daFormation->getName() << endl;
      //error;
   } 
   
   const Interface::Property*  daProperty  = dynamic_cast<const Interface::Property *>(propertyValue->getProperty());
   const string propName = propertyValue->getName();
   string propertyMapName = ( daProperty != 0 ? daProperty->getCauldronName() : propName );
   string propertyUserName = ( daProperty != 0 ? daProperty->getUserName() : propName );
   const string unit = ( daProperty != 0 ? daProperty->getUnit() : "" );
   
   if( debug ) {
      cout << snapshot->getTime() << "Cont: Start adding " <<  propName << " for " <<  daFormation->getName() << endl;
   }
   shared_ptr<CauldronIO::SnapShot> vizSnapshot = getSnapShot( vizProject, snapshot->getTime() );
    
   // find or create property
   shared_ptr<const CauldronIO::Property> vizProperty;
   BOOST_FOREACH(const std::shared_ptr<const CauldronIO::Property>& property, vizProject->getProperties()) {
      if (property->getName() == propName ) {
         vizProperty = property;
         break;
      }
   }
   if( not vizProperty ) {
      vizProperty.reset( new CauldronIO::Property(propName, propertyUserName, propertyMapName, unit, CauldronIO::FormationProperty, CauldronIO::Continuous3DProperty));
      vizProject->addProperty(vizProperty);
      if( debug ) {
         cout << "Cont: Adding vizproperty " << propName << endl;
      }
   }
   if( snapshot->getTime() == 0 ) {
      std::shared_ptr<CauldronIO::Formation> vizFormation = vizProject->findFormation(daFormation->getName());
      if( not vizFormation ) {
         vizFormation.reset(new CauldronIO::Formation(static_cast<int>(info->kStart + 1), propertyValue->getDepth(), daFormation->getName()));
         vizProject->addFormation( vizFormation );         
         if( debug ) {
            cout << "Add formation cont" << daFormation->getName() << " kstart " << info->kStart + 1 << " kend " << propertyValue->getDepth() << endl;
         }
     }
      if( not vizFormation->isDepthRangeDefined() ) {
         vizFormation->updateK_range(static_cast<int>(info->kStart), static_cast<int>(info->kEnd));
         if( debug ) {
            cout << "update krange cont " <<  daFormation->getName()<< " " << info->kStart << " " << info->kEnd << endl;
         }
      }
   } 
   
   
   // find or create snapshot volume and geometry
   CauldronIO::SubsurfaceKind formationKind = (daFormation->kind() == Interface::BASEMENT_FORMATION ? CauldronIO::Basement : CauldronIO::Sediment);
   std::shared_ptr<CauldronIO::Volume> snapshotVolume = vizSnapshot->getVolume();
   std::shared_ptr< CauldronIO::Geometry3D> geometry;
   
   shared_ptr<CauldronIO::VolumeData> volDataNew;
   bool propertyVolumeExisting = false;
   
   const DataAccess::Interface::Grid* grid = projectHandle->getActivityOutputGrid ();
   if( not snapshotVolume ) {
      // create volume and geometry
      snapshotVolume.reset(new CauldronIO::Volume(CauldronIO::None));
      vizSnapshot->setVolume(snapshotVolume);
      geometry.reset(new CauldronIO::Geometry3D(grid->numIGlobal(), grid->numJGlobal(), depthK, minK,
                                                grid->deltaIGlobal(), grid->deltaJGlobal(), grid->minIGlobal(), grid->minJGlobal()));
      vizProject->addGeometry(geometry);
      if( debug ) {
         cout << "Adding Geomerty and volume " << " depth = " << depthK << " mink= " << minK << endl;
      }
   } else {
      // find volume the property
      for (size_t i = 0; i <  snapshotVolume->getPropertyVolumeDataList().size(); ++i ) {
         CauldronIO::PropertyVolumeData& pdata =  snapshotVolume->getPropertyVolumeDataList().at(i); 
         if( pdata.first->getName() == propName ) {
            volDataNew = pdata.second;
            if( volDataNew ) {
               propertyVolumeExisting = true;
            }
            break;
         }
      }
      if( propertyVolumeExisting ) {
         // get geomerty for the existing property volume
         geometry = volDataNew->getGeometry();
      } else {
         // get geometry for the first property (assume the same geometry for derived properties)
         if( snapshotVolume->getPropertyVolumeDataList().size() > 0 ) {
            geometry = snapshotVolume->getPropertyVolumeDataList().at(0).second->getGeometry();
         } else {
            geometry.reset(new CauldronIO::Geometry3D(grid->numIGlobal(), grid->numJGlobal(), depthK, minK,
                                                      grid->deltaIGlobal(), grid->deltaJGlobal(), grid->minIGlobal(), grid->minJGlobal()));
            vizProject->addGeometry(geometry);
         }
      }
   }
   if( debug ) {
      cout << info->numI << " " << info->numJ << " " << info->kEnd  << " " << info->kStart << " " <<  
         info->deltaI << " " <<  info->deltaJ << " " <<  info->minI << " " <<  info->minJ << endl;
      cout << daFormation->getName() << " snapshot " << snapshot->getTime() << " geometry3d size = " << geometry->getSize() << " numk = " << geometry->getNumK() << endl;
   }

   float * internalData = 0;

   int firstK = static_cast<int>(info->kStart);
   int lastK  = static_cast<int>(info->kEnd);

   if( not propertyVolumeExisting ) {
      volDataNew.reset(new CauldronIO::VolumeDataNative(geometry, CauldronIO::DefaultUndefinedValue));
      
      int dataSize = static_cast<int>(geometry->getNumI() * geometry->getNumJ() * geometry->getNumK());
      if( dataSize > inData.size() ) {
         inData.resize( dataSize );
      }
      float *data = &inData[0];
      memset( data, 0, sizeof(float) * dataSize );
      
      volDataNew->setData_IJK(data);
      internalData = const_cast<float *>(volDataNew->getVolumeValues_IJK());
      
   } else {
      internalData = const_cast<float *>(volDataNew->getVolumeValues_IJK());
      if(debug) {
         cout << "Adding to existing voldata" << snapshot->getTime() << " " << propName << " " << daFormation->getName() << " kstart= " << info->kStart << " kend= " << info->kEnd << endl;
      }
   }
   if( not internalData ) {
      //error;
   }
   propertyValue->retrieveData();

   for ( int j = grid->firstJ (); j <= grid->lastJ (); ++j ) {
      for ( int i = grid->firstI(); i <= grid->lastI (); ++i ) {
         unsigned int pk = 0;
         for ( int k = lastK; k >= firstK; --k, ++ pk) {
            internalData[volDataNew->computeIndex_IJK(i, j, k)] = static_cast<float>(propertyValue->getValue( i, j, pk ));
         }
      }
   }
   propertyValue->restoreData ();

   if( not propertyVolumeExisting ) {
      CauldronIO::PropertyVolumeData propVolDataNew(vizProperty, volDataNew);
      snapshotVolume->addPropertyVolumeData(propVolDataNew);
      
      if( debug ) {
         cout << "Adding new voldata to " << snapshot->getTime() << " " << propName << " " << daFormation->getName() << " kstart= " << info->kStart << " kend= " << info->kEnd << endl;
         cout << "kstart= " << info->kStart << " kend= " << info->kEnd <<  endl;
      }
   } 

   
   return true;
}
//------------------------------------------------------------//

bool  DerivedProperties::createVizSnapshotResultPropertyValueDiscontinuous (  std::shared_ptr<CauldronIO::Project> vizProject,
                                                                              GeoPhysics::ProjectHandle* projectHandle, 
                                                                              OutputPropertyValuePtr propertyValue, 
                                                                              const Snapshot* snapshot, const Interface::Formation * formation,
                                                                              std::shared_ptr<CauldronIO::FormationInfoList> &formInfoList,
                                                                              vector<float>& inData ) {
      
   bool debug = false;

   const Interface::Formation* daFormation = dynamic_cast<const Interface::Formation *>(formation);
   const Interface::Property*  daProperty  = dynamic_cast<const Interface::Property *>(propertyValue->getProperty());
   const string propName = propertyValue->getName();
   string propertyMapName = ( daProperty != 0 ? daProperty->getCauldronName() : propName );
   string propertyUserName = ( daProperty != 0 ? daProperty->getUserName() : propName );
   const string unit = ( daProperty != 0 ? daProperty->getUnit() : "" );
   if( propertyMapName == "HorizontalPermeability" ) propertyMapName = "PermeabilityHVec2";
 
   if( debug ) {
      cout << snapshot->getTime() << " Disc: Start adding " <<  propName << " for " <<  daFormation->getName() << endl;
   }
   shared_ptr<CauldronIO::SnapShot> vizSnapshot = getSnapShot( vizProject, snapshot->getTime() );
   
   std::shared_ptr<CauldronIO::FormationInfo> info;

   for (size_t i = 0; i < formInfoList->size(); ++i)  {
      std::shared_ptr<CauldronIO::FormationInfo>& depthInfo = formInfoList->at(i);

      if( depthInfo->formation == formation ) {
         info = formInfoList->at(i);
      }
   }
   if( not info ) {
      cout << "Discont: Not found formation " << daFormation->getName() << endl;
      //error;
   } 
   
   std::shared_ptr< CauldronIO::Formation> vizFormation = vizProject->findFormation( daFormation->getName() );
   if( not vizFormation ) {
      vizFormation.reset(new CauldronIO::Formation(static_cast<int>(info->kStart), static_cast<int>(info->kEnd), daFormation->getName()));
      vizProject->addFormation( vizFormation );    
      if( debug ) { 
         cout << "Add formation " << daFormation->getName() << " kstart " << info->kStart << " kend " << info->kEnd << " depth " << propertyValue->getDepth() << endl;
      }
      if( not vizFormation->isDepthRangeDefined() ) {
         vizFormation->updateK_range(static_cast<int>(info->kStart), static_cast<int>(info->kEnd));
         if( debug ) {
            cout << "update krange " <<  daFormation->getName()<< " " << info->kStart << " " << info->kEnd << endl;
         }
      }
     
   }

   //create geometry
   const DataAccess::Interface::Grid* grid = projectHandle->getActivityOutputGrid ();
   const std::shared_ptr<CauldronIO::Geometry3D> geometry(new CauldronIO::Geometry3D(grid->numIGlobal(), grid->numJGlobal(), propertyValue->getDepth(), info->kStart,
                                                                                     grid->deltaIGlobal(), grid->deltaJGlobal(), grid->minIGlobal(), grid->minJGlobal()));
   vizProject->addGeometry(geometry);

   // find or create property
   shared_ptr<const CauldronIO::Property> vizProperty;
   BOOST_FOREACH(const std::shared_ptr<const CauldronIO::Property>& property, vizProject->getProperties()) {
      if (property->getName() == propName ) {
         vizProperty = property;
         break;
      }
   }
   if( not vizProperty ) {
      vizProperty.reset( new CauldronIO::Property(propName, propertyUserName, propertyMapName, unit, CauldronIO::FormationProperty, CauldronIO::Discontinuous3DProperty));
      vizProject->addProperty(vizProperty);
   }
   
   // create volume data
   shared_ptr<CauldronIO::VolumeData> volDataNew(new CauldronIO::VolumeDataNative(geometry));
   
   int dataSize = static_cast<int>(geometry->getNumI() * geometry->getNumJ() * geometry->getNumK());
   if( inData.size() < dataSize ) {
      inData.resize( dataSize );
   }
   float * data = &inData[0];
   memset( data, 0, sizeof(float) * dataSize );

   propertyValue->retrieveData();

   volDataNew->setData_IJK(data);
   float * internalData = const_cast<float *>(volDataNew->getVolumeValues_IJK());
      
   if( not internalData ) {
      //error;
   }

   int firstK = static_cast<int>(geometry->getFirstK());
   int lastK  = static_cast<int>(geometry->getFirstK() + geometry->getNumK() - 1);

   for ( int i = grid->firstI(); i <= grid->lastI (); ++i ) {
      for ( int j = grid->firstJ (); j <= grid->lastJ (); ++j ) {
         unsigned int pk = 0;
         for ( int k = lastK; k >= firstK; --k, ++ pk ) {
            internalData[volDataNew->computeIndex_IJK(i, j, k)] = static_cast<float>(propertyValue->getValue( i, j, pk ));
         }
      }
   }

   propertyValue->restoreData ();

   int rank;
   MPI_Comm_rank ( MPI_COMM_WORLD, &rank );

   MPI_Reduce( (void *)internalData, (void *)data, dataSize, MPI_FLOAT, MPI_SUM, 0,  MPI_COMM_WORLD );

   if( rank == 0 ) {
      std::memcpy( internalData, data, dataSize * sizeof(float));
   }

   DerivedProperties::updateVolumeDataConstantValue( volDataNew );
   
   CauldronIO::PropertyVolumeData propVolDataNew(vizProperty, volDataNew);
   CauldronIO::SubsurfaceKind formationKind = (daFormation->kind() == Interface::BASEMENT_FORMATION ? CauldronIO::Basement : CauldronIO::Sediment);
   
   CauldronIO::FormationVolumeList& formVolList = const_cast<CauldronIO::FormationVolumeList&>(vizSnapshot->getFormationVolumeList());
   shared_ptr<CauldronIO::Volume> vol;
   
   bool existingVolume = false;
   if( formVolList.size() > 0 ) {
      BOOST_FOREACH(CauldronIO::FormationVolume& volume, formVolList) {
         
         if (volume.first != 0 and volume.first->getName() == daFormation->getName()) {
            vol = volume.second;
            existingVolume = true;
         }
      }
   }
   if( not vol ) {
      vol = shared_ptr<CauldronIO::Volume>(new CauldronIO::Volume(formationKind));
   }
   vol->addPropertyVolumeData(propVolDataNew);
   
   if( not existingVolume ) {
      CauldronIO::FormationVolume formVol = CauldronIO::FormationVolume(vizFormation, vol);
      vizSnapshot->addFormationVolume(formVol);         
   }
   return true;
}

//------------------------------------------------------------//
bool DerivedProperties::createVizSnapshotResultPropertyValueMap (  std::shared_ptr<CauldronIO::Project> vizProject,
                                                                   GeoPhysics::ProjectHandle* projectHandle, 
                                                                   OutputPropertyValuePtr propertyValue, 
                                                                   const Snapshot* snapshot, const Interface::Formation * formation,
                                                                   const Interface::Surface * surface,
                                                                   vector<float> & inData ) {
   
   bool debug = false;

   const Interface::Surface*   daSurface   = dynamic_cast<const Interface::Surface *>(surface);
   const Interface::Formation* daFormation = dynamic_cast<const Interface::Formation *>(formation);
   const Interface::Property*  daProperty  = dynamic_cast<const Interface::Property *>(propertyValue->getProperty());
   const string propName = propertyValue->getName();
   string propertyMapName = ( daProperty != 0 ? daProperty->getCauldronName() : propName );
   string propertyUserName = ( daProperty != 0 ? daProperty->getUserName() : propName );
   
   const string surfaceName = ( daSurface != 0 ? daSurface->getName() : "" );
   const string unit = ( daProperty != 0 ? daProperty->getUnit() : "" );
   if( propertyMapName == "HorizontalPermeability" ) propertyMapName = "PermeabilityHVec2";
   if( propertyValue->getName() == "Reflectivity" )   propertyMapName = "Reflectivity";
   
   if( debug ) {
      cout << snapshot->getTime() << " Start adding "<<  propName << " " << (daSurface ? daSurface->getName() : daFormation->getName()) << " " <<  propertyMapName << endl;
   }

   // Create geometry
   const DataAccess::Interface::Grid* grid = projectHandle->getActivityOutputGrid ();
   std::shared_ptr<const CauldronIO::Geometry2D> geometry(new CauldronIO::Geometry2D(grid->numIGlobal(), grid->numJGlobal(), grid->deltaIGlobal(), 
                                                                                     grid->deltaJGlobal(), grid->minIGlobal(), grid->minJGlobal()));
  vizProject->addGeometry(geometry);
   
   // find/create a property
   shared_ptr<const CauldronIO::Property> vizProperty;
   BOOST_FOREACH(const std::shared_ptr<const CauldronIO::Property>& property, vizProject->getProperties()) {
      if ( property->getName() == propName ) {
         vizProperty = property;
         break;
      }
   }
   if( not vizProperty ) {
      if( daSurface != 0 ) {
         vizProperty.reset( new CauldronIO::Property(propName, propertyUserName, propertyMapName, unit, CauldronIO::FormationProperty, CauldronIO::Surface2DProperty));
      } else {
         vizProperty.reset( new CauldronIO::Property(propName, propertyUserName, propertyMapName, unit, CauldronIO::FormationProperty, CauldronIO::Formation2DProperty));
      }
      vizProject->addProperty(vizProperty);
      if( debug ) {
         cout << "Adding property " << propName << endl;
      }
   }
   
   // find/create snapshot

   shared_ptr<CauldronIO::SnapShot> vizSnapshot = getSnapShot( vizProject, snapshot->getTime() );
   if( not vizSnapshot ) {
      //create snapshot
      CauldronIO::SnapShotKind kind = DerivedProperties::getSnapShotKind( snapshot );
      
      vizSnapshot.reset(new CauldronIO::SnapShot(snapshot->getTime(), kind, snapshot->getType() == MINOR));
      vizProject->addSnapShot(vizSnapshot);
   } 
    // find/create snapshot surface
   std::shared_ptr<CauldronIO::Surface> vizSurface;
   CauldronIO::SubsurfaceKind kind = CauldronIO::None;
   
   std::shared_ptr< CauldronIO::Formation> vizFormation = vizProject->findFormation( daFormation->getName() );
   
   if( daSurface != 0 ) {
      vizSurface = getSurface( vizSnapshot, surfaceName );
      if( not vizSurface ) {
         if( daSurface->kind() == BASEMENT_SURFACE ) kind = CauldronIO::Basement;
         else kind = CauldronIO::Sediment;
         vizSurface.reset(new CauldronIO::Surface(surfaceName,  kind ));
         // assign formations
         if( daSurface->getTopFormation() != 0 ) {
            std::shared_ptr<CauldronIO::Formation> topFormation = vizProject->findFormation( daSurface->getTopFormation()->getName());
            vizSurface->setFormation( topFormation, true);
          }
         if( daSurface->getBottomFormation() != 0 ) {
            std::shared_ptr<CauldronIO::Formation> bottomFormation = vizProject->findFormation( daSurface->getBottomFormation()->getName());
            vizSurface->setFormation( bottomFormation, false );
         }
         
         vizSnapshot->addSurface( vizSurface );
         if( debug ) {
            cout << "Adding surface " << surfaceName << endl;
         }
     } else {
         if( debug ) {
            cout << "Found surface " << surfaceName << endl;
         }
      }
   } else {
      // find/create surface for formation map
      vizSurface = getSurface( vizSnapshot, vizFormation );
      if( not vizSurface ) {
         vizSurface.reset(new CauldronIO::Surface(surfaceName,  kind ));
         
         vizSurface->setFormation(vizFormation, true);
         vizSurface->setFormation(vizFormation, false);
         vizSnapshot->addSurface( vizSurface );
         if( debug ) {
            cout << "Adding surface " << surfaceName << " for formation " << daFormation->getName() << endl;
         }
      } else {
         if( debug ) {
            cout << "Found surface " << surfaceName << " as formation " << daFormation->getName() << endl;
         }
      }
   }

   // copy property data
   unsigned int p_depth = propertyValue->getDepth();
   unsigned int kIndex = 0;
   if( daSurface != 0 and daSurface == daFormation->getTopSurface ()) {
      kIndex = p_depth - 1;
   }
   
   assert( p_depth == 1 );

   int dataSize = static_cast<int>(geometry->getNumI() * geometry->getNumJ());
   if( inData.size() < dataSize ) {
      inData.resize( dataSize );
   }
   float * data = &inData[0]; 
   memset( data, 0, sizeof(float) * dataSize );
   // assign surface map
   std::shared_ptr< CauldronIO::SurfaceData> valueMap(new CauldronIO::MapNative(geometry));
   valueMap->setData_IJ( data ); 

   float * internalData = const_cast<float *>(valueMap->getSurfaceValues());
   propertyValue->retrieveData();
   for ( int i = grid->firstI(); i <= grid->lastI (); ++i ) {
      for ( int j = grid->firstJ (); j <= grid->lastJ (); ++j ) {
         internalData[valueMap->getMapIndex(i, j)] = static_cast<float>(propertyValue->getValue( i, j, kIndex ));
      }
   }
   propertyValue->restoreData ();

   int rank;
   MPI_Comm_rank ( MPI_COMM_WORLD, &rank );

   MPI_Reduce( (void *)internalData, (void *)data, dataSize, MPI_FLOAT, MPI_SUM, 0,  MPI_COMM_WORLD );
   if( rank == 0 ) {
      std::memcpy( internalData, data, dataSize * sizeof(float));
   }

   CauldronIO::PropertySurfaceData propSurface = CauldronIO::PropertySurfaceData(vizProperty, valueMap);

   if( daSurface == 0 or propertyValue->getProperty ()->getPropertyAttribute () == DataModel::DISCONTINUOUS_3D_PROPERTY ) {
      valueMap->setFormation(vizFormation);
   }

   vizSurface->addPropertySurfaceData( propSurface );

   if( debug ) {
      if( propertyValue->getProperty ()->getPropertyAttribute () != DataModel::DISCONTINUOUS_3D_PROPERTY ) {
         cout << snapshot->getTime() << ": Added " << propName << " " << (surface ? daSurface->getName() : daFormation->getName()) << 
            " count= " << vizSurface->getPropertySurfaceDataList().size() << endl;
      } else {
         cout << snapshot->getTime() << ": Added " << propName << " " << daSurface->getName() << " and " <<  daFormation->getName() << 
            " count= " << vizSurface->getPropertySurfaceDataList().size() << endl;
      }
   }
   
   return true;
}
//------------------------------------------------------------//

void DerivedProperties::createVizSnapshotFormationData( std::shared_ptr<CauldronIO::Project> vizProject,
                                                        GeoPhysics::ProjectHandle* projectHandle, 
                                                        const Snapshot * snapshot, const FormationSurface & formationItem, 
                                                        DataAccess::Interface::PropertyList & properties,
                                                        SnapshotFormationSurfaceOutputPropertyValueMap & allOutputPropertyValues,
                                                        std::shared_ptr<CauldronIO::FormationInfoList> &formInfoList,
                                                        std::vector<float> &data ) {
   
   Interface::PropertyList::iterator propertyIter;

   const Interface::Formation * formation = formationItem.first;
   const Interface::Surface * surface = formationItem.second;


   for ( propertyIter = properties.begin(); propertyIter != properties.end(); ++propertyIter ) {
      const Interface::Property * property = *propertyIter;
      
      OutputPropertyValuePtr propertyValue = allOutputPropertyValues[ snapshot ][ formationItem ][ property ];

      if ( propertyValue != 0 )  {
         printDebugMsg ( " Output property avaiable for" , property, formation, surface,  snapshot );

         createVizSnapshotResultPropertyValue ( vizProject, projectHandle, propertyValue, snapshot, formation, surface, formInfoList, data );
      }
      else
      {
         printDebugMsg ( " No property avaiable for" , property, formation, surface, snapshot );
      }
   }
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

void DerivedProperties::updateConstantValue( std::shared_ptr< CauldronIO::SnapShot>& snapshot )
{
   std::shared_ptr<CauldronIO::Volume> snapshotVolume = snapshot->getVolume();
   if( snapshotVolume ) {
      for (size_t i = 0; i <  snapshotVolume->getPropertyVolumeDataList().size(); ++i ) {
         CauldronIO::PropertyVolumeData& pdata =  snapshotVolume->getPropertyVolumeDataList().at(i); 
         shared_ptr<CauldronIO::VolumeData> volDataNew = pdata.second;
         if(volDataNew->isRetrieved()) {
            DerivedProperties::updateVolumeDataConstantValue( volDataNew );
         }
      }
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

   BOOST_FOREACH(const std::shared_ptr<SnapShot>& snapShot, project->getSnapShots())
      if (snapShot->getAge() == age) return snapShot;
   
   return std::shared_ptr<SnapShot>();
}

//------------------------------------------------------------//

   std::shared_ptr< CauldronIO::Surface > DerivedProperties::getSurface( std::shared_ptr< CauldronIO::SnapShot>& snapshot, const std::string& surfaceName )
{
   BOOST_FOREACH(const std::shared_ptr<CauldronIO::Surface>& surface, snapshot->getSurfaceList() )
      if (surface->getName() == surfaceName) return surface;

   return std::shared_ptr< CauldronIO::Surface >();
}

//------------------------------------------------------------//

std::shared_ptr< CauldronIO::Surface > DerivedProperties::getSurface( std::shared_ptr< CauldronIO::SnapShot>& snapshot, std::shared_ptr< CauldronIO::Formation>& formation )
{

   BOOST_FOREACH(const std::shared_ptr<CauldronIO::Surface>& surface, snapshot->getSurfaceList() )
      if (surface->getBottomFormation() == formation && surface->getTopFormation() == formation) return surface;

   return std::shared_ptr< CauldronIO::Surface >();
}
//------------------------------------------------------------//
void DerivedProperties::collectVolumeData( const std::shared_ptr<SnapShot>& snapshot, vector<float> & inData ) {

   const std::shared_ptr<Volume> volume = snapshot->getVolume();
   if (volume)  {

      PropertyVolumeDataList& propVolList = volume->getPropertyVolumeDataList();
      if( propVolList.size() > 0 ) {
         int rank;
         MPI_Comm_rank ( MPI_COMM_WORLD, &rank );
         
 
         BOOST_FOREACH(PropertyVolumeData& propVolume, propVolList) {
            std::shared_ptr< CauldronIO::VolumeData> valueMap = propVolume.second;
            if( valueMap->isRetrieved() ) {
               std::shared_ptr<const Geometry3D> geometry = valueMap->getGeometry();
               int dataSize = static_cast<int>(geometry->getNumI() * geometry->getNumJ() * geometry->getNumK());
               if( dataSize > inData.size() ) {
                  inData.resize( dataSize );
               }
               float *data = &inData[0];
               
               float * internalData = const_cast<float *>(valueMap->getVolumeValues_IJK());
               
               MPI_Reduce( (void *)internalData, (void *)data, dataSize, MPI_FLOAT, MPI_SUM, 0,  MPI_COMM_WORLD );
               
               if( rank == 0 ) {
                  std::memcpy( internalData, data, dataSize * sizeof(float));
               }
               //            MPI_Barrier( MPI_COMM_WORLD );
            }
         }
      }
   }
}
#if 0


//------------------------------------------------------------//
void DerivedProperties::addVolume( GeoPhysics::ProjectHandle* projectHandle, OutputPropertyValuePtr propertyValue, unsigned int kStart, vector<float> & inData ) {


   const DataAccess::Interface::Grid* grid = projectHandle->getActivityOutputGrid ();
   
   inf numK = propertyValue->getDepth() - kStart;
   int numI = grid->numIGlobal();
   int numJ = grid->numJGlobal();
   
   int dataSize = static_cast<int>(numI * numJ * numK);
   if( dataSize > inData.size() ) {
      inData.resize( dataSize );
   }
   float *data = &inData[0];
   memset( data, 0, sizeof(float) * dataSize );
   
   unsigned int firstK = numK - 1 - kEnd;
   unsigned int lastK = numK - 1 - kStart;
   
   // for ( unsigned int i = geometry->getMinI(); i < geometry->getMinI() + geometry->getNumI(); ++i ) {
   //       for ( unsigned int j = geometry->getMinJ(); j < geometry->getMinJ() + geometry->getNumJ(); ++j ) {
   for ( unsigned int i = grid->firstI(); i <= grid->lastI (); ++i ) {
      for ( unsigned int j = grid->firstJ (); j <= grid->lastJ (); ++j ) {
         unsigned int pk = 0;
         for ( unsigned int k = firstK; k <= lastK; ++k, ++ pk ) {
            size_t index = i + j * numI + (k - firstK) * numI * numJ;
            data[index] = propertyValue->getValue( i, j, pk );
         }
      }
   }
   MPI_Reduce( (void *)data, 0, dataSize, MPI_FLOAT, MPI_SUM, 0,  MPI_COMM_WORLD );
   
}
#endif
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
       //        pugi::xml_node surfacesNode = node.append_child("surfaces");
       BOOST_FOREACH(const std::shared_ptr<CauldronIO::Surface>& surfaceIO, surfaces)
       {
          // Data storage
          const PropertySurfaceDataList valueMaps = surfaceIO->getPropertySurfaceDataList();
          
          if (valueMaps.size() > 0)
          {
             //  pugi::xml_node valueMapsNode = ptree.append_child("propertymaps");
             BOOST_FOREACH(const PropertySurfaceData& propertySurfaceData, valueMaps)
             {
                pugi::xml_node node;// = valueMapsNode.append_child("propertymap");
  		//	addPropertySurfaceData(node, dataStore, propertySurfaceData);
                const std::shared_ptr<SurfaceData>& surfaceData = propertySurfaceData.second;
                surfaceDataStore.addSurface(surfaceData, node);
             }
          }
       }
    }
   const std::shared_ptr<Volume> volume = snapShot->getVolume();
   if (volume)
   {
      //    pugi::xml_node volNode = node.append_child("volume");
      //   addVolume(volumeStore, volume, 0);
      //        volume->retrieve();
      if (volume->getPropertyVolumeDataList().size() > 0)
      {
         pugi::xml_node node;// = volNode.append_child("propertyvols");
        BOOST_FOREACH(const PropertyVolumeData& propVolume, volume->getPropertyVolumeDataList())
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
#if 1
   
   FormationVolumeList formVolumes = snapShot->getFormationVolumeList();
   if (formVolumes.size() > 0)
   {
      //    pugi::xml_node formVolumesNode = node.append_child("formvols");
      BOOST_FOREACH(FormationVolume& formVolume, formVolumes)
      {
         // Only add a volume if it contains something
         if (formVolume.second->getPropertyVolumeDataList().size() > 0)
         {
            // General properties
            pugi::xml_node node;// = formVolumesNode.append_child("formvol");
            
            const std::shared_ptr<Volume> subVolume = formVolume.second;
            const std::shared_ptr<const CauldronIO::Formation> subFormation = formVolume.first;
            
            if (volume->getPropertyVolumeDataList().size() > 0)
            {
               pugi::xml_node node;// = volNode.append_child("propertyvols");
               BOOST_FOREACH(const PropertyVolumeData& propVolume, volume->getPropertyVolumeDataList())
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
            //addVolume(volumeStore, subVolume, 0);
         }
      }
   }
#endif
    surfaceDataStore.flush();
    volumeStore.flush();
    snapShot->release();
 
 }
