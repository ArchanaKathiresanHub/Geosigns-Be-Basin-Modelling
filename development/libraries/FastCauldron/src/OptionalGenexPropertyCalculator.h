#ifndef _FASTCAULDRON__OPTIONAL_GENEX5_PROPERTY_CALCULATOR_H_
#define _FASTCAULDRON__OPTIONAL_GENEX5_PROPERTY_CALCULATOR_H_


#include "DerivedOutputPropertyMap.h"

#include "GenexResultManager.h"

#include "GeoPhysicsFormation.h"
#include "GeoPhysicsSourceRock.h"


template<const CBMGenerics::GenexResultManager::ResultId OptionalResult>
class OptionalGenexPropertyCalculator {

public :

   OptionalGenexPropertyCalculator ( LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot );

   void allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties );

   bool operator ()( const OutputPropertyMap::OutputPropertyList& properties, 
                           OutputPropertyMap::PropertyValueList&  propertyValues );

   bool initialise ( OutputPropertyMap::PropertyValueList& propertyValues );

private :

   LayerProps* m_formation;
   const Interface::Surface* m_surface;
   const Interface::Snapshot* m_snapshot;

   const GeoPhysics::GeoPhysicsSourceRock* m_sourceRock;

   bool m_isCalculated;

};



template<const CBMGenerics::GenexResultManager::ResultId OptionalResult>
OptionalGenexPropertyCalculator<OptionalResult>::OptionalGenexPropertyCalculator ( LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) :
   m_formation ( formation ), m_surface ( surface ), m_snapshot ( snapshot ) {

   m_isCalculated = false;
   m_sourceRock = 0;
}


template<const CBMGenerics::GenexResultManager::ResultId OptionalResult>
bool OptionalGenexPropertyCalculator<OptionalResult>::operator ()( const OutputPropertyMap::OutputPropertyList& properties, 
                                                                         OutputPropertyMap::PropertyValueList&  propertyValues ) {

   if ( m_isCalculated ) {
      return true;
   }

   double undefinedValue;

   Interface::GridMap* derivedMap;


   derivedMap = propertyValues [ 0 ]->getGridMap ();
   derivedMap->retrieveData ();

   // The assumption here is that all the maps will have the same undefined-value.
   undefinedValue = derivedMap->getUndefinedValue ();

   // Should initialise the methane-map with null values first.

   m_sourceRock->fillOptionalResultMap ( OptionalResult, derivedMap );

   m_isCalculated = true;
   return true;
}


template<const CBMGenerics::GenexResultManager::ResultId OptionalResult>
void OptionalGenexPropertyCalculator<OptionalResult>::allocatePropertyValues ( OutputPropertyMap::PropertyValueList& properties ) {

   CauldronPropertyValue* derivedProp;


   derivedProp = (CauldronPropertyValue*)(FastcauldronSimulator::getInstance ().createMapPropertyValue ( CBMGenerics::GenexResultManager::getInstance ().GetResultName ( OptionalResult ),
                                                                                                 m_snapshot, 0, 
                                                                                                 m_formation,
                                                                                                 0 ));

   properties.push_back ( derivedProp );
}

template<const CBMGenerics::GenexResultManager::ResultId OptionalResult>
bool OptionalGenexPropertyCalculator<OptionalResult>::initialise ( OutputPropertyMap::PropertyValueList& propertyValues ) {

   bool isActive = false;

   if ( m_formation->isSourceRock ()) {

      isActive = m_formation->getTopSurface ()->getSnapshot ()->getTime () >= m_snapshot->getTime ();

      if ( isActive ) {
         m_sourceRock = (GeoPhysics::GeoPhysicsSourceRock*) (m_formation->getSourceRock1 ());
      } else {
         m_sourceRock = 0;
      }

   } else {
      m_sourceRock = 0;
   }

   if ( m_sourceRock == 0 ) {

      OutputPropertyMap::PropertyValueList::iterator iter;

      for ( iter = propertyValues.begin (); iter != propertyValues.end (); ++iter ) {
         (*iter)->allowOutput ( false );
      }

   }

   return ( m_formation->isSourceRock () and ( m_sourceRock != 0 or not isActive )) or not m_formation->isSourceRock ();
}



OutputPropertyMap* allocateInstantaneousExpulsionApiCalculator ( const PropertyIdentifier& property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot );

OutputPropertyMap* allocateCumulativeExpulsionApiCalculator ( const PropertyIdentifier& property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot );

OutputPropertyMap* allocateInstantaneousExpulsionCondensateGasRatioCalculator ( const PropertyIdentifier& property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot );

OutputPropertyMap* allocateCumulativeExpulsionCondensateGasRatioCalculator ( const PropertyIdentifier& property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot );

OutputPropertyMap* allocateInstantaneousExpulsionGasOilRatioCalculator ( const PropertyIdentifier& property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot );

OutputPropertyMap* allocateCumulativeExpulsionGasOilRatioCalculator ( const PropertyIdentifier& property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot );

OutputPropertyMap* allocateInstantaneousExpulsionGasWetnessCalculator ( const PropertyIdentifier& property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot );

OutputPropertyMap* allocateCumulativeExpulsionGasWetnessCalculator ( const PropertyIdentifier& property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot );

OutputPropertyMap* allocateInstantaneousExpulsionAromaticityCalculator ( const PropertyIdentifier& property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot );

OutputPropertyMap* allocateCumulativeExpulsionAromaticityCalculator ( const PropertyIdentifier& property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot );

OutputPropertyMap* allocateKerogenConversionRatioCalculator ( const PropertyIdentifier& property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot );

OutputPropertyMap* allocateOilGeneratedCumulativeCalculator ( const PropertyIdentifier& property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot );

OutputPropertyMap* allocateOilGeneratedRateCalculator ( const PropertyIdentifier& property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot );

OutputPropertyMap* allocateOilExpelledCumulativeCalculator ( const PropertyIdentifier& property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot );

OutputPropertyMap* allocateOilExpelledRateCalculator ( const PropertyIdentifier& property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot );

OutputPropertyMap* allocateHcGasGeneratedCumulativeCalculator ( const PropertyIdentifier& property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot );

OutputPropertyMap* allocateHcGasGeneratedRateCalculator ( const PropertyIdentifier& property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot );

OutputPropertyMap* allocateHcGasExpelledCumulativeCalculator ( const PropertyIdentifier& property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot );

OutputPropertyMap* allocateHcGasExpelledRateCalculator ( const PropertyIdentifier& property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot );

OutputPropertyMap* allocateDryGasGeneratedCumulativeCalculator ( const PropertyIdentifier& property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot );

OutputPropertyMap* allocateDryGasGeneratedRateCalculator ( const PropertyIdentifier& property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot );

OutputPropertyMap* allocateDryGasExpelledCumulativeCalculator ( const PropertyIdentifier& property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot );

OutputPropertyMap* allocateDryGasExpelledRateCalculator ( const PropertyIdentifier& property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot );

OutputPropertyMap* allocateWetGasGeneratedCumulativeCalculator ( const PropertyIdentifier& property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot );

OutputPropertyMap* allocateWetGasGeneratedRateCalculator ( const PropertyIdentifier& property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot );

OutputPropertyMap* allocateWetGasExpelledCumulativeCalculator ( const PropertyIdentifier& property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot );

OutputPropertyMap* allocateWetGasExpelledRateCalculator ( const PropertyIdentifier& property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot );



#endif // _FASTCAULDRON__OPTIONAL_GENEX5_PROPERTY_CALCULATOR_H_
