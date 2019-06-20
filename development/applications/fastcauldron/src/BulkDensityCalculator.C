#include "BulkDensityCalculator.h"
#include "DerivedOutputPropertyMap.h"
#include "PropertyManager.h"
#include "FastcauldronSimulator.h"


OutputPropertyMap* allocateBulkDensityCalculator ( const PropertyList property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) {
   return new DerivedOutputPropertyMap<BulkDensityCalculator>( property, formation, surface, snapshot );
}


OutputPropertyMap* allocateBulkDensityVolumeCalculator ( const PropertyList property, LayerProps* formation, const Interface::Snapshot* snapshot ) {
   return new DerivedOutputPropertyMap<BulkDensityVolumeCalculator>( property, formation, snapshot );
}

BulkDensityCalculator::BulkDensityCalculator ( LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) :
   m_formation ( formation ), m_surface ( surface ), m_snapshot ( snapshot ) {

   m_porosity = 0;
   m_pressure = 0;
   m_lithopressure = 0;
   m_temperature = 0;
   m_isCalculated = false;
   m_lithologies = 0;
   m_fluid = 0;
   m_isBasementFormationAndALC = false;

}

bool BulkDensityCalculator::operator ()( const OutputPropertyMap::OutputPropertyList& ,
                                               OutputPropertyMap::PropertyValueList&  propertyValues ) {

   if ( m_isCalculated ) {
      return true;
   }

   unsigned int i;
   unsigned int j;
   double value;
   double porosity;
   double undefinedValue;

   bool isHydrostaticDecompaction = FastcauldronSimulator::getInstance ().getCalculationMode () == HYDROSTATIC_DECOMPACTION_MODE;

   Interface::GridMap* bulkDensityMap;

   if ( not m_porosity->isCalculated ()) {

      if ( not m_porosity->calculate ()) {
         return false;
      }

   }

   if ( not isHydrostaticDecompaction and not m_pressure->isCalculated ()) {

      if ( not m_pressure->calculate ()) {
         return false;
      }

   }

   if ( not isHydrostaticDecompaction and not m_temperature->isCalculated ()) {

      if ( not m_temperature->calculate ()) {
         return false;
      }

   }

   if( m_isBasementFormationAndALC ) {
      if( not isHydrostaticDecompaction and not m_lithopressure->isCalculated ()) {
         if ( not m_lithopressure->calculate ()) {
            return false;
         }
      }
   }

   const CompoundLithology * curLithology;
   bulkDensityMap = propertyValues [ 0 ]->getGridMap ();
   bulkDensityMap->retrieveData ();
   undefinedValue = bulkDensityMap->getUndefinedValue ();

   for ( i = bulkDensityMap->firstI (); i <= bulkDensityMap->lastI (); ++i ) {

      for ( j = bulkDensityMap->firstJ (); j <= bulkDensityMap->lastJ (); ++j ) {

         if ( FastcauldronSimulator::getInstance ().nodeIsDefined ( i, j )) {
            porosity = 0.01 * (*m_porosity)( i, j );

            if( m_isBasementFormationAndALC ) {
               curLithology = m_formation->getLithology( i, j, m_kIndex );
            } else {
               curLithology = (*m_lithologies)( i, j );
            }

            if ( isHydrostaticDecompaction ) {
               curLithology->calcBulkDensity ( m_fluid, porosity, value );
            } else {
               curLithology->calcBulkDensity ( m_fluid, porosity, (*m_pressure)( i, j ), (*m_temperature)( i, j ),
                                               ( m_lithopressure != 0 ? (*m_lithopressure)( i, j ) : 0 ), value );
            }

            bulkDensityMap->setValue ( i, j, value );
         } else {
            bulkDensityMap->setValue ( i, j, undefinedValue );
         }

      }

   }

   bulkDensityMap->restoreData ();
   m_isCalculated = true;
   return true;
}

void BulkDensityCalculator::allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties ) {

   properties.push_back ((CauldronPropertyValue*)(FastcauldronSimulator::getInstance ().createMapPropertyValue ( "BulkDensityVec2",
                                                                                                         m_snapshot, 0,
                                                                                                         m_formation,
                                                                                                         m_surface )));

}

bool BulkDensityCalculator::initialise ( OutputPropertyMap::PropertyValueList& propertyValues ) {

   m_isBasementFormationAndALC = m_formation->isBasement() && FastcauldronSimulator::getInstance().isALC();
   m_porosity = PropertyManager::getInstance().findOutputPropertyMap ( "Porosity", m_formation, m_surface, m_snapshot );

   if ( FastcauldronSimulator::getInstance ().getCalculationMode () != HYDROSTATIC_DECOMPACTION_MODE ) {
      m_pressure = PropertyManager::getInstance().findOutputPropertyMap ( "Pressure", m_formation, m_surface, m_snapshot );
      m_temperature = PropertyManager::getInstance().findOutputPropertyMap ( "Temperature", m_formation, m_surface, m_snapshot );
      if( m_isBasementFormationAndALC ) {
         m_lithopressure = PropertyManager::getInstance().findOutputPropertyVolume ( "LithoStaticPressure", m_formation, m_snapshot );
      } else {
         m_lithopressure = 0;
      }
   } else {
      m_pressure = 0;
      m_temperature = 0;
   }

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
   m_lithologies = &m_formation->getCompoundLithologyArray ();
   m_fluid = m_formation->fluid;


   if ( FastcauldronSimulator::getInstance ().getCauldron()->no2Doutput()) {
      propertyValues [ 0 ]->allowOutput( false );
   }

   if ( FastcauldronSimulator::getInstance ().getCalculationMode () != HYDROSTATIC_DECOMPACTION_MODE ) {
      return m_porosity != 0 and m_pressure != 0 and m_temperature != 0 and m_lithologies != 0 and m_fluid != 0 and
         ( m_isBasementFormationAndALC ? m_lithopressure != 0 : true );
   } else {
      return m_porosity != 0 and m_lithologies != 0 and m_fluid != 0;
   }

}


BulkDensityVolumeCalculator::BulkDensityVolumeCalculator ( LayerProps* formation, const Interface::Snapshot* snapshot ) :
   m_formation ( formation ), m_snapshot ( snapshot ) {

   m_porosity = 0;
   m_pressure = 0;
   m_lithopressure = 0;
   m_temperature = 0;
   m_isCalculated = false;
   m_isBasementFormationAndALC = false;
   m_lithologies = 0;
   m_fluid = 0;


}

bool BulkDensityVolumeCalculator::operator ()( const OutputPropertyMap::OutputPropertyList& ,
                                                     OutputPropertyMap::PropertyValueList&  propertyValues ) {

   if ( m_isCalculated ) {
      return true;
   }

   unsigned int i;
   unsigned int j;
   unsigned int k;
   double value;
   double porosity;
   double undefinedValue;
   bool isHydrostaticDecompaction = FastcauldronSimulator::getInstance ().getCalculationMode () == HYDROSTATIC_DECOMPACTION_MODE;

   Interface::GridMap* bulkDensityMap;

   if ( not m_porosity->isCalculated ()) {

      if ( not m_porosity->calculate ()) {
         return false;
      }

   }

   if ( not isHydrostaticDecompaction and not m_pressure->isCalculated ()) {

      if ( not m_pressure->calculate ()) {
         return false;
      }

   }

   if ( not isHydrostaticDecompaction and not m_temperature->isCalculated ()) {

      if ( not m_temperature->calculate ()) {
         return false;
      }

   }

   if( m_isBasementFormationAndALC ) {
      if( not isHydrostaticDecompaction and not m_lithopressure->isCalculated ()) {
         if ( not m_lithopressure->calculate ()) {
            return false;
         }
      }
   }
   const CompoundLithology * curLithology;

   bulkDensityMap = propertyValues [ 0 ]->getGridMap ();
   bulkDensityMap->retrieveData ();
   undefinedValue = bulkDensityMap->getUndefinedValue ();

   for ( i = bulkDensityMap->firstI (); i <= bulkDensityMap->lastI (); ++i ) {

      for ( j = bulkDensityMap->firstJ (); j <= bulkDensityMap->lastJ (); ++j ) {

         for ( k = bulkDensityMap->firstK (); k <= bulkDensityMap->lastK (); ++k ) {

            if ( FastcauldronSimulator::getInstance ().nodeIsDefined ( i, j )) {
               porosity = 0.01 * m_porosity->getVolumeValue ( i, j, k );
               if( m_isBasementFormationAndALC ) {
                  curLithology = m_formation->getLithology( i, j, k );
               } else {
                  curLithology = (*m_lithologies)( i, j );
               }
               if ( isHydrostaticDecompaction ) {
                  curLithology->calcBulkDensity ( m_fluid, porosity, value );
               } else {
                  curLithology->calcBulkDensity ( m_fluid,
                                                  porosity,
                                                  m_pressure->getVolumeValue ( i, j, k ),
                                                  m_temperature->getVolumeValue ( i, j, k ),
                                                  (m_lithopressure != 0 ? m_lithopressure->getVolumeValue( i, j, k ) : 0 ),
                                                  value );
               }

               bulkDensityMap->setValue ( i, j, k, value );
            } else {
               bulkDensityMap->setValue ( i, j, k, undefinedValue );
            }

         }

      }

   }

   bulkDensityMap->restoreData ();
   m_isCalculated = true;
   return true;
}

void BulkDensityVolumeCalculator::allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties ) {

   properties.push_back ((CauldronPropertyValue*)(FastcauldronSimulator::getInstance ().createVolumePropertyValue ( "BulkDensity",
                                                                                                            m_snapshot, 0,
                                                                                                            m_formation,
                                                                                                            m_formation->getMaximumNumberOfElements () + 1 )));

}

bool BulkDensityVolumeCalculator::initialise ( OutputPropertyMap::PropertyValueList& ) {

   m_isBasementFormationAndALC = m_formation->isBasement() && FastcauldronSimulator::getInstance().isALC();
   m_porosity = PropertyManager::getInstance().findOutputPropertyVolume ( "Porosity", m_formation, m_snapshot );

   if ( FastcauldronSimulator::getInstance ().getCalculationMode () != HYDROSTATIC_DECOMPACTION_MODE ) {
      m_pressure = PropertyManager::getInstance().findOutputPropertyVolume ( "Pressure", m_formation, m_snapshot );
      m_temperature = PropertyManager::getInstance().findOutputPropertyVolume ( "Temperature", m_formation, m_snapshot );
      if( m_isBasementFormationAndALC ) {
         m_lithopressure = PropertyManager::getInstance().findOutputPropertyVolume ( "LithoStaticPressure", m_formation, m_snapshot );
      } else {
         m_lithopressure = 0;
      }
   } else {
      m_pressure = 0;
      m_temperature = 0;
   }

   m_lithologies = &m_formation->getCompoundLithologyArray ();
   m_fluid = m_formation->fluid;

   if ( FastcauldronSimulator::getInstance ().getCalculationMode () != HYDROSTATIC_DECOMPACTION_MODE ) {
      return m_porosity != 0 and m_pressure != 0 and m_temperature != 0 and m_lithologies != 0 and m_fluid != 0 and
         ( m_isBasementFormationAndALC ? m_lithopressure != 0 : true );
   } else {
      return m_porosity != 0 and m_lithologies != 0 and m_fluid != 0;
   }

}
