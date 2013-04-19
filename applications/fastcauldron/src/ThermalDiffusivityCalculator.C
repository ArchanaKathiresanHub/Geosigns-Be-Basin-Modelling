#include "ThermalDiffusivityCalculator.h"
#include "DerivedOutputPropertyMap.h"
#include "PropertyManager.h"
#include "FastcauldronSimulator.h"

#include "Interface/RunParameters.h"

OutputPropertyMap* allocateThermalDiffusivityCalculator ( const PropertyList property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) {
   return new DerivedOutputPropertyMap<ThermalDiffusivityCalculator>( property, formation, surface, snapshot );
}

OutputPropertyMap* allocateThermalDiffusivityVolumeCalculator ( const PropertyList property, LayerProps* formation, const Interface::Snapshot* snapshot ) {
   return new DerivedOutputPropertyMap<ThermalDiffusivityVolumeCalculator>( property, formation, snapshot );
}

ThermalDiffusivityCalculator::ThermalDiffusivityCalculator ( LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) :
   m_formation ( formation ), m_surface ( surface ), m_snapshot ( snapshot ) {

   m_porosity = 0;
   m_temperature = 0;
   m_pressure = 0;
   m_lithopressure = 0;

   m_isCalculated = false;

   m_lithologies = 0;
   m_fluid = 0;

   m_isBasementFormationAndALC = false;
}

bool ThermalDiffusivityCalculator::operator ()( const OutputPropertyMap::OutputPropertyList& properties, 
                                                      OutputPropertyMap::PropertyValueList&  propertyValues ) {

   if ( m_isCalculated ) {
      return true;
   }

   unsigned int i;
   unsigned int j;
   double porosity;
   double bulkDensityHeatCapacity;
   double thermalConductivityNormal;
   double thermalConductivityPlane;
   double undefinedValue;

   Interface::GridMap* verticalThermalDiffusivityMap;
//    Interface::GridMap* horizontalThermalDiffusivityMap;

   if ( not m_porosity->isCalculated ()) {

      if ( not m_porosity->calculate ()) {
         return false;
      }

   }

   if ( not m_temperature->isCalculated ()) {

      if ( not m_temperature->calculate ()) {
         return false;
      } 

   }

   if ( not m_pressure->isCalculated ()) {

      if ( not m_pressure->calculate ()) {
         return false;
      } 

   }

   if ( m_isBasementFormationAndALC and not m_lithopressure->isCalculated ()) {

      if ( not m_lithopressure->calculate ()) {
         return false;
      } 

   }
   const CompoundLithology * curLithology;

   verticalThermalDiffusivityMap = propertyValues [ 0 ]->getGridMap ();
   verticalThermalDiffusivityMap->retrieveData ();

   // The assumption here is that all the maps will have the same undefined-value.
   undefinedValue = verticalThermalDiffusivityMap->getUndefinedValue ();

//    horizontalThermalDiffusivityMap = propertyValues [ 1 ]->getGridMap ();
//    horizontalThermalDiffusivityMap->retrieveData ();

   for ( i = verticalThermalDiffusivityMap->firstI (); i <= verticalThermalDiffusivityMap->lastI (); ++i ) {

      for ( j = verticalThermalDiffusivityMap->firstJ (); j <= verticalThermalDiffusivityMap->lastJ (); ++j ) {

         if ( FastcauldronSimulator::getInstance ().nodeIsDefined ( i, j )) {
            porosity = 0.01 * (*m_porosity)( i, j );

            if(  m_isBasementFormationAndALC ) {
               curLithology = m_formation->getLithology(i, j, m_kIndex);
               curLithology ->calcBulkDensXHeatCapacity ( m_fluid, porosity, (*m_pressure)( i, j ), (*m_temperature)( i, j ),
                                                          (*m_lithopressure)( i, j ), bulkDensityHeatCapacity );
               curLithology->calcBulkThermCondNPBasement ( m_fluid, porosity, (*m_temperature)( i, j ), (*m_lithopressure)( i, j ), 
                                                           thermalConductivityNormal, thermalConductivityPlane );
            } else {
               curLithology = (*m_lithologies)( i, j );
               curLithology ->calcBulkDensXHeatCapacity ( m_fluid, porosity, (*m_pressure)( i, j ), (*m_temperature)( i, j ), 0.0,  bulkDensityHeatCapacity );
               curLithology->calcBulkThermCondNP ( m_fluid, porosity, (*m_temperature)( i, j ), thermalConductivityNormal, thermalConductivityPlane );
            }
            
            //             (*m_lithologies)( i, j )->calcBulkDensXHeatCapacity ( m_fluid, porosity, (*m_pressure)( i, j ), (*m_temperature)( i, j ), bulkDensityHeatCapacity );
            //             (*m_lithologies)( i, j )->calcBulkThermCondNP ( m_fluid, porosity, (*m_temperature)( i, j ), thermalConductivityNormal, thermalConductivityPlane );
            
            if ( bulkDensityHeatCapacity != undefinedValue and
                 thermalConductivityPlane != undefinedValue and
                 bulkDensityHeatCapacity != 0.0 ) {
               verticalThermalDiffusivityMap->setValue ( i, j, thermalConductivityNormal / bulkDensityHeatCapacity );
            } else {
               verticalThermalDiffusivityMap->setValue ( i, j, undefinedValue );
            } 

//             horizontalThermalDiffusivityMap->setValue ( i, j, thermalThermalDiffusivityPlane );
         } else {
            verticalThermalDiffusivityMap->setValue ( i, j, undefinedValue );
//             horizontalThermalDiffusivityMap->setValue ( i, j, undefinedValue );
         }

      }

   }

   verticalThermalDiffusivityMap->restoreData ();
//    horizontalThermalDiffusivityMap->restoreData ();

   m_isCalculated = true;
   return true;
}

void ThermalDiffusivityCalculator::allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties ) {

   properties.push_back ((PropertyValue*)(FastcauldronSimulator::getInstance ().createMapPropertyValue ( "DiffusivityVec2", 
                                                                                                         m_snapshot, 0, 
                                                                                                         m_formation,
                                                                                                         m_surface )));


}

bool ThermalDiffusivityCalculator::initialise ( OutputPropertyMap::PropertyValueList& propertyValues ) {

   m_BasinModel = const_cast<AppCtx*>(FastcauldronSimulator::getInstance().getCauldron());
   m_porosity = PropertyManager::getInstance().findOutputPropertyMap ( "Porosity", m_formation, m_surface, m_snapshot );
   m_temperature = PropertyManager::getInstance().findOutputPropertyMap ( "Temperature", m_formation, m_surface, m_snapshot );
   m_pressure = PropertyManager::getInstance().findOutputPropertyMap ( "Pressure", m_formation, m_surface, m_snapshot );
   m_isBasementFormationAndALC = m_formation->kind() == Interface::BASEMENT_FORMATION && m_BasinModel->isALC();

   m_lithologies = &m_formation->getCompoundLithologyArray ();
   if( m_isBasementFormationAndALC ) {
      m_lithopressure = PropertyManager::getInstance().findOutputPropertyMap ( "LithoStaticPressure", m_formation, m_surface, m_snapshot );
   } else {
      m_lithopressure = 0;
   } 
//    m_lithologies = &m_formation->Lithology;
   m_fluid = m_formation->fluid;

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
	
   return m_porosity != 0 and m_temperature != 0 and m_pressure != 0  and m_lithologies != 0 and m_fluid != 0 and
      ( m_isBasementFormationAndALC ? m_lithopressure != 0 : true );
;
}



ThermalDiffusivityVolumeCalculator::ThermalDiffusivityVolumeCalculator ( LayerProps* formation, const Interface::Snapshot* snapshot ) :
   m_formation ( formation ), m_snapshot ( snapshot ) {

   m_porosity = 0;
   m_temperature = 0;
   m_pressure = 0;
   m_lithopressure = 0;

   m_isCalculated = false;

   m_lithologies = 0;
   m_fluid = 0;

   m_isBasementFormationAndALC = false;
}

bool ThermalDiffusivityVolumeCalculator::operator ()( const OutputPropertyMap::OutputPropertyList& properties, 
                                                            OutputPropertyMap::PropertyValueList&  propertyValues ) {

   if ( m_isCalculated ) {
      return true;
   }

   unsigned int i;
   unsigned int j;
   unsigned int k;
   double undefinedValue;
   double porosity;
   double bulkDensityHeatCapacity;
   double thermalConductivityNormal;
   double thermalConductivityPlane;

   Interface::GridMap* verticalThermalDiffusivityMap;
//    Interface::GridMap* horizontalThermalDiffusivityMap;

   if ( not m_porosity->isCalculated ()) {

      if ( not m_porosity->calculate ()) {
         return false;
      }

   }

   if ( not m_temperature->isCalculated ()) {

      if ( not m_temperature->calculate ()) {
         return false;
      } 

   }

   if ( not m_pressure->isCalculated ()) {

      if ( not m_pressure->calculate ()) {
         return false;
      } 

   }

   if ( m_isBasementFormationAndALC and not m_lithopressure->isCalculated ()) {

      if ( not m_lithopressure->calculate ()) {
         return false;
      } 

   }

   verticalThermalDiffusivityMap = propertyValues [ 0 ]->getGridMap ();
   verticalThermalDiffusivityMap->retrieveData ();

   // The assumption here is that all the maps will have the same undefined-value.
   undefinedValue = verticalThermalDiffusivityMap->getUndefinedValue ();

//    horizontalThermalDiffusivityMap = propertyValues [ 1 ]->getGridMap ();
//    horizontalThermalDiffusivityMap->retrieveData ();
   const CompoundLithology * curLithology;

   for ( i = verticalThermalDiffusivityMap->firstI (); i <= verticalThermalDiffusivityMap->lastI (); ++i ) {

      for ( j = verticalThermalDiffusivityMap->firstJ (); j <= verticalThermalDiffusivityMap->lastJ (); ++j ) {

         if ( FastcauldronSimulator::getInstance ().nodeIsDefined ( i, j )) {

             for ( k = verticalThermalDiffusivityMap->firstK (); k <= verticalThermalDiffusivityMap->lastK (); ++k ) {

                porosity = 0.01 * m_porosity->getVolumeValue ( i, j, k );
                if( m_isBasementFormationAndALC ) {
                   curLithology = m_formation->getLithology( i, j, k );
                   curLithology->calcBulkDensXHeatCapacity ( m_fluid, porosity,
                                                             m_pressure->getVolumeValue ( i, j, k ),
                                                             m_temperature->getVolumeValue ( i, j, k ),
                                                             m_lithopressure->getVolumeValue( i, j, k ),
                                                             bulkDensityHeatCapacity );
                   curLithology->calcBulkThermCondNPBasement ( m_fluid, porosity,
                                                               m_temperature->getVolumeValue ( i, j, k ),
                                                               m_lithopressure->getVolumeValue ( i, j, k ),
                                                               thermalConductivityNormal,
                                                               thermalConductivityPlane );
                } else {
                   m_formation->getLithology ( i, j )->calcBulkDensXHeatCapacity ( m_fluid, porosity,
                                                                                   m_pressure->getVolumeValue ( i, j, k ),
                                                                                   m_temperature->getVolumeValue ( i, j, k ),
                                                                                   0.0, bulkDensityHeatCapacity );
                   m_formation->getLithology ( i, j )->calcBulkThermCondNP ( m_fluid, porosity,
                                                                             m_temperature->getVolumeValue ( i, j, k ),
                                                                             thermalConductivityNormal,
                                                                             thermalConductivityPlane );
                }
                
                
//                (*m_lithologies)( i, j )->calcBulkDensXHeatCapacity ( m_fluid, porosity,
//                                                                      m_pressure->getVolumeValue ( i, j, k ),
//                                                                      m_temperature->getVolumeValue ( i, j, k ),
//                                                                      bulkDensityHeatCapacity );
//                (*m_lithologies)( i, j )->calcBulkThermCondNP ( m_fluid, porosity,
//                                                                m_temperature->getVolumeValue ( i, j, k ),
//                                                                thermalConductivityNormal,
//                                                                thermalConductivityPlane );

               if ( bulkDensityHeatCapacity != undefinedValue and
                    thermalConductivityPlane != undefinedValue and
                    bulkDensityHeatCapacity != 0.0 ) {
                  verticalThermalDiffusivityMap->setValue ( i, j, k, thermalConductivityNormal / bulkDensityHeatCapacity );
               } else {
                  verticalThermalDiffusivityMap->setValue ( i, j, k, undefinedValue );
               } 

            }

//             horizontalThermalDiffusivityMap->setValue ( i, j, thermalThermalDiffusivityPlane );
         } else {

            for ( k = verticalThermalDiffusivityMap->firstK (); k <= verticalThermalDiffusivityMap->lastK (); ++k ) {
               verticalThermalDiffusivityMap->setValue ( i, j, k, undefinedValue );
//               horizontalThermalDiffusivityMap->setValue ( i, j, undefinedValue );
            }

         }

      }

   }

   verticalThermalDiffusivityMap->restoreData ();
//    horizontalThermalDiffusivityMap->restoreData ();

   m_isCalculated = true;
   return true;
}

void ThermalDiffusivityVolumeCalculator::allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties ) {

   properties.push_back ((PropertyValue*)(FastcauldronSimulator::getInstance ().createVolumePropertyValue ( "Diffusivity", 
                                                                                                            m_snapshot, 0, 
                                                                                                            m_formation,
                                                                                                            m_formation->getMaximumNumberOfElements () + 1 )));


}

bool ThermalDiffusivityVolumeCalculator::initialise ( OutputPropertyMap::PropertyValueList& propertyValues ) {

   m_BasinModel = const_cast<AppCtx*>(FastcauldronSimulator::getInstance().getCauldron());
   m_porosity = PropertyManager::getInstance().findOutputPropertyVolume ( "Porosity", m_formation, m_snapshot );
   m_temperature = PropertyManager::getInstance().findOutputPropertyVolume ( "Temperature", m_formation, m_snapshot );
   m_pressure = PropertyManager::getInstance().findOutputPropertyVolume ( "Pressure", m_formation, m_snapshot );
   m_isBasementFormationAndALC = m_formation->kind() == Interface::BASEMENT_FORMATION && m_BasinModel->isALC();
   if( m_isBasementFormationAndALC ) {
      m_lithopressure = PropertyManager::getInstance().findOutputPropertyVolume ( "LithoStaticPressure", m_formation, m_snapshot );
   } else {
      m_lithopressure = 0;
   }

   m_lithologies = &m_formation->getCompoundLithologyArray ();
//    m_lithologies = &m_formation->Lithology;
   m_fluid = m_formation->fluid;

   return m_porosity != 0 and m_temperature != 0 and m_pressure != 0 and m_lithologies != 0 and m_fluid != 0 and
      ( m_isBasementFormationAndALC ? m_lithopressure != 0 : true ) ;
}
