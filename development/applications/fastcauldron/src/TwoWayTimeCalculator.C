#include "TwoWayTimeCalculator.h"

#include "DerivedOutputPropertyMap.h"
#include "FastcauldronSimulator.h"
#include "PropertyManager.h"

OutputPropertyMap* allocateTwoWayTimeCalculator ( const PropertyList property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) {
   return new DerivedOutputPropertyMap<TwoWayTimeCalculator>( property, formation, surface, snapshot );
}

OutputPropertyMap* allocateTwoWayTimeVolumeCalculator ( const PropertyList property, LayerProps* formation, const Interface::Snapshot* snapshot ) {
   return new DerivedOutputPropertyMap<TwoWayTimeVolumeCalculator>( property, formation, snapshot );
}

TwoWayTimeCalculator::TwoWayTimeCalculator ( LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) :
   m_formation ( formation ), m_surface ( surface ), m_snapshot ( snapshot ) {

   m_seismicVelocity = 0;
   m_isCalculated = false;

}

bool TwoWayTimeCalculator::operator ()( const OutputPropertyMap::OutputPropertyList& properties, 
                                            OutputPropertyMap::PropertyValueList&  propertyValues ) {

   if ( m_isCalculated ) {
      return true;
   }

   unsigned int i;
   unsigned int j;
   double value;
   double undefinedValue;
   Interface::GridMap* TwoWayTimeMap;

   if (not m_seismicVelocity->isCalculated()) {

      if (not m_seismicVelocity->calculate()) {
         return false;
      }

   }

   TwoWayTimeMap = propertyValues [ 0 ]->getGridMap ();
   TwoWayTimeMap->retrieveData ();
   undefinedValue = TwoWayTimeMap->getUndefinedValue ();

   for ( i = TwoWayTimeMap->firstI (); i <= TwoWayTimeMap->lastI (); ++i ) {

      for ( j = TwoWayTimeMap->firstJ (); j <= TwoWayTimeMap->lastJ (); ++j ) {

         if ( FastcauldronSimulator::getInstance ().nodeIsDefined ( i, j )) {

            value = 0;

            TwoWayTimeMap->setValue ( i, j, value );
         } else {
            TwoWayTimeMap->setValue ( i, j, undefinedValue );
         }

      }

   }

   TwoWayTimeMap->restoreData ();
   m_isCalculated = true;
   return true;
}

void TwoWayTimeCalculator::allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties ) {

   properties.push_back ((PropertyValue*)(FastcauldronSimulator::getInstance ().createMapPropertyValue ( "TwoWayTime", 
                                                                                                         m_snapshot, 0, 
                                                                                                         m_formation,
                                                                                                         m_surface )));

}

bool TwoWayTimeCalculator::initialise ( OutputPropertyMap::PropertyValueList& propertyValues ) {

   m_seismicVelocity = PropertyManager::getInstance().findOutputPropertyMap("Velocity", m_formation, m_surface, m_snapshot);

   if ( FastcauldronSimulator::getInstance ().getCauldron()->no2Doutput()) {
      propertyValues [ 0 ]->allowOutput ( false );
   }
   return m_seismicVelocity != 0;
}


TwoWayTimeVolumeCalculator::TwoWayTimeVolumeCalculator ( LayerProps* formation, const Interface::Snapshot* snapshot ) :
   m_formation ( formation ), m_snapshot ( snapshot ) {

   m_seismicVelocity = 0;
   m_isCalculated = false;
}

bool TwoWayTimeVolumeCalculator::operator ()( const OutputPropertyMap::OutputPropertyList& properties, 
                                                  OutputPropertyMap::PropertyValueList&  propertyValues ) {

   if ( m_isCalculated ) {
      return true;
   }

   unsigned int i;
   unsigned int j;
   unsigned int k;
   double value;
   double undefinedValue;
   Interface::GridMap* TwoWayTimeMap;

   if (not m_seismicVelocity->isCalculated()) {

      if (not m_seismicVelocity->calculate()) {
         return false;
      }

   }

   TwoWayTimeMap = propertyValues [ 0 ]->getGridMap ();
   TwoWayTimeMap->retrieveData ();
   undefinedValue = TwoWayTimeMap->getUndefinedValue ();

   for ( i = TwoWayTimeMap->firstI (); i <= TwoWayTimeMap->lastI (); ++i ) {

      for ( j = TwoWayTimeMap->firstJ (); j <= TwoWayTimeMap->lastJ (); ++j ) {

         if ( FastcauldronSimulator::getInstance ().nodeIsDefined ( i, j )) {
            
            for ( k = TwoWayTimeMap->firstK (); k <= TwoWayTimeMap->lastK (); ++k ) {

				   value = 0;

               TwoWayTimeMap->setValue ( i, j, k, value );
            }

         } else {

            for ( k = TwoWayTimeMap->firstK (); k <= TwoWayTimeMap->lastK (); ++k ) {
               TwoWayTimeMap->setValue ( i, j, k, undefinedValue );
            }

         }

      }

   }

   TwoWayTimeMap->restoreData ();
   m_isCalculated = true;
   return true;
}

void TwoWayTimeVolumeCalculator::allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties ) {

   properties.push_back ((PropertyValue*)(FastcauldronSimulator::getInstance ().createVolumePropertyValue ( "TwoWayTime",
                                                                                                            m_snapshot, 0,
                                                                                                            m_formation,
                                                                                                            m_formation->getMaximumNumberOfElements () + 1 )));

}

bool TwoWayTimeVolumeCalculator::initialise ( OutputPropertyMap::PropertyValueList& propertyValues ) {

   m_seismicVelocity = PropertyManager::getInstance().findOutputPropertyVolume("Velocity", m_formation, m_snapshot);

   return m_seismicVelocity != 0;
}
