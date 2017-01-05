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

#include "ArrayDefinitions.h"
#include "BrinePhases.h"

namespace GeoPhysics
{
   namespace Brine
   {
      /// Forward declaration
      class PhaseStateScalar;
      class PhaseStateVec;

      /// \brief Density is intended to handle the calculations of seismic velocity for brines whose physical parameters (T,P,S) are
      ///        within the allowed ranges (see BrinePhases.C). It uses Batzle-Wang analytic equation in the aqueous phase and ideal-gas
      ///        equation in the vapour phase. In the transition region bi-linear interpolation between the two approaches is used.
      class Density
      {
         /// Finite difference stencil size
         static const int s_fdStencil = 4;

      public:
         /// Constructor
         Density();
         /// Virtual destructor
         ~Density();

         /// Routine that chooses the appropriate region in which the pressure derivative should be calculated and returns the value of that derivative.
         /// \pre Input values should be within the defined ranges. In case they are not they will be internally adjusted to the relevant limits.
         /// \post The return of a double.
         double computeDerivativeP( const GeoPhysics::Brine::PhaseStateScalar & phase ) const;
         void   computeDerivativeP( const GeoPhysics::Brine::PhaseStateVec & phases,
                                    ArrayDefs::Real_ptr derivative ) const;

         /// Routine that chooses the appropriate region in which the temperature derivative should be calculated and returns the value of that derivative.
         /// \pre Input values should be within the defined ranges. In case they are not they will be internally adjusted to the relevant limits.
         /// \post The return of a double.
         double computeDerivativeT( const GeoPhysics::Brine::PhaseStateScalar & phase ) const;
         void   computeDerivativeT( const GeoPhysics::Brine::PhaseStateVec & phases,
                                    ArrayDefs::Real_ptr derivative ) const;
         
         /// Depending on the ordering of temperature, higherTemperature and lowerTemperature calls the appropriate function to calculate
         /// the value of the brine parameter of interest. It then returns the value returned by that function without further checks.
         /// \pre Requires the triplet of T,P,S to be within the allowed ranges and lowerTemperature < higherTemperature.
         /// \post Guarantees the return of the return value of the appropriate function to be called depending on temperature, higherTemperature and lowerTemperature.
         double get( const GeoPhysics::Brine::PhaseStateScalar & phase ) const;
         void   get( const GeoPhysics::Brine::PhaseStateVec & phase,
                     ArrayDefs::Real_ptr brineProp ) const;

      private:
         /// Analytic function implementing the Batzle-Wang equation for the value of density
         /// and applying it only in the aqueous phase of the brine.
         /// \pre Requires the passed arguments to be within the allowed ranges.
         /// \post Guarantees the return of a non-negative value for the density.
#if defined(__INTEL_COMPILER)
         // GCC 4.9.3 gives multiple definition error on linking unit test
         #pragma omp declare simd notinbranch
#endif
         double aqueousBatzleWang ( const double temperature, const double pressure, const double salinity ) const;

         /// Analytic function implementing the ideal-gas equation for the value of the density of a brine.
         /// \pre Requires the passed arguments to be within the allowed ranges (see BrinePhases.C).
         /// \post Guarantees the return of a non-negative value for the density.
#if defined(__INTEL_COMPILER)
         // GCC 4.9.3 gives multiple definition error on linking unit test
         #pragma omp declare simd notinbranch
#endif
         double vapourIdealGas ( const double temperature, const double pressure, const double salinity ) const;

         /// Linearly interpolates between the values at the two sides of the transition region and returns the value.
         /// \pre Requires the passed arguments to be within the allowed ranges (see BrinePhases.C) and lowerTemperature < higherTemperature.
         /// \post Guarantees the return of a non-negative value for the density.
#if defined(__INTEL_COMPILER)
         // GCC 4.9.3 gives multiple definition error on linking unit test
         #pragma omp declare simd notinbranch
#endif
         double transitionRegion ( const double temperature,
                                   const double pressure,
                                   const double salinity,
                                   const double higherTemperature,
                                   const double lowerTemperature ) const;

         /// Partial derivative of the density (in the ideal-gas scenario) with respect to temperature.
         /// \pre Requires positive values for temperature, pressure, salinity.
         /// \post Guarantees the return of a negative number.
#if defined(__INTEL_COMPILER)
         // GCC 4.9.3 gives multiple definition error on linking unit test
         #pragma omp declare simd notinbranch
#endif
         double derivativeTemperatureIdealGas( const double temperature, const double pressure, const double salinity ) const;

         /// Partial derivative of the density (using the Batzle-Wang equation) with respect to temperature.
         /// \pre Requires positive values for temperature, pressure, salinity.
         /// \post Guarantees the return of a negative number.
#if defined(__INTEL_COMPILER)
         // GCC 4.9.3 gives multiple definition error on linking unit test
         #pragma omp declare simd notinbranch
#endif
         double derivativeTemperatureBatzleWang( const double temperature, const double pressure, const double salinity ) const;

         /// Partial derivative of the density (in the ideal-gas scenario) with respect to pressure.
         /// \pre Requires positive values for temperature, pressure, salinity.
         /// \post Guarantees the return of a positive number.
#if defined(__INTEL_COMPILER)
         // GCC 4.9.3 gives multiple definition error on linking unit test
         #pragma omp declare simd notinbranch
#endif
         double derivativePressureIdealGas( const double temperature, const double pressure, const double salinity ) const;

         /// Partial derivative of the density (using the Batzle-Wang equation) with respect to pressure.
         /// \pre Requires positive values for temperature, pressure, salinity.
         /// \post Guarantees the return of a positive number.
#if defined(__INTEL_COMPILER)
         // GCC 4.9.3 gives multiple definition error on linking unit test
         #pragma omp declare simd notinbranch
#endif
         double derivativePressureBatzleWang( const double temperature, const double pressure, const double salinity ) const;

         double computeDerivativeT( const double temperature,
                                    const double pressure,
                                    const double salinity,
                                    const double lowerTemperature,
                                    const double higherTemperature ) const;

         double computeDerivativeP( const double temperature,
                                    const double pressure,
                                    const double salinity,
                                    const double lowerTemperature,
                                    const double higherTemperature ) const;

         /// Finite differences vector for pressure perturbation
         ArrayDefs::Real_ptr m_fdPres;

         /// Finite differences vector for temperature perturbation
         ArrayDefs::Real_ptr m_fdTemp;

         /// Finite differences vector for density perturbation
         ArrayDefs::Real_ptr m_fdDens;

         /// Finite difference perturbated phases
         mutable GeoPhysics::Brine::PhaseStateVec m_perturbatedPhases;
      };

   } /// end Brine

} /// end GeoPhysics


/// Batzle-Wang formula for fluids in the aqueous (liquid) phase.
inline double GeoPhysics::Brine::Density::aqueousBatzleWang( const double temperature,
                                                             const double pressure,
                                                             const double salinity ) const
{
   const double t = temperature;
   const double p = pressure;
   const double s = salinity;

   return 1000.0 * ( s * s * ( 0.44 - 0.0033 * t )
                     + p * p * ( -3.33e-7 - 2.0e-9 * t )+
                     1.75e-9 * ( -1718.91 + t ) * ( -665.977 + t ) * ( 499.172 + t )+
                     s * ( 0.668 + 0.00008 * t + 3.0e-6 * t * t )+
                     p * ( s * ( 0.0003 - 0.000013 * t ) + s * s * ( -0.0024 + 0.000047 * t )-
                           1.3e-11 * ( -1123.64 + t ) * ( 33476.2 - 107.125 * t + t * t )));
}


/// Ideal-gas formula for fluids in the vapour (gas) phase.
inline double GeoPhysics::Brine::Density::vapourIdealGas( const double temperature,
                                                          const double pressure,
                                                          const double salinity ) const
{  
   const double t = temperature;
   const double p = pressure;
   const double s = salinity;

   return p * 1.0e2 / ( t + 273.0 ) * 1.67/1.38 * ( 18.0 * (1.0-s) + 58.44 * s );
}


/// Interpolation between last aqueous value (at T1) and first vapour value (at T2).
inline double GeoPhysics::Brine::Density::transitionRegion( const double temperature,
                                                            const double pressure,
                                                            const double salinity,
                                                            const double higherTemperature,
                                                            const double lowerTemperature ) const
{
   double aqueous = aqueousBatzleWang( lowerTemperature, pressure, salinity );
   double vapour  = vapourIdealGas( higherTemperature, pressure, salinity );
   return ( aqueous + ( temperature - lowerTemperature ) * ( vapour - aqueous ) / ( higherTemperature - lowerTemperature ) );
}


/// Analytic drho / dT in the vapour region.
inline double GeoPhysics::Brine::Density::derivativeTemperatureIdealGas( const double temperature,
                                                                         const double pressure,
                                                                         const double salinity ) const
{
   const double t = temperature;
   const double p = pressure;
   const double s = salinity;

   return - 1.0e2 * 1.67 / 1.38 / ( t + 273.0 ) / ( t + 273.0 ) * p * ( 18.0 * ( 1.0 - s ) + 58.44 * s );
}


/// Analytic drho / dT in the aqueous region.
inline double GeoPhysics::Brine::Density::derivativeTemperatureBatzleWang( const double temperature,
                                                                           const double pressure,
                                                                           const double salinity ) const
{
   const double t = temperature;
   const double p = pressure;
   const double s = salinity;

   double densityDer = (1.e-6) * ( -80.0 - 2.0 * p - 0.002 * p * p - 6.6 * t + 0.032 * p * t + 0.00525 * t * t - 0.000039 * p * t * t );
   return 1000.0 * ( densityDer + (1.e-6) * s * ( 80.0 - 13.0 * p - 3300.0 * s + 6.0 * t + 47.0 * p * s ));
}


/// Analytic drho / dP in the vapour region.
inline double GeoPhysics::Brine::Density::derivativePressureIdealGas( const double temperature,
                                                                      const double pressure,
                                                                      const double salinity ) const
{
   const double t = temperature;
   const double p = pressure;
   const double s = salinity;

   return 1.0e2 * 1.67 / 1.38 / ( t + 273.0 ) * ( 18.0 * ( 1.0 - s ) + 58.44 * s );
}


/// Analytic drho / dP in the aqueous region.
inline double GeoPhysics::Brine::Density::derivativePressureBatzleWang( const double temperature,
                                                                        const double pressure,
                                                                        const double salinity ) const
{
   const double t = temperature;
   const double p = pressure;
   const double s = salinity;

   double densityDer = (1.e-6) * ( 489.0 - 0.666 * p - 2.0 * t - 0.004 * p * t + 0.016 * t * t - 0.000013 * t * t * t );
   return 1000.0 * ( densityDer + (1.e-6) * s * ( 300.0 - 2400.0 * s - 13.0 * t + 47.0 * s * t ));
}

#endif /// GEOPHYSICS_BRINE_DENSITY_H_
