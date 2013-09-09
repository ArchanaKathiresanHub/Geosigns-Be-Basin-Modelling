#include "LithostaticPressureCalculator.h"

#include "PrimaryOutputPropertyVolume.h"
#include "PrimaryOutputPropertyMap.h"
#include "DerivedOutputPropertyMap.h"
#include "PropertyManager.h"
#include "FastcauldronSimulator.h"

OutputPropertyMap* allocateLithostaticPressureCalculator ( const PropertyList property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) {

   if ( formation->kind () == Interface::SEDIMENT_FORMATION ) {
      return new PrimaryOutputPropertyMap ( property, formation, surface, snapshot );
   } else {
      // The lithostatic-pressure is not a primary property if the formation is not a sediment.
      return new DerivedOutputPropertyMap<BasementLithostaticPressureCalculator>( property, formation, surface, snapshot );
   }

}

OutputPropertyMap* allocateLithostaticPressureVolumeCalculator ( const PropertyList property, LayerProps* formation, const Interface::Snapshot* snapshot ) {

   if ( formation->kind () == Interface::SEDIMENT_FORMATION ) {
      return new PrimaryOutputPropertyVolume ( property, formation, snapshot );
   } else {
      // The lithostatic-pressure is not a primary property if the formation is not a sediment.
      return new DerivedOutputPropertyMap<BasementLithostaticPressureVolumeCalculator>( property, formation, snapshot );
   }

}


BasementLithostaticPressureCalculator::BasementLithostaticPressureCalculator ( LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) :
   m_formation ( formation ), m_surface ( surface ), m_snapshot ( snapshot ) {

   m_isCalculated = false;
   m_lithologies = 0;
}




bool BasementLithostaticPressureCalculator::operator ()( const OutputPropertyMap::OutputPropertyList& properties, 
                                                               OutputPropertyMap::PropertyValueList&  propertyValues ) {

   if ( m_isCalculated ) {
      return true;
   }

   unsigned int i;
   unsigned int j;
   int k;
   double value;
   double undefinedValue;
   double segmentThickness;
   Interface::GridMap* lithostaticPressureMap;
   OutputPropertyMap* topPressure;

   lithostaticPressureMap = propertyValues [ 0 ]->getGridMap ();

   const Interface::Surface* surfaceAbove = m_formation->getTopSurface ();

   if ( m_snapshot->getTime () < m_formation->depoage ) {
      topPressure = PropertyManager::getInstance().findOutputPropertyMap ( "LithoStaticPressure", 0, surfaceAbove, m_snapshot );
   } else {
      topPressure = PropertyManager::getInstance().findOutputPropertyMap ( "HydroStaticPressure", 0, surfaceAbove, m_snapshot );
   }

   if ( topPressure != 0 and not topPressure->isCalculated ()) {

      if ( not topPressure->calculate ()) {
         return false;
      } 

   }
   lithostaticPressureMap->retrieveData ();

  
   // Set the lithostatic pressure at the top of the formation to the same as the bottom of the formation above.
   for ( i = lithostaticPressureMap->firstI (); i <= lithostaticPressureMap->lastI (); ++i ) {
      
      for ( j = lithostaticPressureMap->firstJ (); j <= lithostaticPressureMap->lastJ (); ++j ) {
         
         if ( FastcauldronSimulator::getInstance ().nodeIsDefined ( i, j )) {
            lithostaticPressureMap->setValue ( i, j, (*topPressure)( i, j ));
         } else {
            lithostaticPressureMap->setValue ( i, j, undefinedValue );
         }
      }
   }
      
   if( m_kIndex == 0 ) { // for bottom surface of the formation

      PETSC_3D_Array layerTemperature  ( m_formation->layerDA, m_formation->Current_Properties ( Basin_Modelling::Temperature ), INSERT_VALUES, true );
      PETSC_3D_Array layerDepth        ( m_formation->layerDA, m_formation->Current_Properties ( Basin_Modelling::Depth ), INSERT_VALUES, true );
      
      double density, lithoPressureAbove;
      
      undefinedValue = lithostaticPressureMap->getUndefinedValue ();
      
      for ( i = lithostaticPressureMap->firstI (); i <= lithostaticPressureMap->lastI (); ++i ) {
         
         for ( j = lithostaticPressureMap->firstJ (); j <= lithostaticPressureMap->lastJ (); ++j ) {
            
            if ( FastcauldronSimulator::getInstance ().nodeIsDefined ( i, j )) {
               lithoPressureAbove = ( value = lithostaticPressureMap->getValue( i, j ) );
               
               for ( k = m_formation->getMaximumNumberOfElements (); k > 0 ; -- k ) {
                  
                  segmentThickness = layerDepth( k - 1, j, i ) - layerDepth( k, j, i );
                  
                  density =  m_formation->getLithology( i, j, k - 1 )->getSimpleLithology()->getDensity ( layerTemperature( k - 1, j, i ), lithoPressureAbove );
                  
                  value = lithoPressureAbove + density * GRAVITY * Pa_To_MPa * segmentThickness;
                  lithoPressureAbove = value;
               }   
               lithostaticPressureMap->setValue ( i, j, value );
            } else {
               lithostaticPressureMap->setValue ( i, j, undefinedValue );
            }
         }
      }
   }


   lithostaticPressureMap->restoreData ();
   m_isCalculated = true;
   return true;
}

void BasementLithostaticPressureCalculator::allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties ) {

   PropertyValue* lithostaticPressure = (PropertyValue*)(FastcauldronSimulator::getInstance ().createMapPropertyValue ( "LithoStaticPressure", 
                                                                                                                        m_snapshot, 0, 0,
                                                                                                                        m_surface ));
   properties.push_back ( lithostaticPressure );
                                                   
   if ( FastcauldronSimulator::getInstance ().getCalculationMode () == HYDROSTATIC_DECOMPACTION_MODE ||
        FastcauldronSimulator::getInstance ().getCalculationMode () == COUPLED_HIGH_RES_DECOMPACTION_MODE ||
        FastcauldronSimulator::getInstance ().getCalculationMode () == HYDROSTATIC_HIGH_RES_DECOMPACTION_MODE ) {
      lithostaticPressure->allowOutput ( false );
   }
       
}

bool BasementLithostaticPressureCalculator::initialise ( OutputPropertyMap::PropertyValueList& propertyValues ) {

   m_lithologies = &m_formation->getCompoundLithologyArray ();
   if ( m_formation->getTopSurface () == m_surface ) {
      // Number-of-nodes = number-of-elements + 1.
      // But C array indexing starts a 0, so 1 must be subtracted.
      m_kIndex = m_formation->getMaximumNumberOfElements ();
   } else if ( m_formation->getBottomSurface () == m_surface ) {
      m_kIndex = 0;
   } else {
      assert ( false );
      // Error
   }
   
   return m_formation->kind () == Interface::BASEMENT_FORMATION and m_lithologies != 0;
}


//------------------------------------------------------------//


BasementLithostaticPressureVolumeCalculator::BasementLithostaticPressureVolumeCalculator ( LayerProps* formation, const Interface::Snapshot* snapshot ) :
   m_formation ( formation ), m_snapshot ( snapshot ) {

   m_depth = 0;
   m_isCalculated = false;
   m_lithologies = 0;

}

bool BasementLithostaticPressureVolumeCalculator::operator ()( const OutputPropertyMap::OutputPropertyList& properties, 
                                                                     OutputPropertyMap::PropertyValueList&  propertyValues ) {

   if ( m_isCalculated ) {
      return true;
   }

   unsigned int i;
   unsigned int j;
   unsigned int k;
   double value;
   double undefinedValue;
   double segmentThickness;
   Interface::GridMap* lithostaticPressureMap;
   OutputPropertyMap* topPressure;

   if ( not m_depth->isCalculated ()) {

      if ( not m_depth->calculate ()) {
         return false;
      }

   }

   lithostaticPressureMap = propertyValues [ 0 ]->getGridMap ();

   const Interface::Surface* surfaceAbove = m_formation->getTopSurface ();

   if ( m_snapshot->getTime () < m_formation->depoage ) {
      topPressure = PropertyManager::getInstance().findOutputPropertyMap ( "LithoStaticPressure", 0, surfaceAbove, m_snapshot );
   } else {
      topPressure = PropertyManager::getInstance().findOutputPropertyMap ( "HydroStaticPressure", 0, surfaceAbove, m_snapshot );
   }

   if ( topPressure != 0 and not topPressure->isCalculated ()) {

      if ( not topPressure->calculate ()) {
         return false;
      }

   }
   PETSC_3D_Array layerTemperature
      ( m_formation->layerDA,
        m_formation->Current_Properties ( Basin_Modelling::Temperature ),
        INSERT_VALUES, true );
   
   double density;

   lithostaticPressureMap->retrieveData ();
   undefinedValue = lithostaticPressureMap->getUndefinedValue ();

   // Set the lithostatic pressure at the top of the formation to the same as the bottom of the formation above.
   k = lithostaticPressureMap->lastK ();

   for ( i = lithostaticPressureMap->firstI (); i <= lithostaticPressureMap->lastI (); ++i ) {

      for ( j = lithostaticPressureMap->firstJ (); j <= lithostaticPressureMap->lastJ (); ++j ) {

         if ( FastcauldronSimulator::getInstance ().nodeIsDefined ( i, j )) {
            lithostaticPressureMap->setValue ( i, j, k, (*topPressure)( i, j ));
         } else {
            lithostaticPressureMap->setValue ( i, j, k, undefinedValue );
         }

      }

   }

   // Now fill in the rest of the volume lithostatic pressure.
   for ( i = lithostaticPressureMap->firstI (); i <= lithostaticPressureMap->lastI (); ++i ) {

      for ( j = lithostaticPressureMap->firstJ (); j <= lithostaticPressureMap->lastJ (); ++j ) {

         for ( k = lithostaticPressureMap->lastK (); k >= lithostaticPressureMap->firstK () + 1; --k ) {

            if ( FastcauldronSimulator::getInstance ().nodeIsDefined ( i, j )) {
               segmentThickness = m_depth->getVolumeValue ( i, j, k - 1 ) - m_depth->getVolumeValue ( i, j, k );

               density =  m_formation->getLithology( i, j, k - 1 )->getSimpleLithology()->getDensity ( layerTemperature( k - 1, j, i ), lithostaticPressureMap->getValue ( i, j, k ) );

               value = lithostaticPressureMap->getValue ( i, j, k ) + density * GRAVITY * Pa_To_MPa * segmentThickness;
               lithostaticPressureMap->setValue ( i, j, k - 1, value );
            } else {
               lithostaticPressureMap->setValue ( i, j, k - 1, undefinedValue );
            }

         }

      }

   }

   lithostaticPressureMap->restoreData ();
   m_isCalculated = true;
   return true;
}

void BasementLithostaticPressureVolumeCalculator::allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties ) {

    PropertyValue* lithostaticPressure = (PropertyValue*)(FastcauldronSimulator::getInstance ().createVolumePropertyValue ( "LithoStaticPressure", 
                                                                                                                            m_snapshot, 0, 
                                                                                                                            m_formation,
                                                                                                                            m_formation->getMaximumNumberOfElements () + 1 ));
    properties.push_back ( lithostaticPressure );
    
    if ( FastcauldronSimulator::getInstance ().getCalculationMode () == HYDROSTATIC_DECOMPACTION_MODE ||
         FastcauldronSimulator::getInstance ().getCalculationMode () == COUPLED_HIGH_RES_DECOMPACTION_MODE ||
         FastcauldronSimulator::getInstance ().getCalculationMode () == HYDROSTATIC_HIGH_RES_DECOMPACTION_MODE ) {
       lithostaticPressure->allowOutput ( false );
    }
}

bool BasementLithostaticPressureVolumeCalculator::initialise ( OutputPropertyMap::PropertyValueList& propertyValues ) {

   m_depth = PropertyManager::getInstance().findOutputPropertyVolume ( "Depth", m_formation, m_snapshot );

   m_lithologies = &m_formation->getCompoundLithologyArray ();

   return m_formation->kind () == Interface::BASEMENT_FORMATION and m_depth != 0 and m_lithologies != 0;
}


