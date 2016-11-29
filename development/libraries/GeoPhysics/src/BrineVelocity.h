//
// Copyright (C) 2014 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef GEOPHYSICS_BRINE_VELOCITY_H_
#define GEOPHYSICS_BRINE_VELOCITY_H_

#include "BrinePhases.h"

namespace GeoPhysics
{
   namespace Brine
   {
      /// To ensure that the Batzle-Wang equation always returns positive values,
      /// a special and more strict limit on the maximum value of the pressure is needed.
      constexpr double PressureMaxForVelocity = 100.0;

      /// \brief Velocity is intended to handle the calculations of seismic velocity for brines whose physical parameters (T,P,S) are
      ///        within the allowed ranges (see BrinePhases.C). It uses Batzle-Wang analytic equation in the aqueous phase and ideal-gas
      ///        equation in the vapour phase. In the transition region bi-linear interpolation between the two approaches is used.
      class Velocity: public Phases
      {
      public:
         /// Constructor.
         Velocity( const double salinity );
         /// Virtual destructor
         virtual ~Velocity() {}
      protected:
         /// Analytic function implementing the Batzle-Wang equation for the value of the seismic velocity
         /// and applying it only in the aqueous phase of the brine.
         /// \pre Requires the passed arguments to be within the allowed ranges.
         /// \post Guarantees the return of a non-negative value for the velocity.
         double aqueousBatzleWang ( const double temperature, const double pressure ) const;

         /// Analytic function implementing the ideal-gas equation for the value of the seismic velocity of a brine.
         /// \pre Requires the passed arguments to be within the allowed ranges (see BrinePhases.C).
         /// \post Guarantees the return of a non-negative value for the velocity.
         double vapourIdealGas ( const double temperature ) const;

         /// Linearly interpolates between the values at the two sides of the transition region and returns the value.
         /// \pre Requires the passed arguments to be within the allowed ranges (see BrinePhases.C) and lowerTemperature < higherTemperature.
         /// \post Guarantees the return of a non-negative value for the velocity.
         double transitionRegion ( const double temperature,
                                   const double pressure,
                                   const double higherTemperature,
                                   const double lowerTemperature ) const;

      private:
         /// Depending on the ordering of temperature, higherTemperature and lowerTemperature calls the appropriate function to calculate
         /// the value of the brine parameter of interest. It then returns the value returned by that function without further checks.
         /// \pre Requires the triplet of T,P,S to be within the allowed ranges and lowerTemperature < higherTemperature.
         /// \post Guarantees the return of the return value of the appropriate function to be called depending on temperature, higherTemperature and lowerTemperature.
         virtual double chooseRegion ( const double temperature, 
                                       const double pressure,
                                       const double higherTemperature,
                                       const double lowerTemperature ) const;
         virtual void   chooseRegion ( const int n,
                                       ArrayDefs::ConstReal_ptr temperature,
                                       ArrayDefs::ConstReal_ptr pressure,
                                       ArrayDefs::ConstReal_ptr higherTemperature,
                                       ArrayDefs::ConstReal_ptr lowerTemperature,
                                       ArrayDefs::Real_ptr brineProp ) const {};
      };

   } /// end Brine

} // end GeoPhysics

#endif // GEOPHYSICS_BRINE_VELOCITY_H_
