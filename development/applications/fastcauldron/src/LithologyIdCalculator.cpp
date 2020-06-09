#include "LithologyIdCalculator.h"
#include "DerivedOutputPropertyMap.h"
#include "PropertyManager.h"
#include "FastcauldronSimulator.h"
#include "Lithology.h"

OutputPropertyMap* allocateLithologyIdVolumeCalculator ( const PropertyIdentifier& property, LayerProps* formation, const Interface::Snapshot* snapshot ) {
   return new DerivedOutputPropertyMap<LithologyIdVolumeCalculator>( property, formation, snapshot );
}

LithologyIdVolumeCalculator::LithologyIdVolumeCalculator ( LayerProps* formation, const Interface::Snapshot* snapshot ) :
   m_formation ( formation ), m_snapshot ( snapshot ) {

   m_isCalculated = false;
   m_lithologies = 0;

}

bool LithologyIdVolumeCalculator::operator ()( const OutputPropertyMap::OutputPropertyList& properties, 
                                                     OutputPropertyMap::PropertyValueList&  propertyValues ) {

   if ( m_isCalculated ) {
      return true;
   }

   unsigned int i;
   unsigned int j;
   unsigned int k;
   double value;
   double undefinedValue;
   Interface::GridMap* lithologyIdMap;

   lithologyIdMap = propertyValues [ 0 ]->getGridMap ();
   lithologyIdMap->retrieveData ();
   undefinedValue = lithologyIdMap->getUndefinedValue ();

   for ( i = lithologyIdMap->firstI (); i <= lithologyIdMap->lastI (); ++i ) {

      for ( j = lithologyIdMap->firstJ (); j <= lithologyIdMap->lastJ (); ++j ) {

         if ( FastcauldronSimulator::getInstance ().nodeIsDefined ( i, j )) {
            value = double ( ((Lithology*)((*m_lithologies)( i, j )))->getLithologyID ());
         } else {
            value = undefinedValue;
         }

         for ( k = lithologyIdMap->firstK (); k <= lithologyIdMap->lastK (); ++k ) {
            lithologyIdMap->setValue ( i, j, k, value );
         }

      }

   }

   lithologyIdMap->restoreData ();
   m_isCalculated = true;
   return true;
}

void LithologyIdVolumeCalculator::allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties ) {

   properties.push_back ((CauldronPropertyValue*)(FastcauldronSimulator::getInstance ().createVolumePropertyValue ( "Lithology", 
                                                                                                            m_snapshot, 0, 
                                                                                                            m_formation,
                                                                                                            m_formation->getMaximumNumberOfElements () + 1 )));

}

bool LithologyIdVolumeCalculator::initialise ( OutputPropertyMap::PropertyValueList& propertyValues ) {

   m_lithologies = &m_formation->getCompoundLithologyArray ();

   return m_lithologies != 0;
}
