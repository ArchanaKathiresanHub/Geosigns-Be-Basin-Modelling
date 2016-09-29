//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#ifndef _GEOPHYSICS__GEOPHYSICAL_CONSTANTS_H_
#define _GEOPHYSICS__GEOPHYSICAL_CONSTANTS_H_

// std library
#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif
#include <math.h>
#include <string>
#include <cmath>

namespace GeoPhysics {


   //------------------------------------------------------------//
   // Mathematical constants

   /// \var MILLIDARCYTOM2
   /// mDarcy to meter square conversion.
   const double MILLIDARCYTOM2 = 0.9869233E-15;
   const double M2TOMILLIDARCY = (1 / MILLIDARCYTOM2);

   /// \var Pa_To_MPa
   /// Conversion factor for Pascals to Mega Pascals.
   const double Pa_To_MPa = 0.000001;

   /// \var MPa_To_Pa
   /// Conversion factor for Mega Pascals to Pascals.
   const double MPa_To_Pa = 1000000.0;

   /// \var MicroWattsToWatts
   /// Number of Watts in a microWatt.
   const double MicroWattsToWatts = 0.000001;

   /// \var MilliWattsToWatts
   /// Number of Watts in a milliWatt.
   const double MilliWattsToWatts = 0.001;

   /// \var SecondsPerYear
   /// Number of seconds in a year.
   ///
   /// Under the assumption that there are 365.25 days per year.
   const double SecondsPerYear = 3.15576e7;

   /// \var SecondsPerMillionYears
   /// Number of seconds in a million years.
   const double SecondsPerMillionYears = 1.0e6 * SecondsPerYear;

   /// \var Secs_IN_MA
   /// Number of Seconds in a Million Years.
   const double Secs_IN_MA = 3.15576E13;

   /// \var CelciusToKelvin
   /// \brief Value used in temperature conversion from Celcius to Kelvin.
   const double CelciusToKelvin = 273.15;

   /// \var PascalsToMegaPascals
   /// Conversion factor from pascals to mega-pascals.
   const double PascalsToMegaPascals = 1.0e-6;

   //------------------------------------------------------------//
   // Geological constants

   /// \var AtmosphericPressure
   /// Pressure at sea-level, in MPa.
   const double AtmosphericPressure = 0.1;

   /// \var NoFracturePressureValue
   /// The value to return for the fracture pressure if "None" has been selected.
   /// 100,000 MPa, the pore-pressure will never attain this value.
   const double NoFracturePressureValue = 1.0e5; 

   /// \var AccelerationDueToGravity
   /// Acceleration due to gravity in m/s^2, more accurate value: 9.80665 m /s^2.
   const double AccelerationDueToGravity = 9.81;

   /// \var StandardWaterDensity
   /// Density of fresh water, in Kg/m^3
   const double StandardWaterDensity = 1000.0;

   /// \var MinimumSoilMechanicsPorosity
   /// The smallest value that can be obtained when using the soil-mechanics porosity-mode.
   const double MinimumSoilMechanicsPorosity = 0.03;

   /// \var MinimumPorosity
   /// The smallest value that can be obtained for the porosity when using chemical compaction
   const double MinimumPorosity = 0.03;

   /// \var RockViscosityReferenceTemperature
   /// The temperature used in the reference rock viscosity, units Celsius.
   const double RockViscosityReferenceTemperature = 15.0;

   /// \var MolarMassQuartz
   /// The molar mass of quartz, units is g . mol^-1.
   const double MolarMassQuartz = 60.09;

   /// \var DensityQuartz
   /// The density of quartz, units is g . cm^-3.
   const double DensityQuartz = 2.65;

   /// \var GasConstant
   /// Units are J . K^-1 . mol^-1.
   const double GasConstant = 8.314472;

   /// \var Ves0
   /// The Vertical Effective Stress at surface conditions, units are Pa
   const double Ves0 = 1.0E+05;

   /// \var AgeOfEarth
   /// Approximate age of the Earth in million years.
   const double AgeOfEarth = 4500.0;

   //------------------------------------------------------------//
   // Numerical constants

   /// \var PercentagePorosityReboundForSoilMechanics
   /// \brief The percentage of porosity that can be increased on uplift.
   /// only for soil-mechanice lithologies.
   const double PercentagePorosityReboundForSoilMechanics = 0.02; // => %age porosity regain

   /// \var MaximumNumberOfLithologies
   /// The maximum number of simple lithologies that can be set in a compound lithology.
   const int MaximumNumberOfLithologies = 3;

   /// \var ThicknessTolerance
   /// The tolerance for comparing thickness of layers.
   const double ThicknessTolerance = 0.1;

   /// \var MobileLayerNegativeThicknessTolerance
   /// \brief The cut-off tolerance value for negative thicknesses of mobile layers.
   ///
   /// if the thickness of a mobile layer < -MobileLayerNegativeThicknessTolerance then this is an error
   /// if the thickness is in the range [-MobileLayerNegativeThicknessTolerance, 0] then the thickness is set to zero.
   const double MobileLayerNegativeThicknessTolerance = 0.5;

   /// \var MaximumReasonableCrustThinningRatio
   /// \brief A reasonable maximum ratio of the crust thickness differences.
   ///
   /// There is no basis for choosing the value of this number, other than the number of elements
   /// that will be used in the mantle. Numbers smaller than this would also be possible.
   /// For this reason if the computed thinning-ratio is greater than the value here, it
   /// should result in a warning and not an error condtion.
   const double MaximumReasonableCrustThinningRatio = 10.0;

   /// \var GardnerVelocityConstant
   /// Constant used in computation of the Gardner velocity-density velocity algorithm.
   const double GardnerVelocityConstant = 309.4;

   /// \var maxPermeability
   /// The maximum permeability, units are darcy
   const double MaxPermeability = 1000.0;

   //------------------------------------------------------------//
   // Naming constants

   /// \var OneDHiatusLithologyName
   /// Name of the lithology that is to be used in a formation that has not lithologies defined.
   ///
   /// To be used in one-dimension only. The actual value used here is irrelevant, as
   /// long as the simple-lithology exists, since the lithology itself is not used.
   const std::string OneDHiatusLithologyName = "Std. Sandstone";

   /// \var FaultDefaultSealingLithologyName
   /// Value stored in project file indicating the fault lithology type.a
   const std::string FaultDefaultSealingLithologyName = "Default: Sealing litho.";

   /// \var FaultDefaultPassingLithologyName
   /// Value stored in project file indicating the fault lithology type.a
   const std::string FaultDefaultPassingLithologyName = "Default: Passing litho.";

   //------------------------------------------------------------//
   // Mathematical constants

   const double Log10 = std::log( 10.0 );
   const double Pi2    = pow( M_PI, 2 );
   const double Pi2by8 = Pi2 / 8;

}

#endif // _GEOPHYSICS__GEOPHYSICAL_CONSTANTS_H_
