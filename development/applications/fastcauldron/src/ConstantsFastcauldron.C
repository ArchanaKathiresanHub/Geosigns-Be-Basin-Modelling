//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 
#include "ConstantsFastcauldron.h"

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


const std::string& getBoundaryConditionsImage ( const Boundary_Conditions bcs ) {

   static std::string s_bcNames [] = { "Interior_Boundary",
                                       "Interior_Constrained_Overpressure",
                                       "Surface_Boundary",
                                       "Bottom_Boundary",
                                       "Bottom_Boundary_Flux",
                                       "Interior_Neumann_Boundary",
                                       "Side_Neumann_Boundary",
                                       "Bottom_Neumann_Boundary",
                                       "Interior_Constrained_Temperature",
                                       "UNKNOWN "};

   if ( Interior_Boundary <= bcs and bcs <= Interior_Constrained_Temperature ) {
      return s_bcNames [ static_cast<int>(bcs)];
   } else {
      return s_bcNames [ 9 ];
   }

}
