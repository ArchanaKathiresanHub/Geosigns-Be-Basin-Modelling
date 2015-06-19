#include "globaldefs.h"

const std::string& RelativePermeabilityTypeImage ( const RelativePermeabilityType type ) {

   static std::string relPermNames [ 4 ] = { "NO_RELATIVE_PERMEABILITY_FUNCTION"
                                             "TEMIS_PACK_RELATIVE_PERMEABILITY_FUNCTION"
                                             "ANNETTE_RELATIVE_PERMEABILITY_FUNCTION"
                                             "UNKNOWN_RELATIVE_PERMEABILITY_FUNCTION" };

   if ( NO_RELATIVE_PERMEABILITY_FUNCTION <= type and type <= ANNETTE_RELATIVE_PERMEABILITY_FUNCTION ) {
      return relPermNames [ type ];
   } else {
      return relPermNames [ UNKNOWN_RELATIVE_PERMEABILITY_FUNCTION ];
   }

}

RelativePermeabilityType RelativePermeabilityTypeValue ( const std::string& type ) {

   RelativePermeabilityType result;

   if ( type == "none" or type == "NONE" or type == "NO_RELATIVE_PERMEABILITY_FUNCTION" ) {
      result = NO_RELATIVE_PERMEABILITY_FUNCTION;
   } else if ( type == "temis" or type == "Temis" or type == "TEMIS" or type == "TEMIS_PACK_RELATIVE_PERMEABILITY_FUNCTION" ) {
      result = TEMIS_PACK_RELATIVE_PERMEABILITY_FUNCTION;
   } else if ( type == "annette" or type == "ANNETTE" or type == "Annette" or type == "ANNETTE_RELATIVE_PERMEABILITY_FUNCTION" ) {
      result = ANNETTE_RELATIVE_PERMEABILITY_FUNCTION;
   } else {
      result = UNKNOWN_RELATIVE_PERMEABILITY_FUNCTION;
   }

   return result;
}

CalculationMode getSimulationMode ( const std::string& modeStr ) {

   for ( int i = 0; i < NO_CALCULATION_MODE; ++i ) {
      CalculationMode mode = static_cast<CalculationMode>( i );

      if ( getSimulationModeString ( mode ) == modeStr ) {
         return mode;
      }

   }

   return NO_CALCULATION_MODE;
}


const std::string& getSimulationModeString ( const CalculationMode mode ) {

   static const std::string s_simulationModeStr [ NumberOfCalculationModes ] = { "HydrostaticDecompaction",
                                                                                 "HydrostaticHighResDecompaction",
                                                                                 "HydrostaticTemperature",
                                                                                 "Overpressure",
                                                                                 "LooselyCoupledTemperature",
                                                                                 "CoupledHighResDecompaction",
                                                                                 "CoupledPressureAndTemperature",
                                                                                 "HydrostaticDarcy",
                                                                                 "CoupledDarcy",
                                                                                 "NoCalculaction" };

   if ( mode >= HYDROSTATIC_DECOMPACTION_MODE and mode < NO_CALCULATION_MODE ) {
      return s_simulationModeStr [ mode ];
   } else {
      return s_simulationModeStr [ NO_CALCULATION_MODE ];
   }

}
