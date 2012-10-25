#include "TimeOfElementInvasionCalculator.h"
#include "timefilter.h"
#include "DerivedOutputPropertyMap.h"
#include "PropertyManager.h"
#include "FastcauldronSimulator.h"
#include "MultiComponentFlowHandler.h"
#include "PetscBlockVector.h"
#include "Interface/RunParameters.h"
#include "Interface/Interface.h"

OutputPropertyMap* allocateTimeOfElementInvasionVolumeCalculator ( const PropertyList property, LayerProps* formation, const Interface::Snapshot* snapshot ) {
   return new DerivedOutputPropertyMap<TimeOfElementInvasionVolumeCalculator>( property, formation, snapshot );
}

TimeOfElementInvasionVolumeCalculator::TimeOfElementInvasionVolumeCalculator ( LayerProps* formation, const Interface::Snapshot* snapshot ) :
   m_formation ( formation ),  m_snapshot ( snapshot ) {

   m_isCalculated = false;

}

bool TimeOfElementInvasionVolumeCalculator::operator ()( const OutputPropertyMap::OutputPropertyList& properties, 
                                                 OutputPropertyMap::PropertyValueList&  propertyValues ) {

   if ( m_isCalculated ) {
      return true;
   }


   unsigned int i;
   unsigned int j;
   unsigned int k;
   unsigned int p;
   double value;

   const ElementVolumeGrid& grid =  m_formation->getTimeOfElementInvasionGrid ();

   Interface::GridMap* timeOfInvasionMap;
   PetscBlockVector<double> timeOfInvasion;

   timeOfInvasion.setVector ( grid, m_formation->getTimeOfElementInvasionVec (), INSERT_VALUES );

   double undefinedValue;
   double **propertyVector;
   bool isSediment = m_formation->kind () == Interface::SEDIMENT_FORMATION;

   timeOfInvasionMap= propertyValues[0]->getGridMap ();
   timeOfInvasionMap->retrieveData();
   undefinedValue = timeOfInvasionMap->getUndefinedValue ();

   for ( i = timeOfInvasionMap->firstI (); i <= timeOfInvasionMap->lastI (); ++i ) {

      for ( j = timeOfInvasionMap->firstJ (); j <= timeOfInvasionMap->lastJ (); ++j ) {

         for ( k = timeOfInvasionMap->firstK (); k <= timeOfInvasionMap->lastK (); ++k ) {
            timeOfInvasionMap->setValue ( i, j, k, undefinedValue );
         }

      }

   }

   for ( i = grid.firstI (); i <= grid.lastI (); ++i ) {

      for ( j = grid.firstJ (); j <= grid.lastJ (); ++j ) {

         if ( FastcauldronSimulator::getInstance ().getMapElement ( i, j ).isValid ()) {

            for ( k = grid.firstK (); k <= grid.lastK (); ++k ) {
               value = timeOfInvasion( k, j, i );
               timeOfInvasionMap->setValue ( i, j, k, value );

               // Fill other heat flow nodes if current (i,j) position is at end of array
               if ( i == grid.getNumberOfXElements () - 1 ) {
                  timeOfInvasionMap->setValue ( i + 1, j, k, value );
               }
                        
               if ( j == grid.getNumberOfYElements () - 1 ) {
                  timeOfInvasionMap->setValue ( i, j + 1, k, value );
               }
                        
               if ( k == grid.getNumberOfZElements () - 1 ) {
                  timeOfInvasionMap->setValue ( i, j, k + 1, value );
               }
                        
               if ( i == grid.getNumberOfXElements () - 1  and j == grid.getNumberOfYElements () - 1 ) {
                  timeOfInvasionMap->setValue ( i + 1, j + 1, k, value );
               }
                        
               if ( i == grid.getNumberOfXElements () - 1  and k == grid.getNumberOfZElements () - 1 ) {
                  timeOfInvasionMap->setValue ( i + 1, j, k + 1, value );
               }
                        
               if ( j == grid.getNumberOfYElements () - 1  and k == grid.getNumberOfZElements () - 1 ) {
                  timeOfInvasionMap->setValue ( i, j + 1, k + 1, value );
               }
                        
               if ( i == grid.getNumberOfXElements () - 1  and j == grid.getNumberOfYElements () - 1  and k == grid.getNumberOfZElements () - 1 ) {
                  timeOfInvasionMap->setValue ( i + 1, j + 1, k + 1, value );
               }
                        
            }

         }

      }

   }



   timeOfInvasionMap->restoreData ();

  //  if ( isSediment ) {
//       DAVecRestoreArray ( *FastcauldronSimulator::getInstance ().getCauldron ()->mapDA,
//                           m_formation->getTimeOfElementInvasionVec(),
//                           &propertyVector );
//    }

   m_isCalculated = true;
   return true;
}

void TimeOfElementInvasionVolumeCalculator::allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties ) {
  
   properties.push_back ((PropertyValue*)(FastcauldronSimulator::getInstance ().createVolumePropertyValue ( "TimeOfInvasion", 
                                                                                                            m_snapshot, 0,
                                                                                                            m_formation,
                                                                                                            m_formation->getMaximumNumberOfElements () + 1 )));
}

bool TimeOfElementInvasionVolumeCalculator::initialise ( OutputPropertyMap::PropertyValueList& propertyValues ) {

   if ( not FastcauldronSimulator::getInstance ().getMcfHandler ().containsFormation ( m_formation )) {

      OutputPropertyMap::PropertyValueList::iterator iter;

      for ( iter = propertyValues.begin (); iter != propertyValues.end (); ++iter ) {
         (*iter)->allowOutput ( false );
      }

   }

   return true;
}
