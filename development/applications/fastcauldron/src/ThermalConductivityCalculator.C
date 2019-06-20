#include "ThermalConductivityCalculator.h"
#include "DerivedOutputPropertyMap.h"
#include "PropertyManager.h"
#include "FastcauldronSimulator.h"

#include "RunParameters.h"

OutputPropertyMap* allocateThermalConductivityCalculator ( const PropertyList property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) {
   return new DerivedOutputPropertyMap<ThermalConductivityCalculator>( property, formation, surface, snapshot );
}

OutputPropertyMap* allocateThermalConductivityVolumeCalculator ( const PropertyList property, LayerProps* formation, const Interface::Snapshot* snapshot ) {
   return new DerivedOutputPropertyMap<ThermalConductivityVolumeCalculator>( property, formation, snapshot );
}

ThermalConductivityCalculator::ThermalConductivityCalculator ( LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) :
   m_formation ( formation ), m_surface ( surface ), m_snapshot ( snapshot ) {

   m_porosity = 0;
   m_temperature = 0;
   m_porePressure = 0;
   m_lithopressure = 0;
   m_isCalculated = false;

   m_lithologies = 0;
   m_fluid = 0;

   m_isBasementFormationAndALC = false;
}

bool ThermalConductivityCalculator::operator ()( const OutputPropertyMap::OutputPropertyList& properties,
                                                       OutputPropertyMap::PropertyValueList&  propertyValues ) {

   if ( m_isCalculated ) {
      return true;
   }

   unsigned int i;
   unsigned int j;
   double thermalConductivityNorm;
   double thermalConductivityPlane;
   double undefinedValue;

   Interface::GridMap* verticalThermalConductivityMap;
//    Interface::GridMap* horizontalThermalConductivityMap;

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

   if ( not m_porePressure->isCalculated ()) {

      if ( not m_porePressure->calculate ()) {
         return false;
      }

   }

   if ( m_isBasementFormationAndALC and not m_lithopressure->isCalculated ()) {

      if ( not m_lithopressure->calculate ()) {
         return false;
      }

   }

   verticalThermalConductivityMap = propertyValues [ 0 ]->getGridMap ();
   verticalThermalConductivityMap->retrieveData ();
   undefinedValue = verticalThermalConductivityMap->getUndefinedValue ();

//    horizontalThermalConductivityMap = propertyValues [ 1 ]->getGridMap ();
//    horizontalThermalConductivityMap->retrieveData ();
   const CompoundLithology *  curLithology;
   for ( i = verticalThermalConductivityMap->firstI (); i <= verticalThermalConductivityMap->lastI (); ++i ) {

      for ( j = verticalThermalConductivityMap->firstJ (); j <= verticalThermalConductivityMap->lastJ (); ++j ) {

         if ( FastcauldronSimulator::getInstance ().nodeIsDefined ( i, j )) {
            if ( m_isBasementFormationAndALC ) {
               curLithology = m_formation->getLithology(  i, j, m_kIndex );
               curLithology->calcBulkThermCondNPBasement ( m_fluid, 0.01 * (*m_porosity)( i, j ), (*m_temperature)( i, j ),
                                                           (*m_lithopressure)( i, j ), thermalConductivityNorm, thermalConductivityPlane );
            } else {
               (*m_lithologies)( i, j )->calcBulkThermCondNP ( m_fluid, 0.01 * (*m_porosity)( i, j ), (*m_temperature)( i, j ), (*m_porePressure)( i, j ),
                                                               thermalConductivityNorm, thermalConductivityPlane );
            }
            verticalThermalConductivityMap->setValue ( i, j, thermalConductivityNorm );
//             horizontalThermalConductivityMap->setValue ( i, j, thermalConductivityPlane );
         } else {
            verticalThermalConductivityMap->setValue ( i, j, undefinedValue );
//             horizontalThermalConductivityMap->setValue ( i, j, undefinedValue );
         }

      }

   }

   verticalThermalConductivityMap->restoreData ();
//    horizontalThermalConductivityMap->restoreData ();

   m_isCalculated = true;
   return true;
}

void ThermalConductivityCalculator::allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties ) {


   properties.push_back ((CauldronPropertyValue*)(FastcauldronSimulator::getInstance ().createMapPropertyValue ( "ThCondVec2",
                                                                                                         m_snapshot, 0,
                                                                                                         m_formation,
                                                                                                         m_surface )));


//    properties.push_back ((PropertyValue*)(FastcauldronSimulator::getInstance ().createMapPropertyValue ( "ThCondHVec2",
//                                                                                                          m_snapshot, 0,
//                                                                                                          m_formation,
//                                                                                                          m_surface )));

}

bool ThermalConductivityCalculator::initialise ( OutputPropertyMap::PropertyValueList& propertyValues ) {

   m_porosity = PropertyManager::getInstance().findOutputPropertyMap ( "Porosity", m_formation, m_surface, m_snapshot );
   m_temperature = PropertyManager::getInstance().findOutputPropertyMap ( "Temperature", m_formation, m_surface, m_snapshot );
   m_porePressure = PropertyManager::getInstance().findOutputPropertyMap ( "Pressure", m_formation, m_surface, m_snapshot );
   m_isBasementFormationAndALC = m_formation->isBasement() && FastcauldronSimulator::getInstance().isALC();

   if( m_isBasementFormationAndALC ) {
      m_lithopressure = PropertyManager::getInstance().findOutputPropertyMap ( "LithoStaticPressure", m_formation, m_surface, m_snapshot );
    } else {
      m_lithopressure = 0;
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
//    m_lithologies = &m_formation->Lithology;
   m_fluid = m_formation->fluid;

   if ( FastcauldronSimulator::getInstance ().getCauldron()->no2Doutput()) {
      propertyValues [ 0 ]->allowOutput ( false );
   }
   return m_porosity != 0 and m_temperature != 0 and m_lithologies != 0 and m_fluid != 0 and m_porePressure != 0 and
      ( m_isBasementFormationAndALC ? (m_lithopressure != 0) : true );
}



ThermalConductivityVolumeCalculator::ThermalConductivityVolumeCalculator ( LayerProps* formation, const Interface::Snapshot* snapshot ) :
   m_formation ( formation ), m_snapshot ( snapshot ) {

   m_porosity = 0;
   m_temperature = 0;
   m_lithopressure = 0;
   m_porePressure = 0;

   m_isCalculated = false;

   m_lithologies = 0;
   m_fluid = 0;

   m_isBasementFormationAndALC = false;
}

bool ThermalConductivityVolumeCalculator::operator ()( const OutputPropertyMap::OutputPropertyList& properties,
                                                             OutputPropertyMap::PropertyValueList&  propertyValues ) {

   if ( m_isCalculated ) {
      return true;
   }

   unsigned int i;
   unsigned int j;
   unsigned int k;
   double thermalConductivityNorm;
   double thermalConductivityPlane;
   double undefinedValue;

   Interface::GridMap* verticalThermalConductivityMap;
//    Interface::GridMap* horizontalThermalConductivityMap;

   if ( not m_porosity->isCalculated ()) {

      if ( not m_porosity->calculate ()) {
         return false;
      }

   }
   if ( m_isBasementFormationAndALC and not m_lithopressure->isCalculated ()) {

      if ( not m_lithopressure->calculate ()) {
         return false;
      }

   }

   if ( not m_temperature->isCalculated ()) {

      if ( not m_temperature->calculate ()) {
         return false;
      }

   }

   if ( not m_porePressure->isCalculated ()) {

      if ( not m_porePressure->calculate ()) {
         return false;
      }

   }

   verticalThermalConductivityMap = propertyValues [ 0 ]->getGridMap ();
   verticalThermalConductivityMap->retrieveData ();
   undefinedValue = verticalThermalConductivityMap->getUndefinedValue ();

//    horizontalThermalConductivityMap = propertyValues [ 1 ]->getGridMap ();
//    horizontalThermalConductivityMap->retrieveData ();
   const CompoundLithology * curLithology;
   for ( i = verticalThermalConductivityMap->firstI (); i <= verticalThermalConductivityMap->lastI (); ++i ) {

      for ( j = verticalThermalConductivityMap->firstJ (); j <= verticalThermalConductivityMap->lastJ (); ++j ) {

         if ( FastcauldronSimulator::getInstance ().nodeIsDefined ( i, j )) {

            for ( k = verticalThermalConductivityMap->firstK (); k <= verticalThermalConductivityMap->lastK (); ++k ) {
               if( m_isBasementFormationAndALC ) {
                  curLithology = m_formation->getLithology( i, j, k );
                  curLithology ->calcBulkThermCondNPBasement ( m_fluid, 0.01 * m_porosity->getVolumeValue ( i, j, k ),
                                                               m_temperature->getVolumeValue ( i, j, k ),
                                                               m_lithopressure->getVolumeValue ( i, j, k ),
                                                               thermalConductivityNorm,
                                                               thermalConductivityPlane );
               } else {
                  (*m_lithologies)( i, j )->calcBulkThermCondNP ( m_fluid, 0.01 * m_porosity->getVolumeValue ( i, j, k ),
                                                                  m_temperature->getVolumeValue ( i, j, k ),
                                                                  m_porePressure->getVolumeValue ( i, j, k ),
                                                                  thermalConductivityNorm,
                                                                  thermalConductivityPlane );
               }
               verticalThermalConductivityMap->setValue ( i, j, k, thermalConductivityNorm );
//                horizontalThermalConductivityMap->setValue ( i, j, k, thermalConductivityPlane );
            }

         } else {

            for ( k = verticalThermalConductivityMap->firstK (); k <= verticalThermalConductivityMap->lastK (); ++k ) {
               verticalThermalConductivityMap->setValue ( i, j, k, undefinedValue );
//                horizontalThermalConductivityMap->setValue ( i, j, k, undefinedValue );
            }

         }

      }

   }

   verticalThermalConductivityMap->restoreData ();
//    horizontalThermalConductivityMap->restoreData ();

   m_isCalculated = true;
   return true;
}

void ThermalConductivityVolumeCalculator::allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties ) {

   properties.push_back ((CauldronPropertyValue*)(FastcauldronSimulator::getInstance ().createVolumePropertyValue ( "ThCond",
                                                                                                            m_snapshot, 0,
                                                                                                            m_formation,
                                                                                                            m_formation->getMaximumNumberOfElements () + 1 )));


//    properties.push_back ((PropertyValue*)(FastcauldronSimulator::getInstance ().createMapPropertyValue ( "ThCondHVec2",
//                                                                                                          m_snapshot, 0,
//                                                                                                          m_formation,
//                                                                                                          m_surface )));

}

bool ThermalConductivityVolumeCalculator::initialise ( OutputPropertyMap::PropertyValueList& propertyValues ) {

   m_porosity = PropertyManager::getInstance().findOutputPropertyVolume ( "Porosity", m_formation, m_snapshot );
   m_temperature = PropertyManager::getInstance().findOutputPropertyVolume ( "Temperature", m_formation, m_snapshot );
   m_porePressure = PropertyManager::getInstance().findOutputPropertyVolume ( "Pressure", m_formation, m_snapshot );
   m_isBasementFormationAndALC = m_formation->isBasement() && FastcauldronSimulator::getInstance().isALC();

  if( m_isBasementFormationAndALC ) {
      m_lithopressure = PropertyManager::getInstance().findOutputPropertyVolume ( "LithoStaticPressure", m_formation, m_snapshot );
   } else {
      m_lithopressure = 0;
   }

   m_lithologies = &m_formation->getCompoundLithologyArray ();
//    m_lithologies = &m_formation->Lithology;
   m_fluid = m_formation->fluid;

   return m_porosity != 0 and m_temperature != 0 and m_lithologies != 0 and m_fluid != 0 and m_porePressure != 0 and
      (m_isBasementFormationAndALC ? m_lithopressure != 0 : true);
}
