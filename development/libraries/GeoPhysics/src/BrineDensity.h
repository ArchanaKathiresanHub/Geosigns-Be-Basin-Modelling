//
// Copyright (C) 2014 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef GEOPHYSICS_BRINE_DENSITY_H_
#define GEOPHYSICS_BRINE_DENSITY_H_

#include "BrinePhases.h"

namespace GeoPhysics
{
   /// \brief BrineDensity is intended to handle the calculations of seismic velocity for brines whose physical parameters (T,P,S) are
   ///        within the allowed ranges (see BrinePhases.C). It uses Batzle-Wang analytic equation in the aqueous phase and ideal-gas
   ///        equation in the vapour phase. In the transition region bi-linear interpolation between the two approaches is used.

   class BrineDensity: public BrinePhases
   {
   public:
      /// Constructor
      BrineDensity() {}
      /// Virtual destructor
      virtual ~BrineDensity() {}
      /// Routine that chooses the appropriate region in which the pressure derivative should be calculated and returns the value of that derivative.
      /// \pre Input values should be within the defined ranges. In case they are not they will be internally adjusted to the relevant limits.
      /// \post The return of a double.
      double computeDerivativeP ( const double temperature, const double pressure, const double salinity ) const;

      /// Routine that chooses the appropriate region in which the temperature derivative should be calculated and returns the value of that derivative.
      /// \pre Input values should be within the defined ranges. In case they are not they will be internally adjusted to the relevant limits.
      /// \post The return of a double.
      double computeDerivativeT ( const double temperature, const double pressure, const double salinity ) const;

   protected:
      /// Analytic function implementing the Batzle-Wang equation for the value of density
      /// and applying it only in the aqueous phase of the brine.
      /// \pre Requires the passed arguments to be within the allowed ranges.
      /// \post Guarantees the return of a non-negative value for the density.
      double aqueousBatzleWang ( const double temperature, const double pressure, const double salinity ) const;

      /// Analytic function implementing the ideal-gas equation for the value of the density of a brine.
      /// \pre Requires the passed arguments to be within the allowed ranges (see BrinePhases.C).
      /// \post Guarantees the return of a non-negative value for the density.
      double vapourIdealGas ( const double temperature, const double pressure, const double salinity ) const;

      /// Linearly interpolates between the values at the two sides of the transition region and returns the value.
      /// \pre Requires the passed arguments to be within the allowed ranges (see BrinePhases.C) and lowerTemperature < higherTemperature.
      /// \post Guarantees the return of a non-negative value for the density. 
      double transitionRegion ( const double temperature, const double pressure, const double salinity,
                                const double higherTemperature, const double lowerTemperature ) const;

      /// Partial derivative of the density (in the ideal-gas scenario) with respect to temperature.
      /// \pre Requires positive values for temperature, pressure, salinity.
      /// \post Guarantees the return of a negative number.
      double derivativeTemperatureIdealGas( const double temperature, const double pressure, const double salinity ) const;

      /// Partial derivative of the density (using the Batzle-Wang equation) with respect to temperature.
      /// \pre Requires positive values for temperature, pressure, salinity.
      /// \post Guarantees the return of a negative number.
      double derivativeTemperatureBatzleWang( const double temperature, const double pressure, const double salinity ) const;

      /// Partial derivative of the density (in the ideal-gas scenario) with respect to pressure.
      /// \pre Requires positive values for temperature, pressure, salinity.
      /// \post Guarantees the return of a positive number.
      double derivativePressureIdealGas( const double temperature, const double pressure, const double salinity ) const;

      /// Partial derivative of the density (using the Batzle-Wang equation) with respect to pressure.
      /// \pre Requires positive values for temperature, pressure, salinity.
      /// \post Guarantees the return of a positive number.
      double derivativePressureBatzleWang( const double temperature, const double pressure, const double salinity ) const;

   private:
      /// Depending on the ordering of temperature, higherTemperature and lowerTemperature calls the appropriate function to calculate
      /// the value of the brine parameter of interest. It then returns the value returned by that function without further checks.
      /// \pre Requires the triplet of T,P,S to be within the allowed ranges and lowerTemperature < higherTemperature.
      /// \post Guarantees the return of the return value of the appropriate function to be called depending on temperature, higherTemperature and lowerTemperature.
      virtual double chooseRegion ( const double temperature, const double pressure, const double salinity,
                                    const double higherTemperature, const double lowerTemperature ) const;

   };

} /// end GeoPhysics

#endif /// GEOPHYSICS_BRINE_DENSITY_H_
