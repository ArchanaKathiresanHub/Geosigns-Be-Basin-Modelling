//
// Copyright (C) 2014-2017 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef GEOPHYSICS_BRINE_VISCOSITY_H_
#define GEOPHYSICS_BRINE_VISCOSITY_H_

#include <cmath>
#include "ArrayDefinitions.h"

namespace GeoPhysics
{
   namespace Brine
   {
      /// Forward declaration
      class PhaseStateScalar;
      class PhaseStateVec;

      /// The constant returned by transitionRegion().
      constexpr double VapourViscosity = 2.5e-5;

      /// \brief Viscosity is intended to handle the calculations of viscosity for brines whose physical parameters (T,P,S) are
      ///        within the allowed ranges (see BrinePhases.cpp). It uses Batzle-Wang analytic equation in the aqueous phase and a constant
      ///        value in the vapour phase. In the transition region bi-linear interpolation between the two approaches is used.
      class Viscosity
      {
      public:
         /// Constructor.
         explicit Viscosity( const double salinity );
         /// Virtual destructor.
         virtual ~Viscosity();

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
#if defined(__INTEL_COMPILER) and defined(NDEBUG)
         // GCC 4.9.3 gives multiple definition error on linking unit test
         // Intel 16.0.1.20151021 in debug mode gives warning #13401: vector function was emulate
         #pragma omp declare simd notinbranch
#endif
         double aqueousBatzleWang ( const double temperature ) const;

         /// Returns a constant value for brines of any combination of parameters as long as they are in the vapour phase.
         /// \pre None.
         /// \post Guarantees the return of a constant value.
         double vapourConstant () const;

         /// Linearly interpolates between the values at the two sides of the transition region and returns the value.
         /// \pre Requires the passed arguments to be within the allowed ranges (see BrinePhases.cpp) and lowerTemperature < higherTemperature.
         /// \post Guarantees the return of a non-negative value for the density.
#if defined(__INTEL_COMPILER) and defined(NDEBUG)
         // GCC 4.9.3 gives multiple definition error on linking unit test
         // Intel 16.0.1.20151021 in debug mode gives warning #13401: vector function was emulate
         #pragma omp declare simd notinbranch
#endif
         double transitionRegion ( const double temperature,
                                   const double pressure,
                                   const double higherTemperature,
                                   const double lowerTemperature ) const;

         /// ( 0.42 * (std::pow ( salinity, 0.8 ) - 0.17) * (std::pow ( salinity, 0.8 ) - 0.17) + 0.045 )
         const double m_term1;

         /// 0.001 *(0.1 + 0.333 * salinity)
         const double m_term2;

         /// 0.001 *(1.65 + 91.9 * salinity * salinity * salinity)
         const double m_term3;
      };

   } /// end Brine

} // end GeoPhysics

// Batzle-Wang formula for fluids in the aqueous (liquid) phase.
inline double GeoPhysics::Brine::Viscosity::aqueousBatzleWang( const double temperature ) const
{
   return  m_term2 + m_term3 * std::exp ( - m_term1 * std::pow ( temperature, 0.8 ) );
}

// Constant viscosity in the vapour phase.
inline double GeoPhysics::Brine::Viscosity::vapourConstant() const
{
   return VapourViscosity;
}

// Interpolation between last aquous value (at T1) and first vapour value (at T2).
inline double GeoPhysics::Brine::Viscosity::transitionRegion( const double temperature,
                                                              const double ,
                                                              const double higherTemperature,
                                                              const double lowerTemperature) const
{
   const double aqueous = aqueousBatzleWang( lowerTemperature );
   const double vapour  = vapourConstant();
   return ( aqueous + ( temperature - lowerTemperature ) * ( vapour - aqueous ) / ( higherTemperature - lowerTemperature ) );
}

#endif // GEOPHYSICS_BRINE_VISCOSITY_H_
