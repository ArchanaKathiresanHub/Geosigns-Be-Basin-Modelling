#include "OptionalGenexPropertyCalculator.h"


#include "DerivedOutputPropertyMap.h"



OutputPropertyMap* allocateInstantaneousExpulsionApiCalculator ( const PropertyList property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) {
   return new DerivedOutputPropertyMap<OptionalGenexPropertyCalculator<CBMGenerics::GenexResultManager::ExpulsionApiInst> >( property, formation, surface, snapshot );
}


OutputPropertyMap* allocateCumulativeExpulsionApiCalculator ( const PropertyList property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) {
   return new DerivedOutputPropertyMap<OptionalGenexPropertyCalculator<CBMGenerics::GenexResultManager::ExpulsionApiCum> >( property, formation, surface, snapshot );
}


OutputPropertyMap* allocateInstantaneousExpulsionCondensateGasRatioCalculator ( const PropertyList property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) {
   return new DerivedOutputPropertyMap<OptionalGenexPropertyCalculator<CBMGenerics::GenexResultManager::ExpulsionCondensateGasRatioInst> >( property, formation, surface, snapshot );
}


OutputPropertyMap* allocateCumulativeExpulsionCondensateGasRatioCalculator ( const PropertyList property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) {
   return new DerivedOutputPropertyMap<OptionalGenexPropertyCalculator<CBMGenerics::GenexResultManager::ExpulsionCondensateGasRatioCum> >( property, formation, surface, snapshot );
}


OutputPropertyMap* allocateInstantaneousExpulsionGasOilRatioCalculator ( const PropertyList property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) {
   return new DerivedOutputPropertyMap<OptionalGenexPropertyCalculator<CBMGenerics::GenexResultManager::ExpulsionGasOilRatioInst> >( property, formation, surface, snapshot );
}


OutputPropertyMap* allocateCumulativeExpulsionGasOilRatioCalculator ( const PropertyList property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) {
   return new DerivedOutputPropertyMap<OptionalGenexPropertyCalculator<CBMGenerics::GenexResultManager::ExpulsionGasOilRatioCum> >( property, formation, surface, snapshot );
}


OutputPropertyMap* allocateInstantaneousExpulsionGasWetnessCalculator ( const PropertyList property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) {
   return new DerivedOutputPropertyMap<OptionalGenexPropertyCalculator<CBMGenerics::GenexResultManager::ExpulsionGasWetnessInst> >( property, formation, surface, snapshot );
}


OutputPropertyMap* allocateCumulativeExpulsionGasWetnessCalculator ( const PropertyList property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) {
   return new DerivedOutputPropertyMap<OptionalGenexPropertyCalculator<CBMGenerics::GenexResultManager::ExpulsionGasWetnessCum> >( property, formation, surface, snapshot );
}


OutputPropertyMap* allocateInstantaneousExpulsionAromaticityCalculator ( const PropertyList property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) {
   return new DerivedOutputPropertyMap<OptionalGenexPropertyCalculator<CBMGenerics::GenexResultManager::ExpulsionAromaticityInst> >( property, formation, surface, snapshot );
}


OutputPropertyMap* allocateCumulativeExpulsionAromaticityCalculator ( const PropertyList property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) {
   return new DerivedOutputPropertyMap<OptionalGenexPropertyCalculator<CBMGenerics::GenexResultManager::ExpulsionAromaticityCum> >( property, formation, surface, snapshot );
}


OutputPropertyMap* allocateKerogenConversionRatioCalculator ( const PropertyList property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) {
   return new DerivedOutputPropertyMap<OptionalGenexPropertyCalculator<CBMGenerics::GenexResultManager::KerogenConversionRatio> >( property, formation, surface, snapshot );
}


OutputPropertyMap* allocateOilGeneratedCumulativeCalculator ( const PropertyList property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) {
   return new DerivedOutputPropertyMap<OptionalGenexPropertyCalculator<CBMGenerics::GenexResultManager::OilGeneratedCum> >( property, formation, surface, snapshot );
}


OutputPropertyMap* allocateOilGeneratedRateCalculator ( const PropertyList property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) {
   return new DerivedOutputPropertyMap<OptionalGenexPropertyCalculator<CBMGenerics::GenexResultManager::OilGeneratedRate> >( property, formation, surface, snapshot );
}


OutputPropertyMap* allocateOilExpelledCumulativeCalculator ( const PropertyList property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) {
   return new DerivedOutputPropertyMap<OptionalGenexPropertyCalculator<CBMGenerics::GenexResultManager::OilExpelledCum> >( property, formation, surface, snapshot );
}


OutputPropertyMap* allocateOilExpelledRateCalculator ( const PropertyList property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) {
   return new DerivedOutputPropertyMap<OptionalGenexPropertyCalculator<CBMGenerics::GenexResultManager::OilExpelledRate> >( property, formation, surface, snapshot );
}


OutputPropertyMap* allocateHcGasGeneratedCumulativeCalculator ( const PropertyList property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) {
   return new DerivedOutputPropertyMap<OptionalGenexPropertyCalculator<CBMGenerics::GenexResultManager::HcGasGeneratedCum> >( property, formation, surface, snapshot );
}


OutputPropertyMap* allocateHcGasGeneratedRateCalculator ( const PropertyList property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) {
   return new DerivedOutputPropertyMap<OptionalGenexPropertyCalculator<CBMGenerics::GenexResultManager::HcGasGeneratedRate> >( property, formation, surface, snapshot );
}


OutputPropertyMap* allocateHcGasExpelledCumulativeCalculator ( const PropertyList property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) {
   return new DerivedOutputPropertyMap<OptionalGenexPropertyCalculator<CBMGenerics::GenexResultManager::HcGasExpelledCum> >( property, formation, surface, snapshot );
}


OutputPropertyMap* allocateHcGasExpelledRateCalculator ( const PropertyList property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) {
   return new DerivedOutputPropertyMap<OptionalGenexPropertyCalculator<CBMGenerics::GenexResultManager::HcGasExpelledRate> >( property, formation, surface, snapshot );
}


OutputPropertyMap* allocateDryGasGeneratedCumulativeCalculator ( const PropertyList property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) {
   return new DerivedOutputPropertyMap<OptionalGenexPropertyCalculator<CBMGenerics::GenexResultManager::DryGasGeneratedCum> >( property, formation, surface, snapshot );
}


OutputPropertyMap* allocateDryGasGeneratedRateCalculator ( const PropertyList property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) {
   return new DerivedOutputPropertyMap<OptionalGenexPropertyCalculator<CBMGenerics::GenexResultManager::DryGasGeneratedRate> >( property, formation, surface, snapshot );
}


OutputPropertyMap* allocateDryGasExpelledCumulativeCalculator ( const PropertyList property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) {
   return new DerivedOutputPropertyMap<OptionalGenexPropertyCalculator<CBMGenerics::GenexResultManager::DryGasExpelledCum> >( property, formation, surface, snapshot );
}


OutputPropertyMap* allocateDryGasExpelledRateCalculator ( const PropertyList property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) {
   return new DerivedOutputPropertyMap<OptionalGenexPropertyCalculator<CBMGenerics::GenexResultManager::DryGasExpelledRate> >( property, formation, surface, snapshot );
}


OutputPropertyMap* allocateWetGasGeneratedCumulativeCalculator ( const PropertyList property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) {
   return new DerivedOutputPropertyMap<OptionalGenexPropertyCalculator<CBMGenerics::GenexResultManager::WetGasGeneratedCum> >( property, formation, surface, snapshot );
}


OutputPropertyMap* allocateWetGasGeneratedRateCalculator ( const PropertyList property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) {
   return new DerivedOutputPropertyMap<OptionalGenexPropertyCalculator<CBMGenerics::GenexResultManager::WetGasGeneratedRate> >( property, formation, surface, snapshot );
}


OutputPropertyMap* allocateWetGasExpelledCumulativeCalculator ( const PropertyList property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) {
   return new DerivedOutputPropertyMap<OptionalGenexPropertyCalculator<CBMGenerics::GenexResultManager::WetGasExpelledCum> >( property, formation, surface, snapshot );
}


OutputPropertyMap* allocateWetGasExpelledRateCalculator ( const PropertyList property, LayerProps* formation, const Interface::Surface* surface, const Interface::Snapshot* snapshot ) {
   return new DerivedOutputPropertyMap<OptionalGenexPropertyCalculator<CBMGenerics::GenexResultManager::WetGasExpelledRate> >( property, formation, surface, snapshot );
}

