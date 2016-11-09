//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#ifndef GEOPHYSICS__GEOPHYSICAL_CONSTANTS_H
#define GEOPHYSICS__GEOPHYSICAL_CONSTANTS_H

// std library
#include <string>
#include <cmath>

namespace GeoPhysics {


   //------------------------------------------------------------//
   // Geological constants
   
   /// \var NoFracturePressureValue
   /// The value to return for the fracture pressure if "None" has been selected.
   /// 100,000 MPa, the pore-pressure will never attain this value.
   constexpr double NoFracturePressureValue = 1.0e5;

   /// \var MinimumSoilMechanicsPorosity
   /// The smallest value that can be obtained when using the soil-mechanics porosity-mode.
   constexpr double MinimumSoilMechanicsPorosity = 0.03;

   /// \var MinimumPorosity
   /// The smallest value that can be obtained for the porosity when using chemical compaction - legacy value
   /// Will be modified when new rock property library feature is unflagged and released
   constexpr double MinimumPorosity = 0.03;

   /// \var MinimumPorosityNonLegacy
   /// The smallest value that can be obtained for the porosity when using chemical compaction - new value
   /// Part of the new rock property library feature which is flagged for now
   constexpr double MinimumPorosityNonLegacy = 0.001;

   /// \var RockViscosityReferenceTemperature
   /// The temperature used in the reference rock viscosity, units Celsius.
   constexpr double RockViscosityReferenceTemperature = 15.0;

   /// \var MolarMassQuartz
   /// The molar mass of quartz, units is g . mol^-1.
   constexpr double MolarMassQuartz = 60.09;

   /// \var DensityQuartz
   /// The density of quartz, units is g . cm^-3.
   constexpr double DensityQuartz = 2.65;

   /// \var Ves0
   /// The Vertical Effective Stress at surface conditions, units are Pa
   constexpr double Ves0 = 1.0E+05;


   //------------------------------------------------------------//
   // Numerical constants

   /// \var PercentagePorosityReboundForSoilMechanics
   /// \brief The percentage of porosity that can be increased on uplift.
   /// only for soil-mechanice lithologies.
   constexpr double PercentagePorosityReboundForSoilMechanics = 0.02; // => %age porosity regain

   /// \var MaximumNumberOfLithologies
   /// The maximum number of simple lithologies that can be set in a compound lithology.
   constexpr int MaximumNumberOfLithologies = 3;

   /// \var ThicknessTolerance
   /// The tolerance for comparing thickness of layers.
   constexpr double ThicknessTolerance = 0.1;

   /// \var MobileLayerNegativeThicknessTolerance
   /// \brief The cut-off tolerance value for negative thicknesses of mobile layers.
   ///
   /// if the thickness of a mobile layer < -MobileLayerNegativeThicknessTolerance then this is an error
   /// if the thickness is in the range [-MobileLayerNegativeThicknessTolerance, 0] then the thickness is set to zero.
   constexpr double MobileLayerNegativeThicknessTolerance = 0.5;

   /// \var MaximumReasonableCrustThinningRatio
   /// \brief A reasonable maximum ratio of the crust thickness differences.
   ///
   /// There is no basis for choosing the value of this number, other than the number of elements
   /// that will be used in the mantle. Numbers smaller than this would also be possible.
   /// For this reason if the computed thinning-ratio is greater than the value here, it
   /// should result in a warning and not an error condtion.
   constexpr double MaximumReasonableCrustThinningRatio = 10.0;

   /// \var GardnerVelocityConstant
   /// Constant used in computation of the Gardner velocity-density velocity algorithm.
   constexpr double GardnerVelocityConstant = 309.4;

   /// \var MaxPermeability
   /// The maximum permeability, units are darcy
   constexpr double MaxPermeability = 1000.0;

   /// \var LogmaxPermeability
   /// The natural log of maximum permeability.
   const double LogMaxPermeability = std::log ( MaxPermeability );

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
}

#endif // GEOPHYSICS__GEOPHYSICAL_CONSTANTS_H
