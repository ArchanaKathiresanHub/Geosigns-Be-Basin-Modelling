#include "PropertyOutputConstraints.h"

#include <iomanip>

const std::string& PropertyOutputOptionImage ( const Interface::PropertyOutputOption option ) {

   static std::string outputOptionImages [ 5 ] = { "NO_OUTPUT",
                                                   "SOURCE_ROCK_ONLY_OUTPUT",
                                                   "SEDIMENTS_ONLY_OUTPUT",
                                                   "SEDIMENTS_AND_BASEMENT_OUTPUT",
                                                   "UNKNOWN_OPTION"};


   if ( option >= Interface::NO_OUTPUT and option <= Interface::SEDIMENTS_AND_BASEMENT_OUTPUT ) {
      return outputOptionImages [ option ];
   } else {
      return outputOptionImages [ 4 ];
   } 

}


const ApplicableOutputRegion::ApplicableRegion PropertyOutputConstraints::s_propertyOutputRegion [ PropertyListSize ] =  { ApplicableOutputRegion::SEDIMENTS_AND_BASEMENT,      /* DiffusivityVec         */
                                                                                                                           ApplicableOutputRegion::SEDIMENTS_ONLY,              /* PorosityVec            */
                                                                                                                           ApplicableOutputRegion::SEDIMENTS_AND_BASEMENT,      /* VelocityVec            */
                                                                                                                           ApplicableOutputRegion::SEDIMENTS_AND_BASEMENT,      /* ReflectivityVec        */
                                                                                                                           ApplicableOutputRegion::SEDIMENTS_AND_BASEMENT,      /* SonicVec               */
                                                                                                                           ApplicableOutputRegion::SEDIMENTS_AND_BASEMENT,      /* BulkDensityVec         */
                                                                                                                           ApplicableOutputRegion::SEDIMENTS_AND_BASEMENT,      /* ThCondVec              */
                                                                                                                           ApplicableOutputRegion::SEDIMENTS_ONLY,              /* PermeabilityVec        */
                                                                                                                           ApplicableOutputRegion::SEDIMENTS_ONLY,              /* PermeabilityHVec       */
                                                                                                                           ApplicableOutputRegion::SEDIMENTS_ONLY,              /* Vr                     */
                                                                                                                           ApplicableOutputRegion::SEDIMENTS_AND_BASEMENT,      /* Depth                  */
                                                                                                                           ApplicableOutputRegion::SEDIMENTS_AND_BASEMENT,      /* HeatFlow               */
                                                                                                                           ApplicableOutputRegion::SEDIMENTS_AND_BASEMENT,      /* HeatFlowY              */
                                                                                                                           ApplicableOutputRegion::SEDIMENTS_AND_BASEMENT,      /* HeatFlowZ              */
                                                                                                                           ApplicableOutputRegion::SEDIMENTS_AND_BASEMENT,      /* HeatFlow_              */
                                                                                                                           ApplicableOutputRegion::SEDIMENTS_ONLY,              /* FluidVelocity          */
                                                                                                                           ApplicableOutputRegion::SEDIMENTS_ONLY,              /* FluidVelocityY         */
                                                                                                                           ApplicableOutputRegion::SEDIMENTS_ONLY,              /* FluidVelocityZ         */
                                                                                                                           ApplicableOutputRegion::SEDIMENTS_ONLY,              /* IsoStaticMass          */
                                                                                                                           ApplicableOutputRegion::SEDIMENTS_ONLY,              /* IsoStaticDeflection    */
                                                                                                                           ApplicableOutputRegion::SEDIMENTS_ONLY,              /* IsoStaticWaterBottom   */
                                                                                                                           ApplicableOutputRegion::SEDIMENTS_ONLY,              /* Massflux               */
                                                                                                                           ApplicableOutputRegion::SEDIMENTS_ONLY,              /* Massflux_              */
                                                                                                                           ApplicableOutputRegion::SEDIMENTS_ONLY,              /* MaxVes                 */
                                                                                                                           ApplicableOutputRegion::SEDIMENTS_AND_BASEMENT,      /* Temperature            */
                                                                                                                           ApplicableOutputRegion::SEDIMENTS_ONLY,              /* Pressure               */
                                                                                                                           ApplicableOutputRegion::SEDIMENTS_ONLY,              /* HydroStaticPressure    */
                                                                                                                           ApplicableOutputRegion::SEDIMENTS_AND_BASEMENT,      /* LithoStaticPressure    */
                                                                                                                           ApplicableOutputRegion::SEDIMENTS_ONLY,              /* OverPressure           */
                                                                                                                           ApplicableOutputRegion::SEDIMENTS_ONLY,              /* Ves                    */
                                                                                                                           ApplicableOutputRegion::SEDIMENTS_ONLY,              /* Biomarkers             */
                                                                                                                           ApplicableOutputRegion::SEDIMENTS_ONLY,              /* SteraneAromatisation   */
                                                                                                                           ApplicableOutputRegion::SEDIMENTS_ONLY,              /* SteraneIsomerisation   */
                                                                                                                           ApplicableOutputRegion::SEDIMENTS_ONLY,              /* HopaneIsomerisation    */
                                                                                                                           ApplicableOutputRegion::SEDIMENTS_ONLY,              /* IlliteFraction         */
                                                                                                                           ApplicableOutputRegion::SEDIMENTS_ONLY,              /* AllochthonousLithology */
                                                                                                                           ApplicableOutputRegion::SEDIMENTS_ONLY,              /* ErosionFactor          */
                                                                                                                           ApplicableOutputRegion::SEDIMENTS_ONLY,              /* FaultElements          */
                                                                                                                           ApplicableOutputRegion::SEDIMENTS_ONLY,              /* FCTCorrection          */
                                                                                                                           ApplicableOutputRegion::SEDIMENTS_AND_BASEMENT,      /* Thickness              */
                                                                                                                           ApplicableOutputRegion::SEDIMENTS_ONLY,              /* ThicknessError         */
                                                                                                                           ApplicableOutputRegion::SEDIMENTS_ONLY,              /* ChemicalCompaction     */
                                                                                                                           ApplicableOutputRegion::SEDIMENTS_AND_BASEMENT,      /* Lithology              */
                                                                                                                           ApplicableOutputRegion::SEDIMENTS_ONLY,              /* CapillaryPressureGas100 */
                                                                                                                           ApplicableOutputRegion::SEDIMENTS_ONLY,              /* CapillaryPressureGas0   */
                                                                                                                           ApplicableOutputRegion::SEDIMENTS_ONLY,              /* CapillaryPressureOil100 */
                                                                                                                           ApplicableOutputRegion::SEDIMENTS_ONLY,              /* CapillaryPressureOil0   */
                                                                                                                           ApplicableOutputRegion::SOURCE_ROCK_ONLY,            /* genex-properties  */  /* not an output property */
                                                                                                                           ApplicableOutputRegion::SOURCE_ROCK_ONLY,            /* asphaltenes     */
                                                                                                                           ApplicableOutputRegion::SOURCE_ROCK_ONLY,            /* resins          */
                                                                                                                           ApplicableOutputRegion::SOURCE_ROCK_ONLY,            /* C15+Aro         */
                                                                                                                           ApplicableOutputRegion::SOURCE_ROCK_ONLY,            /* C15+Sat         */
                                                                                                                           ApplicableOutputRegion::SOURCE_ROCK_ONLY,            /* C6-14Aro        */
                                                                                                                           ApplicableOutputRegion::SOURCE_ROCK_ONLY,            /* C6-14Sat        */
                                                                                                                           ApplicableOutputRegion::SOURCE_ROCK_ONLY,            /* C5              */
                                                                                                                           ApplicableOutputRegion::SOURCE_ROCK_ONLY,            /* C4              */
                                                                                                                           ApplicableOutputRegion::SOURCE_ROCK_ONLY,            /* C3              */
                                                                                                                           ApplicableOutputRegion::SOURCE_ROCK_ONLY,            /* C2              */
                                                                                                                           ApplicableOutputRegion::SOURCE_ROCK_ONLY,            /* C1              */
                                                                                                                           ApplicableOutputRegion::SOURCE_ROCK_ONLY,            /* COx             */
                                                                                                                           ApplicableOutputRegion::SOURCE_ROCK_ONLY,            /* N2              */
                                                                                                                           ApplicableOutputRegion::SOURCE_ROCK_ONLY,            /* H2S             */
                                                                                                                           ApplicableOutputRegion::SOURCE_ROCK_ONLY,            /* LSC             */
                                                                                                                           ApplicableOutputRegion::SOURCE_ROCK_ONLY,            /* C15+AT          */
                                                                                                                           ApplicableOutputRegion::SOURCE_ROCK_ONLY,            /* C6-14 BT        */
                                                                                                                           ApplicableOutputRegion::SOURCE_ROCK_ONLY,            /* C6-14 DBT       */
                                                                                                                           ApplicableOutputRegion::SOURCE_ROCK_ONLY,            /* C6-14 BP        */
                                                                                                                           ApplicableOutputRegion::SOURCE_ROCK_ONLY,            /* C15+AroS        */
                                                                                                                           ApplicableOutputRegion::SOURCE_ROCK_ONLY,            /* C15+SatS        */
                                                                                                                           ApplicableOutputRegion::SOURCE_ROCK_ONLY,            /* C6-14 SatS      */
                                                                                                                           ApplicableOutputRegion::SOURCE_ROCK_ONLY,            /* C6-14 AroS      */
                                                                                                                           ApplicableOutputRegion::SOURCE_ROCK_ONLY,            /* InstantaneousExpulsionApi                 */
                                                                                                                           ApplicableOutputRegion::SOURCE_ROCK_ONLY,            /* CumulativeExpulsionApi                    */
                                                                                                                           ApplicableOutputRegion::SOURCE_ROCK_ONLY,            /* InstantaneousExpulsionCondensateGasRatio  */
                                                                                                                           ApplicableOutputRegion::SOURCE_ROCK_ONLY,            /* CumulativeExpulsionCondensateGasRatio     */
                                                                                                                           ApplicableOutputRegion::SOURCE_ROCK_ONLY,            /* InstantaneousExpulsionGasOilRatio         */
                                                                                                                           ApplicableOutputRegion::SOURCE_ROCK_ONLY,            /* CumulativeExpulsionGasOilRatio            */
                                                                                                                           ApplicableOutputRegion::SOURCE_ROCK_ONLY,            /* InstantaneousExpulsionGasWetness          */
                                                                                                                           ApplicableOutputRegion::SOURCE_ROCK_ONLY,            /* CumulativeExpulsionGasWetness             */
                                                                                                                           ApplicableOutputRegion::SOURCE_ROCK_ONLY,            /* InstantaneousExpulsionAromaticity         */
                                                                                                                           ApplicableOutputRegion::SOURCE_ROCK_ONLY,            /* CumulativeExpulsionAromaticity            */
                                                                                                                           ApplicableOutputRegion::SOURCE_ROCK_ONLY,            /* KerogenConversionRatio                    */
                                                                                                                           ApplicableOutputRegion::SOURCE_ROCK_ONLY,            /* OilGeneratedCumulative                    */
                                                                                                                           ApplicableOutputRegion::SOURCE_ROCK_ONLY,            /* OilGeneratedRate                          */
                                                                                                                           ApplicableOutputRegion::SOURCE_ROCK_ONLY,            /* OilExpelledCumulative                     */
                                                                                                                           ApplicableOutputRegion::SOURCE_ROCK_ONLY,            /* OilExpelledRate                           */
                                                                                                                           ApplicableOutputRegion::SOURCE_ROCK_ONLY,            /* HcGasGeneratedCumulative                  */
                                                                                                                           ApplicableOutputRegion::SOURCE_ROCK_ONLY,            /* HcGasGeneratedRate                        */
                                                                                                                           ApplicableOutputRegion::SOURCE_ROCK_ONLY,            /* HcGasExpelledCumulative                   */
                                                                                                                           ApplicableOutputRegion::SOURCE_ROCK_ONLY,            /* HcGasExpelledRate                         */
                                                                                                                           ApplicableOutputRegion::SOURCE_ROCK_ONLY,            /* DryGasGeneratedCumulative                 */
                                                                                                                           ApplicableOutputRegion::SOURCE_ROCK_ONLY,            /* DryGasGeneratedRate                       */
                                                                                                                           ApplicableOutputRegion::SOURCE_ROCK_ONLY,            /* DryGasExpelledCumulative                  */
                                                                                                                           ApplicableOutputRegion::SOURCE_ROCK_ONLY,            /* DryGasExpelledRate                        */
                                                                                                                           ApplicableOutputRegion::SOURCE_ROCK_ONLY,            /* WetGasGeneratedCumulative                 */
                                                                                                                           ApplicableOutputRegion::SOURCE_ROCK_ONLY,            /* WetGasGeneratedRate                       */
                                                                                                                           ApplicableOutputRegion::SOURCE_ROCK_ONLY,            /* WetGasExpelledCumulative                  */
                                                                                                                           ApplicableOutputRegion::SOURCE_ROCK_ONLY,            /* WetGasExpelledRate                        */
                                                                                                                           ApplicableOutputRegion::SEDIMENTS_ONLY,              /* Concentrations                            */
                                                                                                                           ApplicableOutputRegion::SEDIMENTS_ONLY,              /* PVT Properties: hc-density and -viscocity */
                                                                                                                           ApplicableOutputRegion::SEDIMENTS_ONLY,              /* Rel perm calculations                     */
                                                                                                                           ApplicableOutputRegion::SEDIMENTS_ONLY,              /* Volume calculations                       */
                                                                                                                           ApplicableOutputRegion::SEDIMENTS_ONLY,              /* Transported volume calculations           */
                                                                                                                           ApplicableOutputRegion::SEDIMENTS_ONLY,              /* Saturations                               */
															   ApplicableOutputRegion::SEDIMENTS_ONLY,              /* AverageSaturations                        */
                                                                                                                           ApplicableOutputRegion::SEDIMENTS_ONLY,              /* HC fluid velocity                         */
															   ApplicableOutputRegion::SEDIMENTS_ONLY,              /* CapillaryPressure                         */
                                                                                                                           ApplicableOutputRegion::SEDIMENTS_ONLY,              /* Fluid Properties such as GOR, COR, OilAPI */
                                                                                                                           ApplicableOutputRegion::SEDIMENTS_ONLY,              /* Brine properties viscosity density        */
															   ApplicableOutputRegion::SEDIMENTS_ONLY,               /* Time of invasion                          */				
                                                                                                                           ApplicableOutputRegion::SEDIMENTS_AND_BASEMENT,      /* ALCSmBasaltThickness   */
                                                                                                                           ApplicableOutputRegion::SEDIMENTS_AND_BASEMENT,      /* ALCMaxAsthenoMantleDepth */
                                                                                                                           ApplicableOutputRegion::SEDIMENTS_AND_BASEMENT,      /* ALCStepTopBasaltDepth    */
                                                                                                                           ApplicableOutputRegion::SEDIMENTS_AND_BASEMENT,      /* ALCStepMohoDepth         */
                                                                                                                           ApplicableOutputRegion::SEDIMENTS_AND_BASEMENT,      /* ALCStepContCrustThickness*/
                                                                                                                           ApplicableOutputRegion::SEDIMENTS_AND_BASEMENT,      /* ALCStepBasaltThickness   */
                                                                                                                           ApplicableOutputRegion::SEDIMENTS_AND_BASEMENT,      /* ALCSmContCrustThickness  */
                                                                                                                           ApplicableOutputRegion::SEDIMENTS_AND_BASEMENT,      /* ALCSmTopBasaltDepth */
                                                                                                                           ApplicableOutputRegion::SEDIMENTS_AND_BASEMENT,      /* ALCSmMohoDepth      */
                                                                                                                           ApplicableOutputRegion::SEDIMENTS_AND_BASEMENT       /* ALCOrigMantle       */
											  



};

const Interface::PropertyOutputOption PropertyOutputConstraints::s_calculationModeMaxima [ NumberOfCalculationModes ]  = { Interface::SEDIMENTS_AND_BASEMENT_OUTPUT, // HYDROSTATIC_DECOMPACTION_MODE
                                                                                                                           Interface::SEDIMENTS_AND_BASEMENT_OUTPUT, // HYDROSTATIC_HIGH_RES_DECOMPACTION_MODE
                                                                                                                           Interface::SEDIMENTS_AND_BASEMENT_OUTPUT, // HYDROSTATIC_TEMPERATURE_MODE
                                                                                                                           Interface::SEDIMENTS_ONLY_OUTPUT,         // OVERPRESSURE_MODE
                                                                                                                           Interface::SEDIMENTS_AND_BASEMENT_OUTPUT, // OVERPRESSURED_TEMPERATURE_MODE
                                                                                                                           Interface::SEDIMENTS_AND_BASEMENT_OUTPUT, // COUPLED_HIGH_RES_DECOMPACTION_MODE
                                                                                                                           Interface::SEDIMENTS_AND_BASEMENT_OUTPUT, // PRESSURE_AND_TEMPERATURE_MODE
                                                                                                                           Interface::SEDIMENTS_AND_BASEMENT_OUTPUT, // HYDROSTATIC_DARCY_MODE
                                                                                                                           Interface::SEDIMENTS_AND_BASEMENT_OUTPUT, // COUPLED_DARCY_MODE
                                                                                                                           Interface::NO_OUTPUT };                   // NO_CALCULATION_MODE


/* 

    s_outputPermitted: indicated whether, or not, a property can be output for a given calculation mode.

    HD   : Hydrostatic decompaction;
    HRD  : High-res decompaction (hydrostatic);
    T    : Temperature (hydrostatic);
    O    : Overpressure;
    LCT  : Loosely-coupled temperature;
    CHRD : High-res decompaction (coupled);
    FC   : Iteratively-coupled;
    HD   : Hydrostatic Darcy;
    HD   : Coupled Darcy;
    NC   : No calculation.



                                                                                                                HD      HRD     T     O     LCT   CHRD    FC      HD     CD     NC                               */
const bool PropertyOutputConstraints::s_outputPermitted [ PropertyListSize ][ NumberOfCalculationModes ] = {{ false, false,  true, false,  true, false,  true,  true,  true, false },  /* DiffusivityVec         */
                                                                                                            {  true,  true,  true,  true, false,  true,  true,  true,  true, false },  /* PorosityVec            */
                                                                                                            { false, false,  true, false,  true, false,  true,  true,  true, false },  /* VelocityVec            */
                                                                                                            { false, false,  true, false,  true, false,  true,  true,  true, false },  /* ReflectivityVec        */
                                                                                                            { false, false,  true,  true,  true, false,  true,  true,  true, false },  /* SonicVec               */
                                                                                                            {  true, false,  true,  true, false, false,  true,  true,  true, false },  /* BulkDensityVec         */
                                                                                                            { false, false,  true, false,  true, false,  true,  true,  true, false },  /* ThCondVec              */
                                                                                                            { false, false,  true,  true, false, false,  true,  true,  true, false },  /* PermeabilityVec        */
                                                                                                            { false, false,  true,  true, false, false,  true,  true,  true, false },  /* PermeabilityHVec       */
                                                                                                            { false, false,  true, false,  true, false,  true,  true,  true, false },  /* Vr                     */
                                                                                                            {  true,  true,  true,  true, false,  true,  true,  true,  true, false },  /* Depth                  */
                                                                                                            { false, false,  true, false,  true, false,  true,  true,  true, false },  /* HeatFlow               */
                                                                                                            { false, false,  true, false,  true, false,  true,  true,  true, false },  /* HeatFlowY              */
                                                                                                            { false, false,  true, false,  true, false,  true,  true,  true, false },  /* HeatFlowZ              */
                                                                                                            { false, false,  true, false,  true, false,  true,  true,  true, false },  /* HeatFlow_              */
                                                                                                            { false, false, false,  true,  true, false,  true, false,  true, false },  /* FluidVelocity          */
                                                                                                            { false, false, false,  true,  true, false,  true, false,  true, false },  /* FluidVelocityY         */
                                                                                                            { false, false, false,  true,  true, false,  true, false,  true, false },  /* FluidVelocityZ         */
                                                                                                            { false, false,  true,  true,  true, false,  true,  true,  true, false },  /* IsoStaticMass          */
                                                                                                            { false, false,  true,  true,  true, false,  true,  true,  true, false },  /* IsoStaticDeflection    */
                                                                                                            { false, false,  true,  true,  true, false,  true,  true,  true, false },  /* IsoStaticWaterBottom   */
                                                                                                            { false, false,  true,  true,  true, false,  true,  true,  true, false },  /* Massflux               */
                                                                                                            { false, false,  true,  true,  true, false,  true,  true,  true, false },  /* Massflux_              */
                                                                                                            {  true,  true,  true,  true, false,  true,  true,  true,  true, false },  /* MaxVes                 */
                                                                                                            { false, false,  true, false,  true, false,  true,  true,  true, false },  /* Temperature            */
                                                                                                            { false, false,  true,  true, false, false,  true,  true,  true, false },  /* Pressure               */
                                                                                                            { false, false,  true,  true, false, false,  true,  true,  true, false },  /* HydroStaticPressure    */
                                                                                                            {  true, true,   true,  true, false,  true,  true,  true,  true, false },  /* LithoStaticPressure    */
                                                                                                            { false, false,  true,  true, false, false,  true,  true,  true, false },  /* OverPressure           */
                                                                                                            {  true,  true,  true,  true, false,  true,  true,  true,  true, false },  /* Ves                    */
                                                                                                            { false, false,  true,  true,  true, false,  true,  true,  true, false },  /* Biomarkers             */
                                                                                                            { false, false,  true,  true,  true, false,  true,  true,  true, false },  /* SteraneAromatisation   */
                                                                                                            { false, false,  true,  true,  true, false,  true,  true,  true, false },  /* SteraneIsomerisation   */
                                                                                                            { false, false,  true,  true,  true, false,  true,  true,  true, false },  /* HopaneIsomerisation    */
                                                                                                            { false, false,  true,  true,  true, false,  true,  true,  true, false },  /* IlliteFraction         */
                                                                                                            { false, false, false,  true, false, false,  true, false,  true, false },  /* AllochthonousLithology */
                                                                                                            { false, false,  true,  true,  true, false,  true,  true,  true, false },  /* ErosionFactor          */
                                                                                                            { false, false, false,  true, false, false,  true, false,  true, false },  /* FaultElements          */
                                                                                                            { false, false, false,  true,  true, false,  true, false,  true, false },  /* FCTCorrection          */
                                                                                                            {  true,  true,  true,  true,  true, false,  true,  true,  true, false },  /* Thickness              */
                                                                                                            { false, false, false,  true, false, false,  true, false,  true, false },  /* ThicknessError         */
                                                                                                            { false, false, false, false, false, false, false, false, false, false },  /* ChemicalCompaction     */
                                                                                                            {  true,  true,  true,  true,  true,  true,  true,  true,  true, false },  /* Lithology              */
                                                                                                            { false, false,  true,  true, false, false,  true,  true,  true, false },  /* CapillaryPressureGas100 */
                                                                                                            { false, false,  true,  true, false, false,  true,  true,  true, false },  /* CapillaryPressureGas0   */
                                                                                                            { false, false,  true,  true, false, false,  true,  true,  true, false },  /* CapillaryPressureOil100 */
                                                                                                            { false, false,  true,  true, false, false,  true,  true,  true, false },  /* CapillaryPressureOil0   */
                                                                                                            { false, false,  true, false,  true, false,  true,  true,  true, false },  /* genex-properties       */  /* not an output property */
                                                                                                            { false, false,  true, false,  true, false,  true,  true,  true, false },  /* asphaltines            */
                                                                                                            { false, false,  true, false,  true, false,  true,  true,  true, false },  /* resins                 */
                                                                                                            { false, false,  true, false,  true, false,  true,  true,  true, false },  /* c15+aro                */
                                                                                                            { false, false,  true, false,  true, false,  true,  true,  true, false },  /* c15+sat                */
                                                                                                            { false, false,  true, false,  true, false,  true,  true,  true, false },  /* c6-14Aro               */
                                                                                                            { false, false,  true, false,  true, false,  true,  true,  true, false },  /* c6-14Sat               */
                                                                                                            { false, false,  true, false,  true, false,  true,  true,  true, false },  /* C5                     */
                                                                                                            { false, false,  true, false,  true, false,  true,  true,  true, false },  /* C4                     */
                                                                                                            { false, false,  true, false,  true, false,  true,  true,  true, false },  /* C3                     */
                                                                                                            { false, false,  true, false,  true, false,  true,  true,  true, false },  /* C2                     */
                                                                                                            { false, false,  true, false,  true, false,  true,  true,  true, false },  /* C1                     */
                                                                                                            { false, false,  true, false,  true, false,  true,  true,  true, false },  /* COx                    */
                                                                                                            { false, false,  true, false,  true, false,  true,  true,  true, false },  /* N2                     */
                                                                                                            { false, false,  true, false,  true, false,  true,  true,  true, false },  /* H2S                    */
                                                                                                            { false, false,  true, false,  true, false,  true,  true,  true, false },  /* LSC                    */
                                                                                                            { false, false,  true, false,  true, false,  true,  true,  true, false },  /* C15+AT                 */
                                                                                                            { false, false,  true, false,  true, false,  true,  true,  true, false },  /* C6-14 BT               */
                                                                                                            { false, false,  true, false,  true, false,  true,  true,  true, false },  /* C6-14 DBT              */
                                                                                                            { false, false,  true, false,  true, false,  true,  true,  true, false },  /* C6-14 BP               */
                                                                                                            { false, false,  true, false,  true, false,  true,  true,  true, false },  /* C15+AroS               */
                                                                                                            { false, false,  true, false,  true, false,  true,  true,  true, false },  /* C15+SatS               */
                                                                                                            { false, false,  true, false,  true, false,  true,  true,  true, false },  /* C6-14 SatS             */
                                                                                                            { false, false,  true, false,  true, false,  true,  true,  true, false },  /* C6-14 AroS             */
                                                                                                            { false, false,  true, false,  true, false,  true,  true,  true, false },  /* InstantaneousExpulsionApi                 */ 
                                                                                                            { false, false,  true, false,  true, false,  true,  true,  true, false },  /* CumulativeExpulsionApi                    */ 
                                                                                                            { false, false,  true, false,  true, false,  true,  true,  true, false },  /* InstantaneousExpulsionCondensateGasRatio  */ 
                                                                                                            { false, false,  true, false,  true, false,  true,  true,  true, false },  /* CumulativeExpulsionCondensateGasRatio     */ 
                                                                                                            { false, false,  true, false,  true, false,  true,  true,  true, false },  /* InstantaneousExpulsionGasOilRatio         */ 
                                                                                                            { false, false,  true, false,  true, false,  true,  true,  true, false },  /* CumulativeExpulsionGasOilRatio            */ 
                                                                                                            { false, false,  true, false,  true, false,  true,  true,  true, false },  /* InstantaneousExpulsionGasWetness          */ 
                                                                                                            { false, false,  true, false,  true, false,  true,  true,  true, false },  /* CumulativeExpulsionGasWetness             */ 
                                                                                                            { false, false,  true, false,  true, false,  true,  true,  true, false },  /* InstantaneousExpulsionAromaticity         */ 
                                                                                                            { false, false,  true, false,  true, false,  true,  true,  true, false },  /* CumulativeExpulsionAromaticity            */ 
                                                                                                            { false, false,  true, false,  true, false,  true,  true,  true, false },  /* KerogenConversionRatio                    */ 
                                                                                                            { false, false,  true, false,  true, false,  true,  true,  true, false },  /* OilGeneratedCumulative                    */ 
                                                                                                            { false, false,  true, false,  true, false,  true,  true,  true, false },  /* OilGeneratedRate                          */ 
                                                                                                            { false, false,  true, false,  true, false,  true,  true,  true, false },  /* OilExpelledCumulative                     */ 
                                                                                                            { false, false,  true, false,  true, false,  true,  true,  true, false },  /* OilExpelledRate                           */ 
                                                                                                            { false, false,  true, false,  true, false,  true,  true,  true, false },  /* HcGasGeneratedCumulative                  */ 
                                                                                                            { false, false,  true, false,  true, false,  true,  true,  true, false },  /* HcGasGeneratedRate                        */ 
                                                                                                            { false, false,  true, false,  true, false,  true,  true,  true, false },  /* HcGasExpelledCumulative                   */ 
                                                                                                            { false, false,  true, false,  true, false,  true,  true,  true, false },  /* HcGasExpelledRate                         */ 
                                                                                                            { false, false,  true, false,  true, false,  true,  true,  true, false },  /* DryGasGeneratedCumulative                 */ 
                                                                                                            { false, false,  true, false,  true, false,  true,  true,  true, false },  /* DryGasGeneratedRate                       */ 
                                                                                                            { false, false,  true, false,  true, false,  true,  true,  true, false },  /* DryGasExpelledCumulative                  */ 
                                                                                                            { false, false,  true, false,  true, false,  true,  true,  true, false },  /* DryGasExpelledRate                        */ 
                                                                                                            { false, false,  true, false,  true, false,  true,  true,  true, false },  /* WetGasGeneratedCumulative                 */ 
                                                                                                            { false, false,  true, false,  true, false,  true,  true,  true, false },  /* WetGasGeneratedRate                       */ 
                                                                                                            { false, false,  true, false,  true, false,  true,  true,  true, false },  /* WetGasExpelledCumulative                  */ 
                                                                                                            { false, false,  true, false,  true, false,  true,  true,  true, false },  /* WetGasExpelledRate                        */ 
                                                                                                            { false, false,  true,  true,  true, false,  true,  true,  true, false },  /* Concentrations                            */ 
                                                                                                            { false, false,  true,  true,  true, false,  true,  true,  true, false },  /* PVT Properties: hc-density and -viscocity */ 
                                                                                                            { false, false,  true,  true,  true, false,  true,  true,  true, false },  /* Rel Perm calculations                     */ 
                                                                                                            { false, false,  true,  true,  true, false,  true,  true,  true, false },  /* Volume calculations                       */ 
                                                                                                            { false, false,  true,  true,  true, false,  true,  true,  true, false },  /* Transported volume calculations           */ 
                                                                                                            { false, false,  true,  true,  true, false,  true,  true,  true, false },  /* Saturations                               */
													    { false, false,  true,  true,  true, false,  true,  true,  true, false },  /* AverageSaturations                        */
                                                                                                            { false, false,  true, false,  true, false,  true,  true,  true, false },  /* HC fliud velocity                         */
													    { false, false,  true,  true,  true, false,  true,  true,  true, false },  /* CapillaryPressure                         */
													    { false, false,  true,  true,  true, false,  true,  true,  true, false },  /* Fluid Properties such as GOR, COR, OilAPI */
                                                                                                            { false, false,  true,  true,  true, false,  true,  true,  true, false },  /* Brine Properties density and viscosity    */
                                                                                                            { false, false,  true,  true,  true, false,  true,  true,  true, false },   /* Time of invasion                          */                                                                                                      
                                                                                                            {  true,  true,  true,  true, false,  true,  true, false },  /* ALCSmBasaltThickness   */
                                                                                                            {  true,  true,  true,  true, false,  true,  true, false },  /* ALCMaxAsthenoMantleDepth*/
                                                                                                            {  true,  true,  true,  true, false,  true,  true, false },  /* ALCStepTopBasaltDepth   */
                                                                                                            {  true,  true,  true,  true, false,  true,  true, false },  /* ALCStepMohoDepth        */
                                                                                                            {  true,  true,  true,  true, false,  true,  true, false },  /* ALCStepContCrustThickness*/
                                                                                                            {  true,  true,  true,  true, false,  true,  true, false },  /* ALCStepBasaltThickness   */
                                                                                                            {  true,  true,  true,  true, false,  true,  true, false },  /* ALCSmContCrustThickness  */
                                                                                                            {  true,  true,  true,  true, false,  true,  true, false },  /* ALCSmTopBasaltDepth      */
                                                                                                            {  true,  true,  true,  true, false,  true,  true, false },  /* ALCSmMohoDepth           */
                                                                                                            {  true,  true,  true,  true, false,  true,  true, false }   /* ALCOrigMantle            */																								
};


//  s_outputRequired: indicated whether, or not, a property should be output for a given calculation mode.
const bool PropertyOutputConstraints::s_outputRequired [ PropertyListSize ][ NumberOfCalculationModes ] = {{ false, false, false, false, false, false, false, false, false, false },  /* DiffusivityVec                            */
                                                                                                           {  true, false,  true,  true, false, false,  true,  true,  true, false },  /* PorosityVec                               */
                                                                                                           { false, false, false, false, false, false, false, false, false, false },  /* VelocityVec                               */
                                                                                                           { false, false, false, false, false, false, false, false, false, false },  /* ReflectivityVec                           */
                                                                                                           { false, false, false, false, false, false, false, false, false, false },  /* SonicVec                                  */
                                                                                                           { false, false, false, false, false, false, false, false, false, false },  /* BulkDensityVec                            */
                                                                                                           { false, false, false, false, false, false, false, false, false, false },  /* ThCondVec                                 */
                                                                                                           { false, false,  true,  true, false, false,  true,  true,  true, false },  /* PermeabilityVec                           */
                                                                                                           { false, false,  true,  true, false, false,  true,  true,  true, false },  /* PermeabilityHVec                          */
                                                                                                           { false, false,  true, false,  true, false,  true,  true,  true, false },  /* Vr                                        */
                                                                                                           {  true,  true,  true,  true, false,  true,  true,  true,  true, false },  /* Depth                                     */
                                                                                                           { false, false, false, false, false, false, false, false, false, false },  /* HeatFlow                                  */
                                                                                                           { false, false, false, false, false, false, false, false, false, false },  /* HeatFlowY                                 */
                                                                                                           { false, false, false, false, false, false, false, false, false, false },  /* HeatFlowZ                                 */
                                                                                                           { false, false, false, false, false, false, false, false, false, false },  /* HeatFlow_                                 */
                                                                                                           { false, false, false, false, false, false, false, false, false, false },  /* FluidVelocity                             */
                                                                                                           { false, false, false, false, false, false, false, false, false, false },  /* FluidVelocityY                            */
                                                                                                           { false, false, false, false, false, false, false, false, false, false },  /* FluidVelocityZ                            */
                                                                                                           { false, false, false, false, false, false, false, false, false, false },  /* IsoStaticMass                             */
                                                                                                           { false, false, false, false, false, false, false, false, false, false },  /* IsoStaticDeflection                       */
                                                                                                           { false, false, false, false, false, false, false, false, false, false },  /* IsoStaticWaterBottom                      */
                                                                                                           { false, false, false, false, false, false, false, false, false, false },  /* Massflux                                  */
                                                                                                           { false, false, false, false, false, false, false, false, false, false },  /* Massflux_                                 */
                                                                                                           {  true,  true,  true,  true, false,  true,  true,  true,  true, false },  /* MaxVes                                    */
                                                                                                           { false, false,  true, false,  true, false,  true,  true,  true, false },  /* Temperature                               */
                                                                                                           { false, false,  true,  true, false, false,  true,  true,  true, false },  /* Pressure                                  */
                                                                                                           { false, false,  true,  true, false, false,  true,  true,  true, false },  /* HydroStaticPressure                       */
                                                                                                           { true,   true,  true,  true, false,  true,  true,  true,  true, false },  /* LithoStaticPressure                       */
                                                                                                           { false, false,  true,  true, false, false,  true,  true,  true, false },  /* OverPressure                              */
                                                                                                           {  true,  true,  true,  true, false,  true,  true,  true,  true, false },  /* Ves                                       */
                                                                                                           { false, false, false, false, false, false, false, false, false, false },  /* Biomarkers                                */
                                                                                                           { false, false, false, false, false, false, false, false, false, false },  /* SteraneAromatisation                      */
                                                                                                           { false, false, false, false, false, false, false, false, false, false },  /* SteraneIsomerisation                      */
                                                                                                           { false, false, false, false, false, false, false, false, false, false },  /* HopaneIsomerisation                       */
                                                                                                           { false, false, false, false, false, false, false, false, false, false },  /* IlliteFraction                            */
                                                                                                           { false, false, false,  true, false, false,  true, false,  true, false },  /* AllochthonousLithology                    */
                                                                                                           { false, false,  true,  true, false, false,  true,  true,  true, false },  /* ErosionFactor                             */
                                                                                                           { false, false, false,  true, false, false,  true, false,  true, false },  /* FaultElements                             */
                                                                                                           { false, false, false,  true, false, false,  true, false,  true, false },  /* FCTCorrection                             */
                                                                                                           { false, false, false, false, false, false, false, false, false, false },  /* Thickness                                 */
                                                                                                           { false, false, false,  true, false, false,  true, false,  true, false },  /* ThicknessError                            */
                                                                                                           { false, false, false, false, false, false, false, false, false, false },  /* ChemicalCompaction                        */
                                                                                                           {  true,  true,  true,  true,  true,  true,  true,  true,  true, false },  /* Lithology                                 */
                                                                                                           { false, false,  true,  true, false, false,  true,  true,  true, false },  /* CapillaryPressureGas100                   */
                                                                                                           { false, false,  true,  true, false, false,  true,  true,  true, false },  /* CapillaryPressureGas0                     */
                                                                                                           { false, false,  true,  true, false, false,  true,  true,  true, false },  /* CapillaryPressureOil100                   */
                                                                                                           { false, false,  true,  true, false, false,  true,  true,  true, false },  /* CapillaryPressureOil0                     */
                                                                                                           { false, false, false, false, false, false, false, false, false, false },  /* genex-properties   NOT AN OUTPUT PROPERTY */
                                                                                                           { false, false, false, false, false, false, false, false, false, false },  /* asphaltines                               */
                                                                                                           { false, false, false, false, false, false, false, false, false, false },  /* resins                                    */
                                                                                                           { false, false, false, false, false, false, false, false, false, false },  /* c15+aro                                   */
                                                                                                           { false, false, false, false, false, false, false, false, false, false },  /* c15+sat                                   */
                                                                                                           { false, false, false, false, false, false, false, false, false, false },  /* c6-14Aro                                  */
                                                                                                           { false, false, false, false, false, false, false, false, false, false },  /* c6-14Sat                                  */
                                                                                                           { false, false, false, false, false, false, false, false, false, false },  /* C5                                        */
                                                                                                           { false, false, false, false, false, false, false, false, false, false },  /* C4                                        */
                                                                                                           { false, false, false, false, false, false, false, false, false, false },  /* C3                                        */
                                                                                                           { false, false, false, false, false, false, false, false, false, false },  /* C2                                        */
                                                                                                           { false, false,  true, false, false, false, false,  true, false, false },  /* C1                                        */
                                                                                                           { false, false, false, false, false, false, false, false, false, false },  /* COx                                       */
                                                                                                           { false, false, false, false, false, false, false, false, false, false },  /* N2                                        */
                                                                                                           { false, false, false, false, false, false, false, false, false, false },  /* H2S                    */
                                                                                                           { false, false, false, false, false, false, false, false, false, false },  /* LSC                    */
                                                                                                           { false, false, false, false, false, false, false, false, false, false },  /* C15+AT                 */
                                                                                                           { false, false, false, false, false, false, false, false, false, false },  /* C6-14 BT               */
                                                                                                           { false, false, false, false, false, false, false, false, false, false },  /* C6-14 DBT              */
                                                                                                           { false, false, false, false, false, false, false, false, false, false },  /* C6-14 BP               */
                                                                                                           { false, false, false, false, false, false, false, false, false, false },  /* C15+AroS               */
                                                                                                           { false, false, false, false, false, false, false, false, false, false },  /* C15+SatS               */
                                                                                                           { false, false, false, false, false, false, false, false, false, false },  /* C6-14 SatS             */
                                                                                                           { false, false, false, false, false, false, false, false, false, false },  /* C6-14 AroS             */
                                                                                                           { false, false, false, false, false, false, false, false, false, false },  /* InstantaneousExpulsionApi                 */ 
                                                                                                           { false, false, false, false, false, false, false, false, false, false },  /* CumulativeExpulsionApi                    */ 
                                                                                                           { false, false, false, false, false, false, false, false, false, false },  /* InstantaneousExpulsionCondensateGasRatio  */ 
                                                                                                           { false, false, false, false, false, false, false, false, false, false },  /* CumulativeExpulsionCondensateGasRatio     */ 
                                                                                                           { false, false, false, false, false, false, false, false, false, false },  /* InstantaneousExpulsionGasOilRatio         */ 
                                                                                                           { false, false, false, false, false, false, false, false, false, false },  /* CumulativeExpulsionGasOilRatio            */ 
                                                                                                           { false, false, false, false, false, false, false, false, false, false },  /* InstantaneousExpulsionGasWetness          */ 
                                                                                                           { false, false, false, false, false, false, false, false, false, false },  /* CumulativeExpulsionGasWetness             */ 
                                                                                                           { false, false, false, false, false, false, false, false, false, false },  /* InstantaneousExpulsionAromaticity         */ 
                                                                                                           { false, false, false, false, false, false, false, false, false, false },  /* CumulativeExpulsionAromaticity            */ 
                                                                                                           { false, false, false, false, false, false, false, false, false, false },  /* KerogenConversionRatio                    */ 
                                                                                                           { false, false, false, false, false, false, false, false, false, false },  /* OilGeneratedCumulative                    */ 
                                                                                                           { false, false, false, false, false, false, false, false, false, false },  /* OilGeneratedRate                          */ 
                                                                                                           { false, false, false, false, false, false, false, false, false, false },  /* OilExpelledCumulative                     */ 
                                                                                                           { false, false, false, false, false, false, false, false, false, false },  /* OilExpelledRate                           */ 
                                                                                                           { false, false, false, false, false, false, false, false, false, false },  /* HcGasGeneratedCumulative                  */ 
                                                                                                           { false, false, false, false, false, false, false, false, false, false },  /* HcGasGeneratedRate                        */ 
                                                                                                           { false, false, false, false, false, false, false, false, false, false },  /* HcGasExpelledCumulative                   */ 
                                                                                                           { false, false, false, false, false, false, false, false, false, false },  /* HcGasExpelledRate                         */ 
                                                                                                           { false, false, false, false, false, false, false, false, false, false },  /* DryGasGeneratedCumulative                 */ 
                                                                                                           { false, false, false, false, false, false, false, false, false, false },  /* DryGasGeneratedRate                       */ 
                                                                                                           { false, false, false, false, false, false, false, false, false, false },  /* DryGasExpelledCumulative                  */ 
                                                                                                           { false, false, false, false, false, false, false, false, false, false },  /* DryGasExpelledRate                        */ 
                                                                                                           { false, false, false, false, false, false, false, false, false, false },  /* WetGasGeneratedCumulative                 */ 
                                                                                                           { false, false, false, false, false, false, false, false, false, false },  /* WetGasGeneratedRate                       */ 
                                                                                                           { false, false, false, false, false, false, false, false, false, false },  /* WetGasExpelledCumulative                  */ 
                                                                                                           { false, false, false, false, false, false, false, false, false, false },  /* WetGasExpelledRate                        */ 
                                                                                                           { false, false, false, false, false, false, false, false, false, false },  /* Concentrations                            */ 
                                                                                                           { false, false, false, false, false, false, false, false, false, false },  /* PVT Properties: hc-density and -viscocity */ 
                                                                                                           { false, false, false, false, false, false, false, false, false, false },  /* Rel Perm calculations                     */ 
                                                                                                           { false, false, false, false, false, false, false, false, false, false },  /* Volume calculations                       */ 
                                                                                                           { false, false, false, false, false, false, false, false, false, false },  /* Transported volume calculations           */ 
                                                                                                           { false, false, false, false, false, false, false, false, false, false },  /* Saturations                               */
													   { false, false, false, false, false, false, false, false, false, false },  /* AverageSaturations                        */
                                                                                                           { false, false, false, false, false, false, false, false, false, false },  /* HC fluid velocity                         */
													   { false, false, false, false, false, false, false, false, false, false },  /* CapillaryPressure                         */
                                                                                                           { false, false, false, false, false, false, false, false, false, false },  /* Fluid Properties such as GOR, COR, OilAPI */
													   { false, false, false, false, false, false, false, false, false, false },  /* Brine properties density viscosity        */
													   { false, false, false, false, false, false, false, false, false, false },   /* Time of Invasion                          */
                                                                                                           {  true,  true,  true,  true, false,  true,  true, false },  /* ALCSmBasaltThickness   */
                                                                                                           {  true,  true,  true,  true, false,  true,  true, false },  /* ALCMaxAsthenoMantleDepth*/
                                                                                                           {  true,  true,  true,  true, false,  true,  true, false },  /* ALCStepTopBasaltDepth   */
                                                                                                           {  true,  true,  true,  true, false,  true,  true, false },  /* ALCStepMohoDepth        */
                                                                                                           {  true,  true,  true,  true, false,  true,  true, false },  /* ALCStepContCrustThickness*/
                                                                                                           {  true,  true,  true,  true, false,  true,  true, false },  /* ALCStepBasaltThickness   */
                                                                                                           {  true,  true,  true,  true, false,  true,  true, false },  /* ALCSmContCrustThickness  */
                                                                                                           {  true,  true,  true,  true, false,  true,  true, false },  /* ALCSmTopBasaltDepth      */
                                                                                                           {  true,  true,  true,  true, false,  true,  true, false },  /* ALCSmMohoDepth           */
                                                                                                           {  true,  true,  true,  true, false,  true,  true, false }   /* ALCOrigMantle            */


};
                                                                                                     

PropertyOutputConstraints::PropertyOutputConstraints () {

#if 0
   int property;
   int calcMode;

   // Perform a consistency check 
   // 
   // output of property ( not permitted and required ) is inconsistent.
   for ( property = 0; property < PropertyListSize; ++property ) {

      for ( calcMode = 0; calcMode < NumberOfCalculationModes; ++calcMode ) {

         if ( not s_outputPermitted [ property ][ calcMode ] and s_outputRequired [ property ][ calcMode ]) {
            cout << setw ( 22 ) << propertyListName ( PropertyList ( property )) << "  " << CalculationModeImage [ calcMode ] << endl;
         }

      }

   }
#endif

}


void PropertyOutputConstraints::applyOutputRegionConstraints () {

   int property;

   for ( property = 0; property < PropertyListSize; ++property ) {

      if ( s_propertyOutputRegion [ property ] == ApplicableOutputRegion::SEDIMENTS_AND_BASEMENT ) {
         m_maximumOutputOption [ property ] = Interface::SEDIMENTS_AND_BASEMENT_OUTPUT;
      } else if ( s_propertyOutputRegion [ property ] == ApplicableOutputRegion::SEDIMENTS_ONLY ) {
         m_maximumOutputOption [ property ] = Interface::SEDIMENTS_ONLY_OUTPUT;
      } else {
         m_maximumOutputOption [ property ] = Interface::SOURCE_ROCK_ONLY_OUTPUT;
      }

      m_minimumOutputOption [ property ] = Interface::SEDIMENTS_ONLY_OUTPUT;
      m_minimumOutputOption [ property ] = Interface::NO_OUTPUT;
   }

}


void PropertyOutputConstraints::applyCalculationModeConstraints ( const CalculationMode calculationMode ) {

   int property;

   for ( property = 0; property < PropertyListSize; ++property ) {

      if ( m_maximumOutputOption [ property ] > s_calculationModeMaxima [ calculationMode ]) {
         m_maximumOutputOption [ property ] = s_calculationModeMaxima [ calculationMode ];
      }

      if ( m_minimumOutputOption [ property ] > s_calculationModeMaxima [ calculationMode ]) {
         m_minimumOutputOption [ property ] = s_calculationModeMaxima [ calculationMode ];
      }

   }


}


void PropertyOutputConstraints::applyOutputPermittedConstraints ( const CalculationMode calculationMode ) {

   int property;

   for ( property = 0; property < PropertyListSize; ++property ) {

      if ( not s_outputPermitted [ property ][ calculationMode ]) {
         m_maximumOutputOption [ property ] = Interface::NO_OUTPUT;
         m_minimumOutputOption [ property ] = Interface::NO_OUTPUT;
      }

      if ( property == OVERPRESSURE ) {
         int var;
         var = 1;
      }

      if ( s_outputRequired [ property ][ calculationMode ]) {

         if ( s_propertyOutputRegion [ property ] != ApplicableOutputRegion::SOURCE_ROCK_ONLY and m_maximumOutputOption [ property ] < Interface::SEDIMENTS_ONLY_OUTPUT ) {
            m_maximumOutputOption [ property ] = Interface::SEDIMENTS_ONLY_OUTPUT;
         }

         if ( s_propertyOutputRegion [ property ] != ApplicableOutputRegion::SOURCE_ROCK_ONLY and m_minimumOutputOption [ property ] < Interface::SEDIMENTS_ONLY_OUTPUT ) {
            m_minimumOutputOption [ property ] = Interface::SEDIMENTS_ONLY_OUTPUT;
         }

      }

   }

}

void PropertyOutputConstraints::ensureConsistency () {

   int property;

   for ( property = 0; property < PropertyListSize; ++property ) {

      if ( m_minimumOutputOption [ property ] > m_maximumOutputOption [ property ]) {
         m_minimumOutputOption [ property ] = m_maximumOutputOption [ property ];
      }

   }

}


void PropertyOutputConstraints::initialise ( const CalculationMode          calculationMode,
                                             const Interface::ModellingMode modellingMode ) {

   applyOutputRegionConstraints ();
   applyCalculationModeConstraints ( calculationMode );
   applyOutputPermittedConstraints ( calculationMode );
   ensureConsistency ();
}

void PropertyOutputConstraints::print ( std::ostream& o ) const {

   int property;

   o << " Maximum options: " << std::endl;

   for ( property = 0; property < PropertyListSize; ++property ) {
      o << "Property: " 
        << std::setw ( 30 ) << propertyListName ( PropertyList ( property )) << "  " 
        << std::setw ( 20 ) << PropertyOutputOptionImage ( m_maximumOutputOption [ property ]) 
        << std::endl;
   }

   o << std::endl << std::endl;
   o << " Minimum options: " << std::endl;

   for ( property = 0; property < PropertyListSize; ++property ) {
      o << "Property: " 
        << std::setw ( 30 ) << propertyListName ( PropertyList ( property )) << "  " 
        << std::setw ( 20 ) << PropertyOutputOptionImage ( m_minimumOutputOption [ property ]) 
        << std::endl;
   }


   o << std::endl << std::endl;
   o << " Min-Maximum options: " << std::endl;

   for ( property = 0; property < PropertyListSize; ++property ) {

      if ( m_maximumOutputOption [ property ] > Interface::NO_OUTPUT or m_minimumOutputOption [ property ] > Interface::NO_OUTPUT ) {
         o << "Property: " 
           << std::setw ( 30 ) << propertyListName ( PropertyList ( property )) << "  " 
           << std::setw ( 20 ) << PropertyOutputOptionImage ( m_minimumOutputOption [ property ]) << "  "
           << std::setw ( 20 ) << PropertyOutputOptionImage ( m_maximumOutputOption [ property ]) << std::endl;
      }

   }

   o << std::endl << std::endl;

   for ( property = 0; property < PropertyListSize; ++property ) {

      if ( m_maximumOutputOption [ property ] == Interface::NO_OUTPUT and m_minimumOutputOption [ property ] == Interface::NO_OUTPUT ) {
         o << "Property: "
           << std::setw ( 30 ) << propertyListName ( PropertyList ( property )) << "  " 
           << std::setw ( 20 ) << PropertyOutputOptionImage ( m_minimumOutputOption [ property ]) << "  "
           << std::setw ( 20 ) << PropertyOutputOptionImage ( m_maximumOutputOption [ property ]) << std::endl;
      }

   }

}

void PropertyOutputConstraints::constrain ( const PropertyList                     property,
                                                  Interface::PropertyOutputOption& option  ) const {

   // if ( property < DIFFUSIVITYVEC or property > LITHOLOGY ) {
   //    // cannot constrain a property that is not known to fastcauldron.
   //    return;
   // }

   if ( option > m_maximumOutputOption [ property ]) {
      option = m_maximumOutputOption [ property ];
   }

   if ( option < m_minimumOutputOption [ property ]) {
      option = m_minimumOutputOption [ property ];
   }

}
