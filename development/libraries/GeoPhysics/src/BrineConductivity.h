//
// Copyright (C) 2014 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _GEOPHYSICS_BRINE_CONDUCTIVITY_H_
#define _GEOPHYSICS_BRINE_CONDUCTIVITY_H_

#include <vector>
#include "Interface/FluidType.h"
#include "Interface/Interface.h"

#include "database.h"
#include "PiecewiseInterpolator.h"
#include "IBSinterpolator2d.h"

#include "BrinePhases.h"

namespace GeoPhysics
{
   /// \brief BrineConductivity is intended to handle the calculations of thermal conductivity for brines whose physical parameters (T,P,S) are
   ///       within the allowed ranges. It makes extensive use of the 2D table (in terms of T,P; salinity is not important) from Sengers et al.
   ///       (1984) and uses bi-linear interpolation to retrieve thermal conductivity values in both the aqueous and the vapour phase.

   class BrineConductivity: public BrinePhases
   {
   public:
      /// Constructor. Initializes m_tempArray, m_presArray and m_thCondArray.
      /// \pre None.
      /// \post Guarantees initialization.
      BrineConductivity();
      /// Virtual destructor
      virtual ~BrineConductivity() {}

   protected:
      /// Uses bi-linear interpolation to return the value of thermal conductivity using the table provided in Sengers et al. (1984)
      /// \pre Requires the values of T,P to be within the allowed ranges.
      /// \post Guarantees the return of non-negative value for the thermal conductivity.
      double aqueousTable ( const double temperature, const double pressure ) const;

      /// Uses bi-linear interpolation to return the value of thermal conductivity using the table provided in Sengers et al. (1984)
      /// \pre Requires the values of T,P to be within the allowed ranges (see BrinePhases.C).
      /// \post Guarantees the return of non-negative value for the thermal conductivity.
      double vapourTable ( const double temperature, const double pressure ) const;

      /// Linearly interpolates between the values at the two sides of the transition region and returns the value.
      /// \pre Requires the passed arguments to be within the allowed ranges (see BrinePhases.C) and lowerTemperature < higherTemperature.
      /// \post Guarantees the return of a non-negative value for the velocity.
      double transitionRegion ( const double temperature, const double pressure,
                                const double higherTemperature, const double lowerTemperature ) const;

   private:
      /// Size and elements of the 2D table from Sengers et al. Thermal conductivity
      /// values are actually implemented as a 1D array to facilitate interpolation.
      static const int s_tempArraySize = 23, s_presArraySize = 29, s_thCondArraySize = 667;
      static const double s_tempArray[s_tempArraySize], s_presArray[s_presArraySize], s_thCondArray[s_thCondArraySize];

      /// The 2D thermal-conductivity table will be handled using these arrays.
      const std::vector<double> m_tempArray;
      const std::vector<double> m_presArray;
      const std::vector<double> m_thCondArray;

      /// Table does not cover the whole range of allowed T,P. Confining the possible values to those
      /// defined by the table (PressureMaxForConductivity and TemperatureMaxForConductivity) is equivalent
      /// to using 'constant' extrapolation for thermal conductivity at high pressures and temperatures.
      static const double PressureMaxForConductivity;
      static const double TemperatureMaxForConductivity;
    
      /// Depending on the ordering of temperature, higherTemperature and lowerTemperature calls the appropriate function to calculate
      /// the value of the brine parameter of interest. It then returns the value returned by that function without further checks.
      /// \pre Requires the triplet of T,P,S to be within the allowed ranges and lowerTemperature < higherTemperature.
      /// \post Guarantees the return of the return value of the appropriate function to be called depending on temperature, higherTemperature and lowerTemperature.
      virtual double chooseRegion ( const double temperature, const double pressure, const double salinity,
                                    const double higherTemperature, const double lowerTemperature ) const;

      /// Bi-linear interpolator used in both the aqueous and the vapour phase of the brines.
      /// \pre Requires that both T and P are within the range of the values defined in the Sengers et al. table (see BrineConductivity.C).
      /// \post Guarantees the return of a non-negative thermal-conductivity value.
      double interpolate2d ( const double temperature, const double pressure ) const;

   };

} /// end GeoPhysics

#endif /// GEOPHYSICS_BRINE_CONDUCTIVITY_H_
