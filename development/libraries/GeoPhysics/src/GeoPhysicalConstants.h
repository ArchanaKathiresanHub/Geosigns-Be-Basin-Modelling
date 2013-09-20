#ifndef _GEOPHYSICS__GEOPHYSICAL_CONSTANTS_H_
#define _GEOPHYSICS__GEOPHYSICAL_CONSTANTS_H_

#include <string>

namespace GeoPhysics {

   //----------------------------//
   // Conversion factors    
   //----------------------------//

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

   //----------------------------//

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

   /// \var PascalsToMegaPascals
   /// Conversion factor from pascals to mega-pascals.
   const double PascalsToMegaPascals = 1.0e-6;

   /// \var MinimumSoilMechanicsPorosity
   /// The smallest value that can be obtained when using the soil-mechanics porosity-mode.
   const double MinimumSoilMechanicsPorosity = 0.03;

   /// \var MinimumPorosity
   /// The smallest value that can be obtained for the porosity when using chemical compaction
   const double MinimumPorosity = 0.03;

   /// \var RockViscosityReferenceTemperature
   /// The temperature used in the reference rock viscosity, units Celsius.
   const double RockViscosityReferenceTemperature = 15.0; 

   /// \var GasConstant
   /// Units are J . K^-1 . mol^-1.
   const double GasConstant = 8.314472;

   //----------------------------//

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

   /// \var AgeOfEarth
   /// Approximate age of the Earth in million years.
   const double AgeOfEarth = 4500.0;

   //----------------------------//

   /// \var GardnerVelocityConstant
   /// Constant used in computation of the Gardner velocity-density velocity algorithm.
   const double GardnerVelocityConstant = 309.4;

   /// \var GardnersVelocityDensityAlgorithm
   /// Gardners velocity-density velocity algorithm.
   const std::string GardnersVelocityDensityAlgorithm = "Gardner\'s Velocity-Density";

   /// \var WylliesTimeAverageAlgorithm
   /// Wyllie's Time-Average velocity algorithm.
   const std::string WylliesTimeAverageAlgorithm = "Wyllie\'s Time-Average";

   /// \var DefaultVelocityAlgorithm
   /// The default velocity-algorithm to be used if none is specified in the project file.
   const std::string DefaultVelocityAlgorithm = GardnersVelocityDensityAlgorithm;

   /// Indicates which velocity algorithm should be used.
   enum VelocityAlgorithm { GARDNERS_VELOCITY_ALGORITHM, WYLLIES_VELOCITY_ALGORITHM };

   const std::string& velocityAlgorithmImage ( const VelocityAlgorithm vel );

   VelocityAlgorithm velocityAlgorithmValue ( const std::string& algorithmName );

   //------------------------------------------------------------//

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

//------------------------------------------------------------//
// Inline functions.
//------------------------------------------------------------//

inline const std::string& GeoPhysics::velocityAlgorithmImage ( const VelocityAlgorithm vel ) {

   static const std::string UnknownAlgorithmName = "UnknownAlgorithm";

   if ( vel == GARDNERS_VELOCITY_ALGORITHM ) {
      return GardnersVelocityDensityAlgorithm;
   } else if ( vel == WYLLIES_VELOCITY_ALGORITHM ) {
      return WylliesTimeAverageAlgorithm;
   } else {
      return UnknownAlgorithmName;
   }

}

inline GeoPhysics::VelocityAlgorithm GeoPhysics::velocityAlgorithmValue ( const std::string& algorithmName ) {

   if ( algorithmName == GardnersVelocityDensityAlgorithm ) {
      return GARDNERS_VELOCITY_ALGORITHM;
   } else if ( algorithmName == WylliesTimeAverageAlgorithm ) {
      return WYLLIES_VELOCITY_ALGORITHM;
   } else {
      return GARDNERS_VELOCITY_ALGORITHM;
   }

}



#endif // _GEOPHYSICS__GEOPHYSICAL_CONSTANTS_H_
