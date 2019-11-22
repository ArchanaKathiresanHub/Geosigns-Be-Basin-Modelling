#include "AllochthonousLithologyMapCalculator.h"
#include "DerivedOutputPropertyMap.h"
#include "PropertyManager.h"
#include "FastcauldronSimulator.h"

OutputPropertyMap* allocateAllochthonousLithologyMapCalculator ( const PropertyIdentifier& property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) {
   return new DerivedOutputPropertyMap<AllochthonousLithologyMapCalculator>( property, formation, surface, snapshot );
}

AllochthonousLithologyMapCalculator::AllochthonousLithologyMapCalculator ( LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) :
   m_formation ( formation ), m_surface ( surface ), m_snapshot ( snapshot ) {

   m_isCalculated = false;

}

bool AllochthonousLithologyMapCalculator::operator ()( const OutputPropertyMap::OutputPropertyList& ,
                                                             OutputPropertyMap::PropertyValueList&  propertyValues ) {

   if ( m_isCalculated ) {
      return true;
   }

   unsigned int i;
   unsigned int j;
   double value;
   double undefinedValue;
   Interface::GridMap* allochthonousLithologyMap;

   if ( not m_formation->hasAllochthonousLithology () ) {
      propertyValues [ 0 ]->allowOutput ( false );
      m_isCalculated = true;
      return true;
   }

   allochthonousLithologyMap = const_cast<Interface::GridMap*>(propertyValues [ 0 ]->getGridMap ());
   allochthonousLithologyMap->retrieveData ();
   undefinedValue = allochthonousLithologyMap->getUndefinedValue ();

   for ( i = allochthonousLithologyMap->firstI (); i <= allochthonousLithologyMap->lastI (); ++i ) {

      for ( j = allochthonousLithologyMap->firstJ (); j <= allochthonousLithologyMap->lastJ (); ++j ) {

         if ( FastcauldronSimulator::getInstance ().nodeIsDefined ( i, j )) {

            if ( m_lithologies->isAllochthonousLithology ( i, j )) {
               value = 1.0;
            } else {
               value = 0.0;
            }

            allochthonousLithologyMap->setValue ( i, j, value );
         } else {
            allochthonousLithologyMap->setValue ( i, j, undefinedValue );
         }

      }

   }

   allochthonousLithologyMap->restoreData ();
   m_isCalculated = true;
   return true;
}

void AllochthonousLithologyMapCalculator::allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties ) {

   properties.push_back ((CauldronPropertyValue*)(FastcauldronSimulator::getInstance ().createMapPropertyValue ( "AllochthonousLithology",
                                                                                                         m_snapshot, 0,
                                                                                                         m_formation,
                                                                                                         0 )));

}

bool AllochthonousLithologyMapCalculator::initialise ( OutputPropertyMap::PropertyValueList&  ) {

   m_lithologies = &m_formation->getCompoundLithologyArray ();

   return m_lithologies != 0;
}
