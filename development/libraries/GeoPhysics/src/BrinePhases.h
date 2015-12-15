//
// Copyright (C) 2014 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _GEOPHYSICS_BRINE_PHASES_H_
#define _GEOPHYSICS_BRINE_PHASES_H_

#include <cmath>
#include <vector>


namespace GeoPhysics
{
   /// \brief BrinePhases serves as the base class for all brine properties that use the same approach of dividing
   ///        the parameter space in three regions, aqueous, vapour and an artificially extended transition region.

   class BrinePhases
   {
   public:
      /// Allowed ranges for temperature, pressure and salinity.
      static const double s_MinTemperature;
      static const double s_MaxTemperature;

      static const double s_MinPressure;
      static const double s_MaxPressure;

      static const double s_MinSalinity;
      static const double s_MaxSalinity;

      /// Constructor that initializes m_pres vector and assigns values from TabulatedBrineP[] to its elements.
      /// \pre None.
      /// \post Guarantees initialization.
      BrinePhases();
      /// Virtual destructor
      virtual ~BrinePhases(){}
      
      /// phaseChange() returns the value of a given physical parameter for an input triplet of T, P, S (temperature, pressure, salinity) values.
      /// \pre Input values should be within the defined ranges (see BrinePhases.C). In case they are not they will be internally adjusted to the relevant limits.
      /// \post Guarantees the return of a non-negative number.
      double phaseChange( const double temperature, const double pressure, const double salinity ) const;

   protected:
      /// Uses linear 1D interpolation to return the value of the temperature at the high end of the artificially broadened transition region
      /// using pressure as an argument.
      /// \pre Input value of pressure should be within the allowed pressure range, otherwise the algorithm will effectively extrapolate.
      /// \post Guarantees the return of a temperature value within the range defined by the m_temps[] array.
      double findT2( const double pressure ) const;

      /// Subtracts ( 2 x s_halfwidth ) from the input temperature to return the temperature value at the low end of the extended transition region.
      /// \preNone.
      /// \post Guarantees the return of a value which is smaller by ( 2 x s_halfwidth ) compared to the input value.
      double findT1( const double higherTemperature ) const;

      /// Checks whether all three of the defining physical parameters (T,P,S) are within the allowed ranges and if not adjusts them
      /// \pre Requires any valid values for the triplet passed by value.
      /// \post Guarantees assignment of valid and within the allowed ranges values to the triplet that is passed by reference.
      void enforceRanges (const double temperature, const double pressure, const double salinity, double & temp, double & pres, double & sal) const
      {
         temp = temperature > s_MaxTemperature ? s_MaxTemperature : ( temperature < s_MinTemperature ? s_MinTemperature : temperature );
         pres = pressure    > s_MaxPressure    ? s_MaxPressure    : ( pressure    < s_MinPressure    ? s_MinPressure    : pressure    );
         sal  = salinity    > s_MaxSalinity    ? s_MaxSalinity    : ( salinity         < s_MinSalinity    ? s_MinSalinity    : salinity    );
      }

   private:
      /// 11 values of pressure have tabulated transition temperatures.
      static const int s_TabulatedTransitions = 11;

      /// Half of the width of the transition region.
      static const double s_halfWidth;

      /// Values of pressure and the corresponding transition temperature (middle of the transition region)
      static const double s_TabulatedBrineP[s_TabulatedTransitions];
      static const double s_TabulatedBrineT[s_TabulatedTransitions];

      /// The tabulated pressure values will be handled using this vector.
      const std::vector<double> m_pres;

      /// The upper temperatures of the transitions, at different pressures.
      double m_temps[s_TabulatedTransitions];

      /// Useful quantities for the interpolation function in the transition region.
      double interpolRatio[s_TabulatedTransitions-1];
      double interpolTerm[s_TabulatedTransitions-1];

      /// Depending on the ordering of temperature, higherTemperature and lowerTemperature calls the appropriate function to calculate
      /// the value of the brine parameter of interest. It then returns the value returned by that function without further checks.
      /// \pre Requires the triplet of T,P,S to be within the allowed ranges and lowerTemperature < higherTemperature.
      /// \post Guarantees the return of the return value of the appropriate function to be called depending on temperature, higherTemperature and lowerTemperature.
      virtual double chooseRegion ( const double temperature, const double pressure, const double salinity,
                                    const double higherTemperature, const double lowerTemperature ) const = 0;
   };

} /// end GeoPhysics

#endif /// GEOPHYSICS_BRINE_PHASES_H_
