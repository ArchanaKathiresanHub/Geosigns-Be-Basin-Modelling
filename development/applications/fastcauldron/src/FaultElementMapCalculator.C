#include "FaultElementMapCalculator.h"
#include "DerivedOutputPropertyMap.h"
#include "PropertyManager.h"
#include "FastcauldronSimulator.h"

OutputPropertyMap* allocateFaultElementMapCalculator ( const PropertyList property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) {
   return new DerivedOutputPropertyMap<FaultElementMapCalculator>( property, formation, surface, snapshot );
}

FaultElementMapCalculator::FaultElementMapCalculator ( LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) :
   m_formation ( formation ), m_surface ( surface ), m_snapshot ( snapshot ) {

   m_isCalculated = false;

}

bool FaultElementMapCalculator::operator ()( const OutputPropertyMap::OutputPropertyList& properties, 
                                                   OutputPropertyMap::PropertyValueList&  propertyValues ) {

   if ( m_isCalculated ) {
      return true;
   }

   if ( not m_formation->getContainsFaults () || 
        FastcauldronSimulator::getInstance ().getCauldron()->no2Doutput()) {
      propertyValues [ 0 ]->allowOutput ( false );
      m_isCalculated = true;
      return true;
   }

   unsigned int i;
   unsigned int j;
   double value;
   double undefinedValue;
   Interface::GridMap* faultElementMap;

   faultElementMap = propertyValues [ 0 ]->getGridMap ();
   faultElementMap->retrieveData ();
   undefinedValue = faultElementMap->getUndefinedValue ();

   for ( i = faultElementMap->firstI (); i <= faultElementMap->lastI (); ++i ) {

      for ( j = faultElementMap->firstJ (); j <= faultElementMap->lastJ (); ++j ) {

         if ( FastcauldronSimulator::getInstance ().nodeIsDefined ( i, j )) {

            if ( m_formation->getLithology ( i, j )->isFault ()) {
               value = 1.0;
            } else {
               value = 0.0;
            }

            faultElementMap->setValue ( i, j, value );
         } else {
            faultElementMap->setValue ( i, j, undefinedValue );
         }

      }

   }

   faultElementMap->restoreData ();
   m_isCalculated = true;
   return true;
}

void FaultElementMapCalculator::allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties ) {

   properties.push_back ((PropertyValue*)(FastcauldronSimulator::getInstance ().createMapPropertyValue ( "FaultElements", 
                                                                                                         m_snapshot, 0, 
                                                                                                         m_formation,
                                                                                                         0 )));

}

bool FaultElementMapCalculator::initialise ( OutputPropertyMap::PropertyValueList& propertyValues ) {
   return true;
}
