#ifndef _TIMEFILTER_H_
#define _TIMEFILTER_H_

#include <map>
#include <string>
#include <vector>
using namespace std;

#include "Interface/Interface.h"

// Change to Interface::PropertyOutputOption
enum OutputOption {NOOUTPUT,SOURCEROCKONLY,SEDIMENTSONLY,SEDIMENTSPLUSBASEMENT};

const string OutputOptionName[] = {
  "None",
  "SourceRockOnly",
  "SedimentsOnly",
  "SedimentsPlusBasement"
};
 
enum PropertyList {
   //Please use alphabetic order inside subcategories, and update PropertyOutputConstraints.C according to the order

   //Vector Properties
   BULKDENSITYVEC, DIFFUSIVITYVEC, PERMEABILITYHVEC, PERMEABILITYVEC, POROSITYVEC,
   REFLECTIVITYVEC, SONICVEC, THCONDVEC, VELOCITYVEC, VR, MAXVES,

   //Scalar Properties
   DEPTH, HEAT_FLOW, HEAT_FLOWY, HEAT_FLOWZ, HEAT_FLOW_,
   FLUID_VELOCITY, FLUID_VELOCITY_Y, FLUID_VELOCITY_Z, ISOSTATICMASS, 
   ISOSTATICDEFLECTION, ISOSTATICWB, MASSFLUX, MASSFLUX_, TEMPERATURE, 
   PRESSURE, HYDROSTATICPRESSURE, LITHOSTATICPRESSURE, OVERPRESSURE, FRACTURE_PRESSURE, VES, 
   BIOMARKERS, STERANEAROMATISATION, STERANEISOMERISATION,HOPANEISOMERISATION,
   ILLITEFRACTION, ALLOCHTHONOUS_LITHOLOGY, EROSIONFACTOR, 
   FAULTELEMENTS, FCTCORRECTION, THICKNESS, THICKNESSERROR, CHEMICAL_COMPACTION,
   LITHOLOGY, TWOWAYTIME, TWOWAYTIME_RESIDUAL,

   //
   CAPILLARYPRESSUREGAS100, CAPILLARYPRESSUREGAS0, CAPILLARYPRESSUREOIL100, CAPILLARYPRESSUREOIL0,

   // Genex properties.
   //
   // Species related properties
   GENEX_PROPERTIES,
   //
   SPECIES_ASPHALTENE, SPECIES_RESIN, SPECIES_C15_PLUS_ARO, SPECIES_C15_PLUS_SAT,
   // Oil components.
   SPECIES_C6_MINUS_14_ARO, SPECIES_C6_MINUS_14_SAT,
   // Gas components
   SPECIES_C5, SPECIES_C4, SPECIES_C3, SPECIES_C2, SPECIES_C1, SPECIES_COx, SPECIES_N2,

   // Species related to sulphur bearing source-rocka.
   SPECIES_H2S,
   SPECIES_LSC,
   SPECIES_C15_AT,
   SPECIES_C6_MINUS_14_BT,
   SPECIES_C6_MINUS_14_DBT,
   SPECIES_C6_MINUS_14_BP,
   SPECIES_C15_PLUS_ARO_S,
   SPECIES_C15_PLUS_SAT_S,
   SPECIES_C6_MINUS_14_SAT_S,
   SPECIES_C6_MINUS_14_ARO_S,


   // General genex properties.
   EXPULSION_API_INST,                  EXPULSION_API_CUM,
   EXPULSION_CONDENSATE_GAS_RATIO_INST, EXPULSION_CONDENSATE_GAS_RATIO_CUM,
   EXPULSION_GAS_OIL_RATIO_INST,        EXPULSION_GAS_OIL_RATIO_CUM,
   EXPULSION_GAS_WETNESS_INST,          EXPULSION_GAS_WETNESS_CUM,
   EXPULSION_AROMATICITY_INST,          EXPULSION_AROMATICITY_CUM,
   KEROGEN_CONVERSION_RATIO,

   OIL_GENERATED_CUM,                   OIL_GENERATED_RATE, 
   OIL_EXPELLED_CUM,                    OIL_EXPELLEDRATE,
   HC_GAS_GENERATED_CUM,                HC_GAS_GENERATED_RATE,
   HC_GAS_EXPELLED_CUM,                 HC_GAS_EXPELLED_RATE,
   DRY_GAS_GENERATED_CUM,               DRY_GAS_GENERATED_RATE,
   DRY_GAS_EXPELLED_CUM,                DRY_GAS_EXPELLED_RATE,
   WET_GAS_GENERATED_CUM,               WET_GAS_GENERATED_RATE,
   WET_GAS_EXPELLED_CUM,                WET_GAS_EXPELLED_RATE,

   // Concentration of all transported component.
   GENEX_PROPERTY_CONCENTRATIONS,

   /// Vapour and liquid density and viscosity.
   PVT_PROPERTIES,

   /// The relative permeabilities of the three phases.
   RELATIVE_PERMEABILITY,

   // A debugging (might be useful in general) output.
   VOLUME_CALCULATIONS,

   // A debugging (might be useful in general) output.
   TRANSPORTED_VOLUME_CALCULATIONS,


#if 0
   //
   SPECIES_ASPHALTENE_CONCENTRATION, SPECIES_RESIN_CONCENTRATION, SPECIES_C15_PLUS_ARO_CONCENTRATION, SPECIES_C15_PLUS_SAT_CONCENTRATION,

   // Oil components.
   SPECIES_C6_MINUS_14_ARO_CONCENTRATION, SPECIES_C6_MINUS_14_SAT_CONCENTRATION,

   // HC Gas components
   SPECIES_C5_CONCENTRATION, SPECIES_C4_CONCENTRATION, SPECIES_C3_CONCENTRATION, SPECIES_C2_CONCENTRATION, SPECIES_C1_CONCENTRATION,

   // Non HC Gas components
   SPECIES_COx_CONCENTRATION, SPECIES_N2_CONCENTRATION,
#endif

   // Saturation of water, oil and gas.
   SATURATION,

   //Averaged saturation
   AVERAGE_SATURATION,
   
   // The velocity of the hydrocarbon, both vapour and liquid.
   HC_FLUID_VELOCITY,

   //Add Properties Here ...

   //Capillary pressure
   CAPILLARY_PRESSURE,

   //fluid properties such as GOR, COR, API,
   FLUID_PROPERTIES,
   //brine density and viscosity
   BRINE_PROPERTIES,

   //time of element invasion
   TIME_OF_ELEMENT_INVASION,
   
   // ALC properties
   BASALTTHICKNESS, HLMOD, TOPBASALTALC, MOHOALC, THICKNESSCCRUSTALC, THICKNESSBASALTALC,
   ALCSMCRUST, ALCSMTOPBASALT, ALCSMMOHO, ALCORIGMANTLE, 

   // End of enum. Do not put anything after this.
   ENDPROPERTYLIST
};






#if 0
namespace MajorProperty {

   enum MajorPropertyName {
      DIFFUSIVITY,             //!< Vector property.
      POROSITY,                //!< Vector property.
      VELOCITY,                //!< Vector property.
      REFLECTIVITY,            //!< Vector property.
      SONIC,                   //!< Only output in 1d, vector property.
      BULK_DENSITY,            //!< Vector property.
      THERMAL_CONDUCTIVITY,    //!< Vector property.
      PERMEABILITY,            //!< Permeability, 2 components in 3d (horizontal and vertical) and 1 component in 1d (vertical), vector property.
      DEPTH,                   //!< Scalar property.
      HEAT_FLOW,               //!< Heat-flow, 3 components in 3d (-x, -y, and -z) and 1 component in 1d (-z), scalar property.
      FLUID_VELOCITY,          //!< Fluid-velocity, 3 components in 3d (-x, -y, and -z) and 1 component in 1d (-z), scalar property.
//       ISOSTATIC_MASS,          //!< ?
//       ISOSTATIC_DEFLECTION,    //!< ?
//       ISOSTATIC_WB,            //!< ?
//       MASS_FLUX,               //!< ?
      TEMPERATURE,             //!< Scalar property.
      PORE_PRESSURE,           //!< Scalar property.
      HYDROSTATIC_PRESSURE,    //!< Scalar property.
      LITHOSTATIC_PRESSURE,    //!< Scalar property.
      OVERPRESSURE,            //!< Scalar property.
      VES,                     //!< Scalar property.
      VR,                      //!< Scalar property.
      BIOMARKERS,              //!< Only output in 1d, HopaneIsomerisation, SteraneIsomerisation and SteraneAromatisation, scalar property.
      ILLITE_FRACTION,         //!< Only output in 1d, scalar property.
      ALLOCHTHONOUS_LITHOLOGY, //!< Only output in 3d, scalar property.
      EROSION_FACTOR,          //!< Scalar property.
      FAULT_ELEMENTS,          //!< Elements containing faults, only output in 3d, scalar property.
      FCT_CORRECTION,          //!< Scalar property.
      THICKNESS,               //!< The computed thickness, scalar property.
      THICKNESS_ERROR,         //!< Error in the input and computed thicknesses, scalar property.
      CHEMICAL_COMPACTION,     //!< Chemical compaction values, these should never be output, scalar property.
      LITHOLOGY,               //!< Scalar property.
      BASALTTHICKNESS,         //!< Smooth basalt thickness
      HLMOD,                   //!< Depth of lithosphere below ECT
      TOPBASALTALC,            //!< Top of basalt layer, calculated in ALC
      MOHOALC,                 //!< Bottom of basalt layer, calculated in ALC
      THICKNESSCCRUSTALC,      //!< Effective crustalThickness excluding Basalt
      THICKNESSBASALTALC,      //!< approximated thickness of Basalt
      ALCSMCRUST,              //!< smooth cont crust thickness
      ALCSMTOPBASALT,          //!< smooth basalt thickness
      ALCSMMOHO,               //!< calculated depth to top of smooth basalt
      ALCORIGMANTLE,           //!< base of uplifted mantle
      // Add additional properties before here.

      UNKNOWN_PROPERTY         //!< 
   };

   Interface::PropertyOutputOption getMaximumOutputOption ( const MajorPropertyName property,
                                                            const CalculationMode   calculation );

   Interface::PropertyOutputOption getMinimumOutputOption ( const MajorPropertyName property,
                                                            const CalculationMode   calculation );

   Interface::PropertyOutputOption getMaximumOutputOption ( const std::string&      property,
                                                            const CalculationMode   calculation );

   Interface::PropertyOutputOption getMinimumOutputOption ( const std::string&      property,
                                                            const CalculationMode   calculation );

   const std::string& MajorPropertyNameImage ( const MajorPropertyName property );

   const MajorPropertyName MajorPropertyNameValue ( const std::string property );

   const int NumberOfMajorProperties = int ( UNKNOWN_PROPERTY ) + 1;

}
#endif

typedef vector<PropertyList> PropListVec;

PropertyList& operator++(PropertyList& pl);
PropertyList  operator++(PropertyList& pl, int);

PropertyList getPropertyList ( const std::string& name );

const int PropertyListSize = int(ENDPROPERTYLIST);

const std::string& propertyListName ( const PropertyList property );

const string PropertyName[] = {
  "BulkDensityVec",
  "DiffusivityVec",
  "PermeabilityHVec",
  "PermeabilityVec",
  "PorosityVec",
  "ReflectivityVec",
  "SonicVec",
  "ThCondVec",
  "VelocityVec",
  "VrVec",
  "MaxVes",
  "Depth",
  "HeatFlow",
  "HeatFlowY",
  "HeatFlowZ",
  "HeatFlow_",
  "FluidVelocity",
  "FluidVelocityY",
  "FluidVelocityZ",
  "IsoStaticMass",
  "IsoStaticDeflection",
  "IsoStaticWaterBottom",
  "Massflux",
  "Massflux_",
  "Temperature",
  "Pressure",
  "HydroStaticPressure",
  "LithoStaticPressure",
  "OverPressure",
  "FracturePressure",
  "Ves",
  "Biomarkers",
  "SteraneAromatisation", 
  "SteraneIsomerisation",
  "HopaneIsomerisation",
  "IlliteFraction",
  "AllochthonousLithology",
  "ErosionFactor",
  "FaultElements",
  "FCTCorrection",
  "Thickness",
  "ThicknessError",
  "ChemicalCompaction",
  "Lithology",
  "TwoWayTime",
  "TwoWayTimeResidual",
  "CapillaryPressureGas100",
  "CapillaryPressureGas0",
  "CapillaryPressureOil100",
  "CapillaryPressureOil0",
  "GenexProperties",
  "asphaltenesExpelledCumulative",
  "resinsExpelledCumulative",
  "C15+AroExpelledCumulative",
  "C15+SatExpelledCumulative",
  "C6-14AroExpelledCumulative",
  "C6-14SatExpelledCumulative",
  "C5ExpelledCumulative",
  "C4ExpelledCumulative",
  "C3ExpelledCumulative",
  "C2ExpelledCumulative",
  "C1ExpelledCumulative",
  "COxExpelledCumulative",
  "N2ExpelledCumulative",
  "H2SExpelledCumulative",
  "LSCExpelledCumulative",
  "C15+ATExpelledCumulative",
  "C6-14BTExpelledCumulative",
  "C6-14DBTExpelledCumulative",
  "C6-14BPExpelledCumulative",
  "C15+AroSExpelledCumulative",
  "C15+SatSExpelledCumulative",
  "C6-14SatSExpelledCumulative",
  "C6-14AroSExpelledCumulative",
  "InstantaneousExpulsionApi",
  "CumulativeExpulsionApi", 
  "InstantaneousExpulsionCondensateGasRatio",
  "CumulativeExpulsionCondensateGasRatio",
  "InstantaneousExpulsionGasOilRatio",
  "CumulativeExpulsionGasOilRatio",
  "InstantaneousExpulsionGasWetness",
  "CumulativeExpulsionGasWetness",
  "InstantaneousExpulsionAromaticity",
  "CumulativeExpulsionAromaticity",
  "KerogenConversionRatio", 
  "OilGeneratedCumulative",
  "OilGeneratedRate",
  "OilExpelledCumulative",
  "OilExpelledRate",
  "HcGasGeneratedCumulative",
  "HcGasGeneratedRate",
  "HcGasExpelledCumulative",
  "HcGasExpelledRate",
  "DryGasGeneratedCumulative",
  "DryGasGeneratedRate",
  "DryGasExpelledCumulative",
  "DryGasExpelledRate",
  "WetGasGeneratedCumulative",
  "WetGasGeneratedRate",
  "WetGasExpelledCumulative",
  "WetGasExpelledRate", 
//   "ExpulsionApiInst",
//   "ExpulsionApiCum",
//   "ExpulsionCondensateGasRatioInst",
//   "ExpulsionCondensateGasRatioCum",
//   "ExpulsionGasOilRatioInst",
//   "ExpulsionGasOilRatioCum",
//   "ExpulsionGasWetnessInst",
//   "ExpulsionGasWetnessCum",
//   "ExpulsionAromaticityInst",
//   "ExpulsionAromaticityCum",
//   "KerogenConversionRatio", 
//   "OilGeneratedCum",
//   "OilGeneratedRate",
//   "OilExpelledCum",
//   "OilExpelledRate",
//   "HcGasGeneratedCum",
//   "HcGasGeneratedRate",
//   "HcGasExpelledCum",
//   "HcGasExpelledRate",
//   "DryGasGeneratedCum",
//   "DryGasGeneratedRate",
//   "DryGasExpelledCum",
//   "DryGasExpelledRate",
//   "WetGasGeneratedCum",
//   "WetGasGeneratedRate",
//   "WetGasExpelledCum",
//   "WetGasExpelledRate", 
  "Concentrations",
  "PVTProperties",
  "RelativePermeability",
  "ElementVolumes",
  "TransportedMass",
  "Saturations",
  "AverageSaturations",
  "CapillaryPressures",
  "FluidProperties",
  "BrineProperties",
  "HcFluidVelocity",
  "TimeOfInvasion",
  "ALCSmBasaltThickness",
  "ALCMaxAsthenoMantleDepth",
  "ALCStepTopBasaltDepth",
  "ALCStepMohoDepth",
  "ALCStepContCrustThickness",
  "ALCStepBasaltThickness",
  "ALCSmContCrustThickness",
  "ALCSmTopBasaltDepth",
  "ALCSmMohoDepth",
  "ALCOrigLithMantleDepth",
  "UNKNOWN"
};

class TimeFilter
{
 public:
  TimeFilter();
  ~TimeFilter();

  bool propertyIsSelected ( const PropertyList propertyId ) const;

  OutputOption PropertyOutputOption[PropertyListSize];
  OutputOption getOutputRange(const string & outputOption);

  void setFilter(const string& propertyName, const string& outputOption);

  void setFilter(const string& propertyName, const DataAccess::Interface::PropertyOutputOption outputOption);

  bool IsSomethingSelected()const;

   const std::string& getPropertyName(PropertyList propertyId) const {return PropertyName[propertyId];}

   PropertyList getPropertylist ( const std::string& propertyName ) const;

 private:

   std::map <std::string, PropertyList> m_string2PropertyName;
  
};

#endif /* _TIMEFILTER_H_ */
