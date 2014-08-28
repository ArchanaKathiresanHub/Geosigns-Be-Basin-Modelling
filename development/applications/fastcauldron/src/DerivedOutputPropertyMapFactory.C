#include "DerivedOutputPropertyMapFactory.h"

#include "DerivedOutputPropertyMap.h"
#include "PrimaryOutputPropertyMap.h"
#include "PrimaryOutputPropertyVolume.h"

#include "MaxVesCalculator.h"
#include "PorosityCalculator.h"
#include "PermeabilityCalculator.h"
#include "BulkDensityCalculator.h"
#include "ThicknessCalculator.h"
#include "VelocityCalculator.h"
#include "ReflectivityCalculator.h"
#include "SonicCalculator.h"
#include "ThermalConductivityCalculator.h"
#include "ThermalDiffusivityCalculator.h"
#include "AllochthonousLithologyMapCalculator.h"
#include "ErosionFactorCalculator.h"
#include "FaultElementMapCalculator.h"
#include "HeatFlowCalculator.h"
#include "FluidVelocityCalculator.h"
#include "VitriniteReflectanceCalculator.h"
#include "SmectiteIlliteAdapter.h"
#include "BiomarkersAdapter.h"
#include "LithologyIdCalculator.h"

#include "ComponentConcentrationCalculator.h"
#include "SaturationCalculator.h"


#include "RequiredGenex5PropertyCalculator.h"
#include "OptionalGenexPropertyCalculator.h"

#include "BasementPropertyCalculator.h"

#include "ThicknessErrorMapCalculator.h"
#include "FCTCorrectionCalculator.h"

#include "PvtHcProperties.h"
#include "HcFluidVelocityCalculator.h"
#include "VolumeCalculator.h"
#include "TransportedVolumeCalculator.h"
#include "RelativePermeabilityCalculator.h"

#include "CapillaryPressureCalculator.h"
#include "FluidPropertyCalculator.h"
#include "BrineProperties.h"
#include "TimeOfElementInvasionCalculator.h"
#include "FracturePressureVolumeCalculator.h"

DerivedOutputPropertyMapFactory::DerivedOutputPropertyMapFactory () {

   MapPropertyTraits    mapTraits;
   VolumePropertyTraits volumeTraits;

//     ISOSTATICDEFLECTION,
//     ISOSTATICWB,
//     MASSFLUX,
//     BIOMARKERS,
//     STERANEAROMATISATION,
//     STERANEISOMERISATION,
//     HOPANEISOMERISATION,
//     ILLITEFRACTION, 

   // Primary properties.
   mapTraits.m_propertyAllocator = allocatePrimaryPropertyCalculator;
   mapTraits.m_outputAssociation = SURFACE_ASSOCIATION;
   mapTraits.m_isPrimaryProperty = true;
   m_mapPropertyTraitsMap [ DEPTH ] = mapTraits;

   mapTraits.m_propertyAllocator = allocatePrimaryPropertyCalculator;
   mapTraits.m_outputAssociation = SURFACE_ASSOCIATION;
   mapTraits.m_isPrimaryProperty = true;
   m_mapPropertyTraitsMap [ HYDROSTATICPRESSURE ] = mapTraits;

   mapTraits.m_propertyAllocator = allocatePrimaryPropertyCalculator;
   mapTraits.m_outputAssociation = SURFACE_ASSOCIATION;
   mapTraits.m_isPrimaryProperty = true;
   m_mapPropertyTraitsMap [ LITHOSTATICPRESSURE ] = mapTraits;

   mapTraits.m_propertyAllocator = allocatePrimaryPropertyCalculator;
   mapTraits.m_outputAssociation = SURFACE_ASSOCIATION;
   mapTraits.m_isPrimaryProperty = true;
   m_mapPropertyTraitsMap [ OVERPRESSURE ] = mapTraits;

   mapTraits.m_propertyAllocator = allocatePrimaryPropertyCalculator;
   mapTraits.m_outputAssociation = SURFACE_ASSOCIATION;
   mapTraits.m_isPrimaryProperty = true;
   m_mapPropertyTraitsMap [ PRESSURE ] = mapTraits;

   mapTraits.m_propertyAllocator = allocatePrimaryPropertyCalculator;
   mapTraits.m_outputAssociation = SURFACE_FORMATION_ASSOCIATION;
   mapTraits.m_isPrimaryProperty = true;
   m_mapPropertyTraitsMap [ CHEMICAL_COMPACTION ] = mapTraits;

   mapTraits.m_propertyAllocator = allocatePrimaryPropertyCalculator;
   mapTraits.m_outputAssociation = SURFACE_ASSOCIATION;
   mapTraits.m_isPrimaryProperty = true;
   m_mapPropertyTraitsMap [ VES ] = mapTraits;

   mapTraits.m_propertyAllocator = allocatePrimaryPropertyCalculator;
   mapTraits.m_outputAssociation = SURFACE_ASSOCIATION;
   mapTraits.m_isPrimaryProperty = true;
   m_mapPropertyTraitsMap [ MAXVES ] = mapTraits;

#if 0
   mapTraits.m_propertyAllocator = allocateMaxVesCalculator;
   mapTraits.m_outputAssociation = SURFACE_FORMATION_ASSOCIATION;
   mapTraits.m_isPrimaryProperty = true;
   m_mapPropertyTraitsMap [ MAXVES ] = mapTraits;
#endif

   mapTraits.m_propertyAllocator = allocatePrimaryPropertyCalculator;
   mapTraits.m_outputAssociation = SURFACE_ASSOCIATION;
   mapTraits.m_isPrimaryProperty = true;
   m_mapPropertyTraitsMap [ TEMPERATURE ] = mapTraits;


   // Derived properties.

   // Surface properties.
   mapTraits.m_propertyAllocator = allocateHeatFlowCalculator;
   mapTraits.m_outputAssociation = SURFACE_ASSOCIATION;
   mapTraits.m_isPrimaryProperty = false;
   m_mapPropertyTraitsMap [ HEAT_FLOW ] = mapTraits;

   // mapTraits.m_propertyAllocator = allocateVitriniteReflectanceCalculator;
   // mapTraits.m_outputAssociation = SURFACE_ASSOCIATION;
   // mapTraits.m_isPrimaryProperty = false;
   // m_mapPropertyTraitsMap [ VR ] = mapTraits;

   mapTraits.m_propertyAllocator = allocateSmectiteIlliteAdapter;
   mapTraits.m_outputAssociation = SURFACE_ASSOCIATION;
   mapTraits.m_isPrimaryProperty = false;
   m_mapPropertyTraitsMap [ ILLITEFRACTION ] = mapTraits;

   mapTraits.m_propertyAllocator = allocateBiomarkersAdapter;
   mapTraits.m_outputAssociation = SURFACE_ASSOCIATION;
   mapTraits.m_isPrimaryProperty = false;
   m_mapPropertyTraitsMap [ BIOMARKERS ] = mapTraits;

   // Formation properties.
   mapTraits.m_propertyAllocator = allocateThicknessCalculator;
   mapTraits.m_outputAssociation = FORMATION_ASSOCIATION;
   mapTraits.m_isPrimaryProperty = false;
   m_mapPropertyTraitsMap [ THICKNESS ] = mapTraits;

   mapTraits.m_propertyAllocator = allocateThicknessErrorMapCalculator;
   mapTraits.m_outputAssociation = FORMATION_ASSOCIATION;
   mapTraits.m_isPrimaryProperty = false;
   m_mapPropertyTraitsMap [ THICKNESSERROR ] = mapTraits;

   mapTraits.m_propertyAllocator = allocateFCTCorrectionCalculator;
   mapTraits.m_outputAssociation = FORMATION_ASSOCIATION;
   mapTraits.m_isPrimaryProperty = false;
   m_mapPropertyTraitsMap [ FCTCORRECTION ] = mapTraits;

   mapTraits.m_propertyAllocator = allocateAllochthonousLithologyMapCalculator;
   mapTraits.m_outputAssociation = FORMATION_ASSOCIATION;
   mapTraits.m_isPrimaryProperty = false;
   m_mapPropertyTraitsMap [ ALLOCHTHONOUS_LITHOLOGY ] = mapTraits;

   mapTraits.m_propertyAllocator = allocateErosionFactorCalculator;
   mapTraits.m_outputAssociation = FORMATION_ASSOCIATION;
   mapTraits.m_isPrimaryProperty = false;
   m_mapPropertyTraitsMap [ EROSIONFACTOR ] = mapTraits;

   mapTraits.m_propertyAllocator = allocateFaultElementMapCalculator;
   mapTraits.m_outputAssociation = FORMATION_ASSOCIATION;
   mapTraits.m_isPrimaryProperty = false;
   m_mapPropertyTraitsMap [ FAULTELEMENTS ] = mapTraits;

   // Surface-Formation properties, i.e. vector properties.
   mapTraits.m_propertyAllocator = allocatePorosityCalculator;
   mapTraits.m_outputAssociation = SURFACE_FORMATION_ASSOCIATION;
   mapTraits.m_isPrimaryProperty = false;
   m_mapPropertyTraitsMap [ POROSITYVEC ] = mapTraits;

   mapTraits.m_propertyAllocator = allocatePermeabilityCalculator;
   mapTraits.m_outputAssociation = SURFACE_FORMATION_ASSOCIATION;
   mapTraits.m_isPrimaryProperty = false;
   m_mapPropertyTraitsMap [ PERMEABILITYVEC ] = mapTraits;

   mapTraits.m_propertyAllocator = allocateBulkDensityCalculator;
   mapTraits.m_outputAssociation = SURFACE_FORMATION_ASSOCIATION;
   mapTraits.m_isPrimaryProperty = false;
   m_mapPropertyTraitsMap [ BULKDENSITYVEC ] = mapTraits;

   mapTraits.m_propertyAllocator = allocateReflectivityCalculator;
   mapTraits.m_outputAssociation = SURFACE_FORMATION_ASSOCIATION;
   mapTraits.m_isPrimaryProperty = false;
   m_mapPropertyTraitsMap [ REFLECTIVITYVEC ] = mapTraits;

   mapTraits.m_propertyAllocator = allocateVelocityCalculator;
   mapTraits.m_outputAssociation = SURFACE_FORMATION_ASSOCIATION;
   mapTraits.m_isPrimaryProperty = false;
   m_mapPropertyTraitsMap [ VELOCITYVEC ] = mapTraits;

   mapTraits.m_propertyAllocator = allocateSonicCalculator;
   mapTraits.m_outputAssociation = SURFACE_FORMATION_ASSOCIATION;
   mapTraits.m_isPrimaryProperty = false;
   m_mapPropertyTraitsMap [ SONICVEC ] = mapTraits;

   mapTraits.m_propertyAllocator = allocateThermalConductivityCalculator;
   mapTraits.m_outputAssociation = SURFACE_FORMATION_ASSOCIATION;
   mapTraits.m_isPrimaryProperty = false;
   m_mapPropertyTraitsMap [ THCONDVEC ] = mapTraits;

   mapTraits.m_propertyAllocator = allocateThermalDiffusivityCalculator;
   mapTraits.m_outputAssociation = SURFACE_FORMATION_ASSOCIATION;
   mapTraits.m_isPrimaryProperty = false;
   m_mapPropertyTraitsMap [ DIFFUSIVITYVEC ] = mapTraits;

   mapTraits.m_propertyAllocator = allocateFluidVelocityCalculator;
   mapTraits.m_outputAssociation = SURFACE_FORMATION_ASSOCIATION;
   mapTraits.m_isPrimaryProperty = false;
   m_mapPropertyTraitsMap [ FLUID_VELOCITY ] = mapTraits;

   // Genex properties
   mapTraits.m_propertyAllocator = allocateRequiredGenex5PropertyCalculator;
   mapTraits.m_outputAssociation = FORMATION_ASSOCIATION;
   mapTraits.m_isPrimaryProperty = false;
   m_mapPropertyTraitsMap [ GENEX_PROPERTIES ] = mapTraits;
//    m_mapPropertyTraitsMap [ SPECIES_C1 ] = mapTraits;




   mapTraits.m_propertyAllocator = allocateInstantaneousExpulsionApiCalculator;
   mapTraits.m_outputAssociation = FORMATION_ASSOCIATION;
   mapTraits.m_isPrimaryProperty = false;
   m_mapPropertyTraitsMap [ EXPULSION_API_INST ] = mapTraits;


   mapTraits.m_propertyAllocator = allocateCumulativeExpulsionApiCalculator;
   mapTraits.m_outputAssociation = FORMATION_ASSOCIATION;
   mapTraits.m_isPrimaryProperty = false;
   m_mapPropertyTraitsMap [ EXPULSION_API_CUM ] = mapTraits;


   mapTraits.m_propertyAllocator = allocateInstantaneousExpulsionCondensateGasRatioCalculator;
   mapTraits.m_outputAssociation = FORMATION_ASSOCIATION;
   mapTraits.m_isPrimaryProperty = false;
   m_mapPropertyTraitsMap [ EXPULSION_CONDENSATE_GAS_RATIO_INST ] = mapTraits;


   mapTraits.m_propertyAllocator = allocateCumulativeExpulsionCondensateGasRatioCalculator;
   mapTraits.m_outputAssociation = FORMATION_ASSOCIATION;
   mapTraits.m_isPrimaryProperty = false;
   m_mapPropertyTraitsMap [ EXPULSION_CONDENSATE_GAS_RATIO_CUM ] = mapTraits;


   mapTraits.m_propertyAllocator = allocateInstantaneousExpulsionGasOilRatioCalculator;
   mapTraits.m_outputAssociation = FORMATION_ASSOCIATION;
   mapTraits.m_isPrimaryProperty = false;
   m_mapPropertyTraitsMap [ EXPULSION_GAS_OIL_RATIO_INST ] = mapTraits;


   mapTraits.m_propertyAllocator = allocateCumulativeExpulsionGasOilRatioCalculator;
   mapTraits.m_outputAssociation = FORMATION_ASSOCIATION;
   mapTraits.m_isPrimaryProperty = false;
   m_mapPropertyTraitsMap [ EXPULSION_GAS_OIL_RATIO_CUM ] = mapTraits;


   mapTraits.m_propertyAllocator = allocateInstantaneousExpulsionGasWetnessCalculator;
   mapTraits.m_outputAssociation = FORMATION_ASSOCIATION;
   mapTraits.m_isPrimaryProperty = false;
   m_mapPropertyTraitsMap [ EXPULSION_GAS_WETNESS_INST ] = mapTraits;


   mapTraits.m_propertyAllocator = allocateCumulativeExpulsionGasWetnessCalculator;
   mapTraits.m_outputAssociation = FORMATION_ASSOCIATION;
   mapTraits.m_isPrimaryProperty = false;
   m_mapPropertyTraitsMap [ EXPULSION_GAS_WETNESS_CUM ] = mapTraits;


   mapTraits.m_propertyAllocator = allocateInstantaneousExpulsionAromaticityCalculator;
   mapTraits.m_outputAssociation = FORMATION_ASSOCIATION;
   mapTraits.m_isPrimaryProperty = false;
   m_mapPropertyTraitsMap [ EXPULSION_AROMATICITY_INST ] = mapTraits;


   mapTraits.m_propertyAllocator = allocateCumulativeExpulsionAromaticityCalculator;
   mapTraits.m_outputAssociation = FORMATION_ASSOCIATION;
   mapTraits.m_isPrimaryProperty = false;
   m_mapPropertyTraitsMap [ EXPULSION_AROMATICITY_CUM ] = mapTraits;


   mapTraits.m_propertyAllocator = allocateKerogenConversionRatioCalculator;
   mapTraits.m_outputAssociation = FORMATION_ASSOCIATION;
   mapTraits.m_isPrimaryProperty = false;
   m_mapPropertyTraitsMap [ KEROGEN_CONVERSION_RATIO ] = mapTraits;


   mapTraits.m_propertyAllocator = allocateOilGeneratedCumulativeCalculator;
   mapTraits.m_outputAssociation = FORMATION_ASSOCIATION;
   mapTraits.m_isPrimaryProperty = false;
   m_mapPropertyTraitsMap [ OIL_GENERATED_CUM ] = mapTraits;


   mapTraits.m_propertyAllocator = allocateOilGeneratedRateCalculator;
   mapTraits.m_outputAssociation = FORMATION_ASSOCIATION;
   mapTraits.m_isPrimaryProperty = false;
   m_mapPropertyTraitsMap [ OIL_GENERATED_RATE ] = mapTraits;


   mapTraits.m_propertyAllocator = allocateOilExpelledCumulativeCalculator;
   mapTraits.m_outputAssociation = FORMATION_ASSOCIATION;
   mapTraits.m_isPrimaryProperty = false;
   m_mapPropertyTraitsMap [ OIL_EXPELLED_CUM ] = mapTraits;

   mapTraits.m_propertyAllocator = allocateVitriniteReflectanceCalculator;
   mapTraits.m_outputAssociation = SURFACE_FORMATION_ASSOCIATION;
   mapTraits.m_isPrimaryProperty = false;
   m_mapPropertyTraitsMap [ VR ] = mapTraits;


   mapTraits.m_propertyAllocator = allocateOilExpelledRateCalculator;
   mapTraits.m_outputAssociation = FORMATION_ASSOCIATION;
   mapTraits.m_isPrimaryProperty = false;
   m_mapPropertyTraitsMap [ OIL_EXPELLEDRATE ] = mapTraits;


   mapTraits.m_propertyAllocator = allocateHcGasGeneratedCumulativeCalculator;
   mapTraits.m_outputAssociation = FORMATION_ASSOCIATION;
   mapTraits.m_isPrimaryProperty = false;
   m_mapPropertyTraitsMap [ HC_GAS_GENERATED_CUM ] = mapTraits;


   mapTraits.m_propertyAllocator = allocateHcGasGeneratedRateCalculator;
   mapTraits.m_outputAssociation = FORMATION_ASSOCIATION;
   mapTraits.m_isPrimaryProperty = false;
   m_mapPropertyTraitsMap [ HC_GAS_GENERATED_RATE ] = mapTraits;


   mapTraits.m_propertyAllocator = allocateHcGasExpelledCumulativeCalculator;
   mapTraits.m_outputAssociation = FORMATION_ASSOCIATION;
   mapTraits.m_isPrimaryProperty = false;
   m_mapPropertyTraitsMap [ HC_GAS_EXPELLED_CUM ] = mapTraits;


   mapTraits.m_propertyAllocator = allocateHcGasExpelledRateCalculator;
   mapTraits.m_outputAssociation = FORMATION_ASSOCIATION;
   mapTraits.m_isPrimaryProperty = false;
   m_mapPropertyTraitsMap [ HC_GAS_EXPELLED_RATE ] = mapTraits;


   mapTraits.m_propertyAllocator = allocateDryGasGeneratedCumulativeCalculator;
   mapTraits.m_outputAssociation = FORMATION_ASSOCIATION;
   mapTraits.m_isPrimaryProperty = false;
   m_mapPropertyTraitsMap [ DRY_GAS_GENERATED_CUM ] = mapTraits;


   mapTraits.m_propertyAllocator = allocateDryGasGeneratedRateCalculator;
   mapTraits.m_outputAssociation = FORMATION_ASSOCIATION;
   mapTraits.m_isPrimaryProperty = false;
   m_mapPropertyTraitsMap [ DRY_GAS_GENERATED_RATE ] = mapTraits;


   mapTraits.m_propertyAllocator = allocateDryGasExpelledCumulativeCalculator;
   mapTraits.m_outputAssociation = FORMATION_ASSOCIATION;
   mapTraits.m_isPrimaryProperty = false;
   m_mapPropertyTraitsMap [ DRY_GAS_EXPELLED_CUM ] = mapTraits;


   mapTraits.m_propertyAllocator = allocateDryGasExpelledRateCalculator;
   mapTraits.m_outputAssociation = FORMATION_ASSOCIATION;
   mapTraits.m_isPrimaryProperty = false;
   m_mapPropertyTraitsMap [ DRY_GAS_EXPELLED_RATE ] = mapTraits;


   mapTraits.m_propertyAllocator = allocateWetGasGeneratedCumulativeCalculator;
   mapTraits.m_outputAssociation = FORMATION_ASSOCIATION;
   mapTraits.m_isPrimaryProperty = false;
   m_mapPropertyTraitsMap [ WET_GAS_GENERATED_CUM ] = mapTraits;


   mapTraits.m_propertyAllocator = allocateWetGasGeneratedRateCalculator;
   mapTraits.m_outputAssociation = FORMATION_ASSOCIATION;
   mapTraits.m_isPrimaryProperty = false;
   m_mapPropertyTraitsMap [ WET_GAS_GENERATED_RATE ] = mapTraits;


   mapTraits.m_propertyAllocator = allocateWetGasExpelledCumulativeCalculator;
   mapTraits.m_outputAssociation = FORMATION_ASSOCIATION;
   mapTraits.m_isPrimaryProperty = false;
   m_mapPropertyTraitsMap [ WET_GAS_EXPELLED_CUM ] = mapTraits;


   mapTraits.m_propertyAllocator = allocateWetGasExpelledRateCalculator;
   mapTraits.m_outputAssociation = FORMATION_ASSOCIATION;
   mapTraits.m_isPrimaryProperty = false;
   m_mapPropertyTraitsMap [ WET_GAS_EXPELLED_RATE ] = mapTraits;

   mapTraits.m_propertyAllocator = allocateBasementPropertyCalculator;
   mapTraits.m_outputAssociation = FORMATION_ASSOCIATION;
   mapTraits.m_isPrimaryProperty = false;
   m_mapPropertyTraitsMap [ BASALTTHICKNESS ] = mapTraits;

   mapTraits.m_propertyAllocator = allocateBasementPropertyCalculator;
   mapTraits.m_outputAssociation = FORMATION_ASSOCIATION;
   mapTraits.m_isPrimaryProperty = false;
   m_mapPropertyTraitsMap [ HLMOD ] = mapTraits;

   mapTraits.m_propertyAllocator = allocateBasementPropertyCalculator;
   mapTraits.m_outputAssociation = FORMATION_ASSOCIATION;
   mapTraits.m_isPrimaryProperty = false;
   m_mapPropertyTraitsMap [ TOPBASALTALC ] = mapTraits;

   mapTraits.m_propertyAllocator = allocateBasementPropertyCalculator;
   mapTraits.m_outputAssociation = FORMATION_ASSOCIATION;
   mapTraits.m_isPrimaryProperty = false;
   m_mapPropertyTraitsMap [ MOHOALC ] = mapTraits;

   mapTraits.m_propertyAllocator = allocateBasementPropertyCalculator;
   mapTraits.m_outputAssociation = FORMATION_ASSOCIATION;
   mapTraits.m_isPrimaryProperty = false;
   m_mapPropertyTraitsMap [ THICKNESSCCRUSTALC ] = mapTraits;

   mapTraits.m_propertyAllocator = allocateBasementPropertyCalculator;
   mapTraits.m_outputAssociation = FORMATION_ASSOCIATION;
   mapTraits.m_isPrimaryProperty = false;
   m_mapPropertyTraitsMap [ THICKNESSBASALTALC ] = mapTraits;

   mapTraits.m_propertyAllocator = allocateBasementPropertyCalculator;
   mapTraits.m_outputAssociation = FORMATION_ASSOCIATION;
   mapTraits.m_isPrimaryProperty = false;
   m_mapPropertyTraitsMap [ ALCSMCRUST ] = mapTraits;

   mapTraits.m_propertyAllocator = allocateBasementPropertyCalculator;
   mapTraits.m_outputAssociation = FORMATION_ASSOCIATION;
   mapTraits.m_isPrimaryProperty = false;
   m_mapPropertyTraitsMap [ ALCSMTOPBASALT ] = mapTraits;

   mapTraits.m_propertyAllocator = allocateBasementPropertyCalculator;
   mapTraits.m_outputAssociation = FORMATION_ASSOCIATION;
   mapTraits.m_isPrimaryProperty = false;
   m_mapPropertyTraitsMap [ ALCSMMOHO ] = mapTraits;

   mapTraits.m_propertyAllocator = allocateBasementPropertyCalculator;
   mapTraits.m_outputAssociation = FORMATION_ASSOCIATION;
   mapTraits.m_isPrimaryProperty = false;
   m_mapPropertyTraitsMap [ ALCORIGMANTLE ] = mapTraits;



   // Volume properties.

//       ISOSTATICMASS, 
//       ISOSTATICDEFLECTION,
//       ISOSTATICWB,
//       MASSFLUX,
//       BIOMARKERS,
//       STERANEAROMATISATION,
//       STERANEISOMERISATION,
//       HOPANEISOMERISATION,
//       ILLITEFRACTION, 

   volumeTraits.m_propertyAllocator = allocatePrimaryPropertyVolumeCalculator;
   volumeTraits.m_isPrimaryProperty = true;
   m_volumePropertyTraitsMap [ DEPTH ] = volumeTraits;

   volumeTraits.m_propertyAllocator = allocatePrimaryPropertyVolumeCalculator;
   volumeTraits.m_isPrimaryProperty = true;
   m_volumePropertyTraitsMap [ HYDROSTATICPRESSURE ] = volumeTraits;

   volumeTraits.m_propertyAllocator = allocatePrimaryPropertyVolumeCalculator;
   volumeTraits.m_isPrimaryProperty = true;
   m_volumePropertyTraitsMap [ LITHOSTATICPRESSURE ] = volumeTraits;

   volumeTraits.m_propertyAllocator = allocatePrimaryPropertyVolumeCalculator;
   volumeTraits.m_isPrimaryProperty = true;
   m_volumePropertyTraitsMap [ OVERPRESSURE ] = volumeTraits;

   volumeTraits.m_propertyAllocator = allocatePrimaryPropertyVolumeCalculator;
   volumeTraits.m_isPrimaryProperty = true;
   m_volumePropertyTraitsMap [ PRESSURE ] = volumeTraits;

   volumeTraits.m_propertyAllocator = allocateFracturePressureVolumeCalculator;
   volumeTraits.m_isPrimaryProperty = true;
   m_volumePropertyTraitsMap [ FRACTURE_PRESSURE ] = volumeTraits;

   volumeTraits.m_propertyAllocator = allocatePrimaryPropertyVolumeCalculator;
   volumeTraits.m_isPrimaryProperty = true;
   m_volumePropertyTraitsMap [ CHEMICAL_COMPACTION ] = volumeTraits;

   volumeTraits.m_propertyAllocator = allocatePrimaryPropertyVolumeCalculator;
   volumeTraits.m_isPrimaryProperty = true;
   m_volumePropertyTraitsMap [ VES ] = volumeTraits;

   volumeTraits.m_propertyAllocator = allocatePrimaryPropertyVolumeCalculator;
   volumeTraits.m_isPrimaryProperty = true;
   m_volumePropertyTraitsMap [ MAXVES ] = volumeTraits;

   volumeTraits.m_propertyAllocator = allocatePrimaryPropertyVolumeCalculator;
   volumeTraits.m_isPrimaryProperty = true;
   m_volumePropertyTraitsMap [ TEMPERATURE ] = volumeTraits;

   volumeTraits.m_propertyAllocator = allocatePorosityVolumeCalculator;
   volumeTraits.m_isPrimaryProperty = false;
   m_volumePropertyTraitsMap [ POROSITYVEC ] = volumeTraits;

   volumeTraits.m_propertyAllocator = allocatePermeabilityVolumeCalculator;
   volumeTraits.m_isPrimaryProperty = false;
   m_volumePropertyTraitsMap [ PERMEABILITYVEC ] = volumeTraits;

   volumeTraits.m_propertyAllocator = allocateHeatFlowVolumeCalculator;
   volumeTraits.m_isPrimaryProperty = false;
   m_volumePropertyTraitsMap [ HEAT_FLOW ] = volumeTraits;

   volumeTraits.m_propertyAllocator = allocateBulkDensityVolumeCalculator;
   volumeTraits.m_isPrimaryProperty = false;
   m_volumePropertyTraitsMap [ BULKDENSITYVEC ] = volumeTraits;

   volumeTraits.m_propertyAllocator = allocateReflectivityVolumeCalculator;
   volumeTraits.m_isPrimaryProperty = false;
   m_volumePropertyTraitsMap [ REFLECTIVITYVEC ] = volumeTraits;

   volumeTraits.m_propertyAllocator = allocateThermalDiffusivityVolumeCalculator;
   volumeTraits.m_isPrimaryProperty = false;
   m_volumePropertyTraitsMap [ DIFFUSIVITYVEC ] = volumeTraits;

   volumeTraits.m_propertyAllocator = allocateThermalConductivityVolumeCalculator;
   volumeTraits.m_isPrimaryProperty = false;
   m_volumePropertyTraitsMap [ THCONDVEC ] = volumeTraits;

   volumeTraits.m_propertyAllocator = allocateFluidVelocityVolumeCalculator;
   volumeTraits.m_isPrimaryProperty = false;
   m_volumePropertyTraitsMap [ FLUID_VELOCITY ] = volumeTraits;

   volumeTraits.m_propertyAllocator = allocateSonicVolumeCalculator;
   volumeTraits.m_isPrimaryProperty = false;
   m_volumePropertyTraitsMap [ SONICVEC ] = volumeTraits;

   volumeTraits.m_propertyAllocator = allocateVelocityVolumeCalculator;
   volumeTraits.m_isPrimaryProperty = false;
   m_volumePropertyTraitsMap [ VELOCITYVEC ] = volumeTraits;

   volumeTraits.m_propertyAllocator = allocateVitriniteReflectanceVolumeCalculator;
   volumeTraits.m_isPrimaryProperty = false;
   m_volumePropertyTraitsMap [ VR ] = volumeTraits;

   volumeTraits.m_propertyAllocator = allocateSmectiteIlliteVolumeAdapter;
   volumeTraits.m_isPrimaryProperty = false;
   m_volumePropertyTraitsMap [ ILLITEFRACTION ] = volumeTraits;

   volumeTraits.m_propertyAllocator = allocateBiomarkersVolumeAdapter;
   volumeTraits.m_isPrimaryProperty = false;
   m_volumePropertyTraitsMap [ BIOMARKERS ] = volumeTraits;

   volumeTraits.m_propertyAllocator = allocateLithologyIdVolumeCalculator;
   volumeTraits.m_isPrimaryProperty = false;
   m_volumePropertyTraitsMap [ LITHOLOGY ] = volumeTraits;

  //  volumeTraits.m_propertyAllocator = allocateCapillaryPressureGas100VolumeCalculator;
//    volumeTraits.m_isPrimaryProperty = false;
//    m_volumePropertyTraitsMap [ CAPILLARYPRESSUREGAS100 ] = volumeTraits;

//    volumeTraits.m_propertyAllocator = allocateCapillaryPressureGas0VolumeCalculator;
//    volumeTraits.m_isPrimaryProperty = false;
//    m_volumePropertyTraitsMap [ CAPILLARYPRESSUREGAS0 ] = volumeTraits;

//    volumeTraits.m_propertyAllocator = allocateCapillaryPressureOil100VolumeCalculator;
//    volumeTraits.m_isPrimaryProperty = false;
//    m_volumePropertyTraitsMap [ CAPILLARYPRESSUREOIL100 ] = volumeTraits;

//    volumeTraits.m_propertyAllocator = allocateCapillaryPressureOil0VolumeCalculator;
//    volumeTraits.m_isPrimaryProperty = false;
//    m_volumePropertyTraitsMap [ CAPILLARYPRESSUREOIL0 ] = volumeTraits;



   // This set of properties is made up of all the 13 hc species-concentrations.
   mapTraits.m_propertyAllocator = allocateComponentConcentrationCalculator;
   mapTraits.m_outputAssociation = FORMATION_ASSOCIATION;
   mapTraits.m_isPrimaryProperty = false;
   m_mapPropertyTraitsMap [ GENEX_PROPERTY_CONCENTRATIONS ] = mapTraits;

   volumeTraits.m_propertyAllocator = allocateComponentConcentrationVolumeCalculator;
   // This set of properties is made up of all the hc species-concentrations.
   volumeTraits.m_isPrimaryProperty = false;
   m_volumePropertyTraitsMap [ GENEX_PROPERTY_CONCENTRATIONS ] = volumeTraits;



   // mapTraits.m_propertyAllocator = allocatePvtPropertiesCalculator;
   // mapTraits.m_outputAssociation = FORMATION_ASSOCIATION;
   // mapTraits.m_isPrimaryProperty = false;
   // m_mapPropertyTraitsMap [ PVT_PROPERTIES ] = mapTraits;

   // This set of properties is comprised of: liquid- and vapour-density and -viscosity.
   // Should we also include the brine-density and -viscosity?
   volumeTraits.m_propertyAllocator = allocatePvtHcPropertiesVolumeCalculator;
   volumeTraits.m_isPrimaryProperty = false;
   m_volumePropertyTraitsMap [ PVT_PROPERTIES ] = volumeTraits;

   // Calculates the volumes of hc liquid and vapour (m^3/m^3), element volume and element pore volume.
   volumeTraits.m_propertyAllocator = allocateVolumeCalculator;
   volumeTraits.m_isPrimaryProperty = false;
   m_volumePropertyTraitsMap [ VOLUME_CALCULATIONS ] = volumeTraits;

   // Calculates the mass of hc that was transported around the system over the snapshot-time.
   volumeTraits.m_propertyAllocator = allocateTransportedVolumeCalculator;
   volumeTraits.m_isPrimaryProperty = false;
   m_volumePropertyTraitsMap [ TRANSPORTED_VOLUME_CALCULATIONS ] = volumeTraits;


   // The set of liquid- vapour- and brine-saturations.
   mapTraits.m_propertyAllocator = allocateSaturationCalculator;
   mapTraits.m_outputAssociation = FORMATION_ASSOCIATION;
   mapTraits.m_isPrimaryProperty = false;
   m_mapPropertyTraitsMap [ SATURATION ] = mapTraits;

   volumeTraits.m_propertyAllocator = allocateSaturationVolumeCalculator;
   volumeTraits.m_isPrimaryProperty = false;
   m_volumePropertyTraitsMap [ SATURATION ] = volumeTraits;

    //average saturations
   mapTraits.m_propertyAllocator = allocateAverageSaturationCalculator;
   mapTraits.m_outputAssociation = FORMATION_ASSOCIATION;
   mapTraits.m_isPrimaryProperty = false;
   m_mapPropertyTraitsMap [ AVERAGE_SATURATION ] = mapTraits;

   volumeTraits.m_propertyAllocator = allocateAverageSaturationVolumeCalculator;
   volumeTraits.m_isPrimaryProperty = false;
   m_volumePropertyTraitsMap [ AVERAGE_SATURATION ] = volumeTraits;


    //Capillary Pressure
  
   volumeTraits.m_propertyAllocator = allocateCapillaryPressureVolumeCalculator;
   volumeTraits.m_isPrimaryProperty = false;
   m_volumePropertyTraitsMap [ CAPILLARY_PRESSURE  ] = volumeTraits;

 //Fluid properties such as GOR, CGR, API
   // mapTraits.m_propertyAllocator = allocateFluidPropertyCalculator;
//    mapTraits.m_outputAssociation = FORMATION_ASSOCIATION;
//    mapTraits.m_isPrimaryProperty = false;
//    m_mapPropertyTraitsMap [ FLUID_PROPERTIES ] = mapTraits;

   volumeTraits.m_propertyAllocator = allocateFluidPropertyVolumeCalculator;
   volumeTraits.m_isPrimaryProperty = false;
   m_volumePropertyTraitsMap [ FLUID_PROPERTIES ] = volumeTraits;

   //Brine properties viscosity and density
   
   volumeTraits.m_propertyAllocator = allocateBrinePropertiesVolumeCalculator;
   volumeTraits.m_isPrimaryProperty = false;
   m_volumePropertyTraitsMap [ BRINE_PROPERTIES ] = volumeTraits;

   //Time of Element Invasion
   volumeTraits.m_propertyAllocator = allocateTimeOfElementInvasionVolumeCalculator;
   volumeTraits.m_isPrimaryProperty = false;
   m_volumePropertyTraitsMap [ TIME_OF_ELEMENT_INVASION ] = volumeTraits;
   
   volumeTraits.m_propertyAllocator = allocateRelativePermeabilityCalculator;
   volumeTraits.m_isPrimaryProperty = false;
   m_volumePropertyTraitsMap [ RELATIVE_PERMEABILITY ] = volumeTraits;


   volumeTraits.m_propertyAllocator = allocateHcFluidVelocityVolumeCalculator;
   volumeTraits.m_isPrimaryProperty = false;
   m_volumePropertyTraitsMap [ HC_FLUID_VELOCITY ] = volumeTraits;

}

OutputPropertyMap* DerivedOutputPropertyMapFactory::allocateMap ( const PropertyList         derivedProperty,
                                                                        LayerProps*          formation,
                                                                  const Interface::Surface*  surface,
                                                                  const Interface::Snapshot* snapshot ) const {

   MapPropertyTraitsMap::const_iterator traitsIter = m_mapPropertyTraitsMap.find ( derivedProperty );

   if ( traitsIter != m_mapPropertyTraitsMap.end ()) {
      return traitsIter->second.m_propertyAllocator ( derivedProperty, formation, surface, snapshot );
   } else {
      return 0;
   }

}

OutputPropertyMap* DerivedOutputPropertyMapFactory::allocateVolume ( const PropertyList         derivedProperty,
                                                                           LayerProps*          formation,
                                                                     const Interface::Snapshot* snapshot ) const {

   VolumePropertyTraitsMap::const_iterator traitsIter = m_volumePropertyTraitsMap.find ( derivedProperty );

   if ( traitsIter != m_volumePropertyTraitsMap.end ()) {
      return traitsIter->second.m_propertyAllocator ( derivedProperty, formation, snapshot );
   } else {
      return 0;
   }

}


bool DerivedOutputPropertyMapFactory::isVectorQuantity ( const PropertyList derivedProperty ) const {

   MapPropertyTraitsMap::const_iterator traitsIter = m_mapPropertyTraitsMap.find ( derivedProperty );

   if ( traitsIter != m_mapPropertyTraitsMap.end ()) {
      return traitsIter->second.m_outputAssociation == SURFACE_FORMATION_ASSOCIATION;
   } else {
      return false;
   }

}

OutputPropertyMapAssociation DerivedOutputPropertyMapFactory::getMapAssociation ( const PropertyList derivedProperty ) const {

   MapPropertyTraitsMap::const_iterator traitsIter = m_mapPropertyTraitsMap.find ( derivedProperty );

   if ( traitsIter != m_mapPropertyTraitsMap.end ()) {
      return traitsIter->second.m_outputAssociation;
   } else {
      return UNKNOWN_ASSOCIATION;
   }

}

bool DerivedOutputPropertyMapFactory::isPrimary ( const PropertyList derivedProperty ) const {

   MapPropertyTraitsMap::const_iterator traitsIter = m_mapPropertyTraitsMap.find ( derivedProperty );

   if ( traitsIter != m_mapPropertyTraitsMap.end ()) {
      return traitsIter->second.m_isPrimaryProperty;
   } else {
      return false;
   }

}

bool DerivedOutputPropertyMapFactory::isMapDefined ( const PropertyList derivedProperty ) const {
   return m_mapPropertyTraitsMap.find ( derivedProperty ) != m_mapPropertyTraitsMap.end ();
}

bool DerivedOutputPropertyMapFactory::isVolumeDefined ( const PropertyList derivedProperty ) const {
   return m_volumePropertyTraitsMap.find ( derivedProperty ) != m_volumePropertyTraitsMap.end ();
}
