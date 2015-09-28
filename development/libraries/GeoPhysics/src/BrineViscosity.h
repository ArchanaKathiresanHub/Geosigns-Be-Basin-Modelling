//
// Copyright (C) 2014 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _GEOPHYSICS_BRINE_VISCOSITY_H_
#define _GEOPHYSICS_BRINE_VISCOSITY_H_

#include "BrinePhases.h"

namespace GeoPhysics
{
   /// \brief BrineViscosity is intended to handle the calculations of viscosity for brines whose physical parameters (T,P,S) are
   ///        within the allowed ranges (see BrinePhases.C). It uses Batzle-Wang analytic equation in the aqueous phase and a constant
   ///        value in the vapour phase. In the transition region bi-linear interpolation between the two approaches is used.

   class BrineViscosity: public BrinePhases
   {
   public:
      /// Constructor.
      BrineViscosity() {}
      /// Virtual destructor.
      virtual ~BrineViscosity() {}
   protected:
      /// Analytic function implementing the Batzle-Wang equation for the value of density
      /// and applying it only in the aqueous phase of the brine.
      /// \pre Requires the passed arguments to be within the allowed ranges.
      /// \post Guarantees the return of a non-negative value for the density.
      double aqueousBatzleWang ( const double temperature, const double salinity ) const;

      /// Returns a constant value for brines of any combination of parameters as long as they are in the vapour phase.
      /// \pre None.
      /// \post Guarantees the return of a constant value.
      double vapourConstant () const;

      /// Linearly interpolates between the values at the two sides of the transition region and returns the value.
      /// \pre Requires the passed arguments to be within the allowed ranges (see BrinePhases.C) and lowerTemperature < higherTemperature.
      /// \post Guarantees the return of a non-negative value for the density.
      double transitionRegion ( const double temperature, const double pressure, const double salinity,
                                const double higherTemperature, const double lowerTemperature ) const;

   private:
      /// The constant returned by transitionRegion().
      static const double VapourViscosity;

      /// Depending on the ordering of temperature, higherTemperature and lowerTemperature calls the appropriate function to calculate
      /// the value of the brine parameter of interest. It then returns the value returned by that function without further checks.
      /// \pre Requires the triplet of T,P,S to be within the allowed ranges and lowerTemperature < higherTemperature.
      /// \post Guarantees the return of the return value of the appropriate function to be called depending on temperature, higherTemperature and lowerTemperature.
      virtual double chooseRegion ( const double temperature, const double pressure, const double salinity,
                                    const double higherTemperature, const double lowerTemperature ) const;
   };

} // end GeoPhysics

#endif // GEOPHYSICS_BRINE_VISCOSITY_H_
