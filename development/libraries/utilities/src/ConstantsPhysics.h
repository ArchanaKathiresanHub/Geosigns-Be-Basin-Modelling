//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file ConstantsPhysics.h
/// @brief This file stores the physical constants used by all IBS applications and libraries

#ifndef UTILITIES_CONSTANTSPHYSICS_H
#define UTILITIES_CONSTANTSPHYSICS_H

#include "ConstantsMathematics.h"

namespace Utilities
{
   namespace Physics {

      /// @defgroup Force
      /// @{
      /// \brief Acceleration due to gravity [M.S^-2]
      constexpr double AccelerationDueToGravity        = 9.81;
      /// \brief Acceleration due to gravity used in genex application and libraries [M.S^-2]
      /// \details This values as been used to calibrate all others genex constants and equations
      constexpr double AccelerationDueToGravityoGenex  = 9.807;
      /// @}

      /// @defgroup Gas
      /// @{
      /// \brief The ideal gas constant [J.K^-1.mol^-1]
      constexpr double IdealGasConstant       = 8.314472; 
      /// \brief The ideal gas constant used in genex libraries and applications [J.K^-1.mol^-1]
      /// \details This values as been used to calibrate all others genex constants and equations
      constexpr double IdealGasConstantGenex  = 8.314511;
      /// \brief Radius of gas molecules {C1,C2,C3,C4,C5} [M]
      /// \details Molecular volumes and the Stokes-Einstein equation, 1970, Journal of Chemical Education 47, no. 4: 261
      constexpr double GasRadius[5] = { 1.89e-10, 2.21e-10, 2.46e-10, 2.66e-10, 2.84e-10 };
      /// @}

      /// @defgroup Energy
      /// @{
      /// \brief The Blotzmaan constant [J.K^-1]
      constexpr double BoltzmannConstant = 1.38064852e-23;
      /// \brief The Blotzmaan constant used in genex application and libraries [J.K^-1]
      /// \details This values as been used to calibrate all others genex constants and equations
      constexpr double BoltzmannConstantGenex = 1.3806503e-23;
      /// \brief The Blotzmaan constant divided by the Plank constant and multiplied by the number of seconds in
      ///    one million year
      constexpr double BoltzmannOverPlanckByMillionYear = 6.571073735576960e+23;
      /// @}


      /// @defgroup Pressure
      /// @{
      constexpr double StandardPressure                   = 101325.353;        ///< The detailed standard pressure condition    (at sea level) [Pa]
      constexpr double PressureAtSeaLevel                 = 1.0e5;             ///< The aproximated standard pressure condition (at sea level) [Pa]
      constexpr double StockTankPressureMPa               = 101325.0 * 1.0e-6; ///< The detailed standard pressure condition    (at sea level) [MPa]
      constexpr double AtmosphericPressureMpa             = 0.1;               ///< The aproximated standard pressure condition (at sea level) [MPa]
      constexpr double DefaultHydrostaticPressureGradient = 10.0;              ///< Assumed pressure gradient used when computing the simple fluid density [MPa.Km^-1]
      /// @}

      /// @defgroup Density
      /// @{
      /// \brief Density of fresh water [Kg.m-3]
      constexpr double StandardWaterDensity = 1000.0;
      /// @}

      /// @defgroup Temperature
      /// @{
      /// \brief The surface conditions termperature used in Genex [K]
      /// \details This values as been used to calibrate all others genex constants and equations
      constexpr double StandardTemperatureGenexK  = 15.5555556 + Utilities::Maths::CelciusToKelvin;
      /// \brief The surface conditions termperature [C]
      constexpr double StockTankTemperatureC = 15.0;
      /// @}

   }
}
#endif
