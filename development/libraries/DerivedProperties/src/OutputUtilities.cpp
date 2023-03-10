//
// Copyright (C) 2016-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include <iostream>
#include <map>

#include "LogHandler.h"
#include "OutputUtilities.h"
#include "AbstractPropertyManager.h"

using namespace AbstractDerivedProperties;
using namespace Interface;

using namespace std;

bool DerivedProperties::acquireProperties( GeoPhysics::ProjectHandle& projectHandle, const AbstractPropertyManager& propertyManager,
                                           PropertyList & properties, StringVector & propertyNames  ) {

   StringVector::iterator stringIter;

   for ( stringIter = propertyNames.begin(); stringIter != propertyNames.end(); ++stringIter ) {

      const Property * property = projectHandle.findProperty( *stringIter );
      bool isComputable = false;

      if ( property == 0 ) {
         LogHandler( LogHandler::WARNING_SEVERITY ) << "Could not find property named '" << *stringIter << "'";
         //   cout << "Could not find property named '" << *stringIter << "'" << endl;
         continue;
      }

      if (( property->getPropertyAttribute () == DataModel::CONTINUOUS_3D_PROPERTY or
            property->getPropertyAttribute () == DataModel::DISCONTINUOUS_3D_PROPERTY ) and
          propertyManager.formationPropertyIsComputable ( property )) {

         isComputable = true;
      }
      else if (( property->getPropertyAttribute () == DataModel::CONTINUOUS_3D_PROPERTY or
                 property->getPropertyAttribute () == DataModel::SURFACE_2D_PROPERTY ) and
               propertyManager.surfacePropertyIsComputable ( property )) {

         isComputable = true;
      }
      else if ( property->getPropertyAttribute () == DataModel::DISCONTINUOUS_3D_PROPERTY and
                propertyManager.formationSurfacePropertyIsComputable ( property )) {

         isComputable = true;
      }
      else if ( property->getPropertyAttribute () == DataModel::FORMATION_2D_PROPERTY and
                ( propertyManager.formationMapPropertyIsComputable ( property ) or propertyManager.reservoirPropertyIsComputable ( property ))) {

         isComputable = true;

      }

      if ( isComputable ) {
         properties.push_back( property );
      } else {
         LogHandler( LogHandler::DEBUG_SEVERITY ) << "Could not find calculator for property named '" << *stringIter << "'";
      }

   }

   return true;
}

//------------------------------------------------------------//

bool  DerivedProperties::acquireFormations( GeoPhysics::ProjectHandle& projectHandle, FormationSurfaceVector & formationSurfacePairs,
                                            StringVector & formationNames, const bool includeBasement ) {

   if ( formationNames.size() != 0 ) {

      StringVector::iterator stringIter;
      for ( stringIter = formationNames.begin(); stringIter != formationNames.end(); ++stringIter ) {
         const Formation * formation = projectHandle.findFormation( *stringIter );
         if ( !formation ) {
            LogHandler( LogHandler::INFO_SEVERITY ) << "Could not find formation named '" << *stringIter << "'";
            // cout << "Could not find formation named '" << *stringIter << "'" << endl;
            continue;
         }

         formationSurfacePairs.push_back( FormationSurface( formation, static_cast<const Surface *>(0) ));
      }
   } else {

      const Snapshot * zeroSnapshot = projectHandle.findSnapshot( 0 );

      FormationList * formations = projectHandle.getFormations( zeroSnapshot, includeBasement );
      FormationList::iterator formationIter;

      for ( formationIter = formations->begin(); formationIter != formations->end(); ++formationIter ) {

         const Formation * formation = *formationIter;
         formationSurfacePairs.push_back( FormationSurface( formation, static_cast<const Surface *>(0) ));
      }
      delete formations;
   }
   return true;
}

//------------------------------------------------------------//

bool  DerivedProperties::acquireFormationSurfaces( GeoPhysics::ProjectHandle& projectHandle, FormationSurfaceVector & formationSurfacePairs,
                                                   StringVector & formationNames, bool useTop, const bool includeBasement ) {

   if ( formationNames.size() != 0 )  {

      StringVector::iterator stringIter;
      for ( stringIter = formationNames.begin(); stringIter != formationNames.end(); ++stringIter ) {

         const Formation * formation = projectHandle.findFormation( *stringIter );
         if ( !formation ) {
            LogHandler( LogHandler::WARNING_SEVERITY ) << "Could not find formation named '" << *stringIter << "'" ;
            //cout << "Could not find formation named '" << *stringIter << "'"  << endl;
            continue;
         }

         const Surface * surface = useTop ? formation->getTopSurface() : formation->getBottomSurface();
         if ( !surface ) {
            LogHandler( LogHandler::WARNING_SEVERITY ) << "Could not find " << ( useTop ? "Top " : "Bottom" ) << " surface for formation named '" << *stringIter << "'";
            //cout << "Could not find " << ( useTop ? "Top " : "Bottom" ) << " surface for formation named '" << *stringIter << "'" << endl;
            continue;
         }
         formationSurfacePairs.push_back( FormationSurface( formation, surface ));
      }
   } else {
      const Snapshot * zeroSnapshot = projectHandle.findSnapshot( 0 );

      FormationList * formations = projectHandle.getFormations( zeroSnapshot, includeBasement );
      FormationList::iterator formationIter;
      for ( formationIter = formations->begin(); formationIter != formations->end(); ++formationIter ) {
         const Formation * formation = *formationIter;
         const Surface * surface = useTop ? formation->getTopSurface() : formation->getBottomSurface();

         if ( !surface ) {
            //   cout << "Could not find " << ( useTop ? "Top " : "Bottom" ) << " surface for formation named '" << formation->getName() << "'" << endl;
            LogHandler( LogHandler::WARNING_SEVERITY ) << "Could not find " << ( useTop ? "Top " : "Bottom" ) << " surface for formation named '" << formation->getName() << "'";
            continue;
         }
         formationSurfacePairs.push_back( FormationSurface( formation, surface ));
      }
      delete formations;
   }

   return true;
}

//------------------------------------------------------------//

void  DerivedProperties::removeProperties (  const Snapshot * snapshot,
                                             SnapshotFormationSurfaceOutputPropertyValueMap & allOutputPropertyValues ) {

   SnapshotFormationSurfaceOutputPropertyValueMap::iterator propertyValueToRemove;
   for( propertyValueToRemove = allOutputPropertyValues.begin(); propertyValueToRemove != allOutputPropertyValues.end(); ) {
      if( propertyValueToRemove->first == snapshot ) {

         FormationSurfaceOutputPropertyValueMap map = propertyValueToRemove->second;
         FormationSurfaceOutputPropertyValueMap::iterator mapToRemove;
         for( mapToRemove = map.begin(); mapToRemove != map.end(); mapToRemove = map.erase( mapToRemove ) ) {

            OutputPropertyValueMap map1 = (*mapToRemove).second;
            OutputPropertyValueMap::iterator map2;
            for( map2 = map1.begin(); map2 != map1.end(); map2 = map1.erase( map2 ) ) {
               (map2->second).reset();
            }
         }
         propertyValueToRemove = allOutputPropertyValues.erase( propertyValueToRemove );
      } else {
         ++ propertyValueToRemove;
      }
   }
}

//------------------------------------------------------------//

void  DerivedProperties::outputSnapshotFormationData( GeoPhysics::ProjectHandle& projectHandle,
                                                      const Snapshot * snapshot, const FormationSurface & formationItem,
                                                      DataAccess::Interface::PropertyList & properties,
                                                      SnapshotFormationSurfaceOutputPropertyValueMap & allOutputPropertyValues ) {

   PropertyList::iterator propertyIter;

   const Formation * formation = formationItem.first;
   const Surface * surface = formationItem.second;

   for ( propertyIter = properties.begin(); propertyIter != properties.end(); ++propertyIter ) {
      const Property * property = *propertyIter;

      OutputPropertyValuePtr propertyValue = allOutputPropertyValues[ snapshot ][ formationItem ][ property ];

      if ( propertyValue != 0 )  {
         printDebugMsg ( " Output property available for" , property, formation, surface,  snapshot );

         createSnapshotResultPropertyValue ( projectHandle, propertyValue, snapshot, formation, surface );
      }
      else
      {
         printDebugMsg ( " No property available for" , property, formation, surface, snapshot );
      }
   }
}

//------------------------------------------------------------//

OutputPropertyValuePtr  DerivedProperties::allocateOutputProperty (       AbstractPropertyManager& propertyManager,
                                                                    const DataModel::AbstractProperty* property,
                                                                    const DataModel::AbstractSnapshot* snapshot,
                                                                    const FormationSurface& formationItem,
                                                                    const bool basementIncluded ) {

   OutputPropertyValuePtr outputProperty;

   const Interface::Formation* formation = formationItem.first;
   const Interface::Surface* topSurface = 0;
   const Interface::Surface* bottomSurface = 0;

   if ( formationItem.second == 0 ) {

      if (( property->getPropertyAttribute () == DataModel::CONTINUOUS_3D_PROPERTY or
            property->getPropertyAttribute () == DataModel::DISCONTINUOUS_3D_PROPERTY ) and
          propertyManager.formationPropertyIsComputable ( property, snapshot, formation )) {

         printDebugMsg( "Allocating Formation", property, formation, 0, snapshot );
         outputProperty.reset( new FormationOutputPropertyValue ( propertyManager, property, snapshot, formation ));
      }
      // check if the formation-map or reservoir property is computable
      else if ( property->getPropertyAttribute () == DataModel::FORMATION_2D_PROPERTY ) {
         if ( propertyManager.formationMapPropertyIsComputable ( property, snapshot, formation )) {

            printDebugMsg( "Allocating FM", property, formation, 0, snapshot );
            outputProperty.reset ( new FormationMapOutputPropertyValue ( propertyManager, property, snapshot, formation ));
         }
         else if ( propertyManager.reservoirPropertyIsComputable ( property )) {

            Interface::ReservoirList * reservoirs = formation->getReservoirs();

            if(reservoirs != 0) {
               Interface::ReservoirList::iterator reservoirIter;
               for (reservoirIter = reservoirs->begin(); reservoirIter != reservoirs->end(); ++ reservoirIter) {
                  const Interface::Reservoir * res = * reservoirIter;
                  outputProperty.reset (new ReservoirOutputPropertyValue ( propertyManager, property, snapshot, res ));
                  printDebugMsg( "Allocating Reservoir", property, formation, 0, snapshot );
               }
            }
         }
      }
   }

   if ( outputProperty == 0 ) {
      if ( formation != 0 and formationItem.second != 0  ) {

         if ( formation->getTopSurface () != 0 and formationItem.second == formation->getTopSurface ()) {
            topSurface = formation->getTopSurface ();
         } else if ( formation->getBottomSurface () != 0 and formationItem.second == formation->getBottomSurface ()) {
            bottomSurface = formation->getBottomSurface ();
         }

      }

      // First check if the surface property is computable
      if (( property->getPropertyAttribute () == DataModel::CONTINUOUS_3D_PROPERTY or
            property->getPropertyAttribute () == DataModel::SURFACE_2D_PROPERTY ) and
          (( topSurface != 0    and propertyManager.surfacePropertyIsComputable ( property, snapshot, topSurface )) or
           ( bottomSurface != 0 and propertyManager.surfacePropertyIsComputable ( property, snapshot, bottomSurface )))) {

         if ( topSurface != 0 ) {

            printDebugMsg( "Allocating Top Surface", property, 0, topSurface, snapshot );
            if (!(topSurface->getBottomFormation()->kind() == BASEMENT_FORMATION and topSurface->getBottomFormation()->getName() == "Crust")) {
                outputProperty.reset(new SurfaceOutputPropertyValue(propertyManager, property, snapshot, topSurface));
            }
         } else if ( bottomSurface != 0 ) {

            // Allocate the bottom surface of the domain
            if( bottomSurface->getName() == "Bottom of Lithospheric Mantle" or (bottomSurface->getBottomFormation()->kind() == BASEMENT_FORMATION and bottomSurface->getBottomFormation()->getName() == "Crust")) {

               printDebugMsg( "Allocating Bottom Surface", property, 0, bottomSurface, snapshot );               
               outputProperty.reset( new SurfaceOutputPropertyValue ( propertyManager, property, snapshot, bottomSurface ));
            }
         }
      }
      // Next check if the formation-surface property is computable
      else if ( property->getPropertyAttribute () == DataModel::DISCONTINUOUS_3D_PROPERTY and
                (( topSurface != 0    and propertyManager.formationSurfacePropertyIsComputable ( property, snapshot, formation, topSurface )) or
                 ( bottomSurface != 0 and propertyManager.formationSurfacePropertyIsComputable ( property, snapshot, formation, bottomSurface ))))  {

         if ( topSurface != 0 ) {

            printDebugMsg( "Allocating FS", property, 0, topSurface, snapshot );
            outputProperty.reset ( new FormationSurfaceOutputPropertyValue ( propertyManager, property, snapshot, formation, topSurface ));
        } else if ( bottomSurface != 0 ) {

            printDebugMsg( "Allocating FS", property, 0, bottomSurface, snapshot );
            outputProperty.reset ( new FormationSurfaceOutputPropertyValue ( propertyManager, property, snapshot, formation, bottomSurface ));
         }

      }
   }

   return outputProperty;
}

//------------------------------------------------------------//

bool  DerivedProperties::createSnapshotResultPropertyValue (  GeoPhysics::ProjectHandle& projectHandle, OutputPropertyValuePtr propertyValue,
                                                              const Snapshot* snapshot, const Interface::Formation * formation,
                                                              const Interface::Surface * surface ) {

   // This check is needed because some of the derived properties are creating null properties
   // instead of using the isComputable() function
   if (not propertyValue->hasProperty())
   {
      return true;
   }

   unsigned int p_depth = propertyValue->getDepth();
   unsigned int kIndex = 0;

   PropertyValue *thePropertyValue = 0;

   if( p_depth > 1 and surface == 0 ) {

      if( not propertyValue->isPrimary() ) {
         thePropertyValue = projectHandle.createVolumePropertyValue ( propertyValue->getName(), snapshot, 0, formation, p_depth );
      } else {
         //  the property is already in the output file
      }
   } else if( not ( surface == 0 and propertyValue->isPrimary() )) {

      const Interface::Surface*   daSurface   = dynamic_cast<const Interface::Surface *>(surface);
      const Interface::Formation* daFormation = dynamic_cast<const Interface::Formation *>(formation);
      const Interface::Property*  daProperty  = dynamic_cast<const Interface::Property *>(propertyValue->getProperty());

      if( surface != 0 and surface == formation->getTopSurface ()) {
         kIndex = p_depth - 1;
      }

      string propertyMapName = ( daProperty != 0 ? daProperty->getCauldronName() : propertyValue->getName() );
      if( propertyMapName == "HorizontalPermeability" ) propertyMapName = "PermeabilityHVec2";
      if( propertyValue->getName() == "Reflectivity" )  propertyMapName = "Reflectivity";

      if ( propertyValue->getProperty ()->getPropertyAttribute () == DataModel::SURFACE_2D_PROPERTY ) {
         thePropertyValue = projectHandle.createMapPropertyValue ( propertyMapName, snapshot, 0, 0, daSurface );
      } else if ( propertyValue->getProperty ()->getPropertyAttribute () == DataModel::FORMATION_2D_PROPERTY ) {
         thePropertyValue = projectHandle.createMapPropertyValue ( propertyMapName, snapshot, 0, daFormation, 0 );
      } else if ( propertyValue->getProperty ()->getPropertyAttribute () == DataModel::DISCONTINUOUS_3D_PROPERTY ) {
         thePropertyValue = projectHandle.createMapPropertyValue ( propertyMapName, snapshot, 0, daFormation, daSurface );
      } else if ( propertyValue->getProperty ()->getPropertyAttribute () == DataModel::CONTINUOUS_3D_PROPERTY ) {
         thePropertyValue = projectHandle.createMapPropertyValue ( propertyMapName, snapshot, 0, 0, daSurface );
      }
   }

   if( thePropertyValue != 0 ) {

      printDebugMsg( "Saving ",  propertyValue->getProperty (), formation, surface, snapshot );

      GridMap * theMap = thePropertyValue->getGridMap();

      if( theMap != 0 ) {
         propertyValue->retrieveData();
         theMap->retrieveData();

         for ( unsigned int i = theMap->firstI (); i <= theMap->lastI (); ++i ) {
            for ( unsigned int j = theMap->firstJ (); j <= theMap->lastJ (); ++j ) {
               unsigned int pk = kIndex;
               for ( unsigned int k = 0; k < theMap->getDepth (); ++k, ++ pk ) {
                  theMap->setValue (i, j, k, propertyValue->getValue( i, j, pk ));
               }
            }
         }
         theMap->restoreData (true);
         propertyValue->restoreData ();
         return true;
      }
   }

   return false;
}


//------------------------------------------------------------//

bool DerivedProperties::isEqualPropertyName( const string s1, const string s2 ) {

   return ( s1 == s2 );
}

//------------------------------------------------------------//
void DerivedProperties::printDebugMsg ( const string outputMsg,
                                        const DataModel::AbstractProperty  * property,
                                        const DataModel::AbstractFormation * formation,
                                        const DataModel::AbstractSurface   * surface,
                                        const DataModel::AbstractSnapshot  * snapshot ) {

   const string fname = "log.log";
   const int my_rank = 0;


   LogHandler( LogHandler::DEBUG_SEVERITY) << outputMsg << " " << property->getName() << " " <<
      ( formation != 0 ? formation->getName() : "" ) << " " <<
      ( surface != 0 ? surface->getName() : "" ) << " at " << snapshot->getTime();
}
