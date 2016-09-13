//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "ExponentialPorosity.h"

#include "GeoPhysicalConstants.h"
#include "NumericFunctions.h"
#include <cmath>


using namespace DataAccess;


namespace GeoPhysics
{

   ///Parameters from constructor
   ExponentialPorosity::ExponentialPorosity(const double depoPorosity,
                                            const double minimumMechanicalPorosity,
                                            const double compactionIncr,
                                            const double compactionDecr,
                                            const bool isLegacy) :
                                            Algorithm( depoPorosity, minimumMechanicalPorosity ),
      m_compactionIncr(compactionIncr),
      m_compactionDecr(compactionDecr)
   {
      m_isLegacy = isLegacy;
   }

   ///Exponential porosity function
   double ExponentialPorosity::calculate( const double ves,
                                          const double maxVes,
                                          const bool includeChemicalCompaction,
                                          const double chemicalCompactionTerm ) const
   {
      double calculatedPorosity;
      const bool loadingPhase = (ves >= maxVes);

      //legacy behaviour
      if (m_isLegacy)
      {
         if (includeChemicalCompaction)
         {
            if (loadingPhase)
            {
               calculatedPorosity = (m_depoPorosity - m_minimumMechanicalPorosity) * exp( -m_compactionIncr * ves ) + m_minimumMechanicalPorosity;
            }
            else
            {
               calculatedPorosity = (m_depoPorosity - m_minimumMechanicalPorosity) * exp( m_compactionDecr * (maxVes - ves) - m_compactionIncr * maxVes ) + m_minimumMechanicalPorosity;
            }

            calculatedPorosity += chemicalCompactionTerm;
            calculatedPorosity = NumericFunctions::Maximum( calculatedPorosity, MinimumPorosity );
         }
         else
         {
            if (loadingPhase)
            {
               calculatedPorosity = m_depoPorosity * exp( -m_compactionIncr * ves );
            }
            else
            {
               calculatedPorosity = m_depoPorosity * exp( m_compactionDecr * (maxVes - ves) - m_compactionIncr * maxVes );
            }
         }
      }
      //New rock property library behaviour
      else
      {
         const double minimumMechanicalPorosity = NumericFunctions::Maximum( m_minimumMechanicalPorosity, MinimumPorosityNonLegacy );

         if (loadingPhase)
         {
            calculatedPorosity = (m_depoPorosity - minimumMechanicalPorosity) * exp( -m_compactionIncr * ves ) + minimumMechanicalPorosity;
         }
         else
         {
            calculatedPorosity = (m_depoPorosity - minimumMechanicalPorosity) * exp( m_compactionDecr * (maxVes - ves) - m_compactionIncr * maxVes ) + minimumMechanicalPorosity;
         }

         if (includeChemicalCompaction)
         {
            calculatedPorosity += chemicalCompactionTerm;
            calculatedPorosity = NumericFunctions::Maximum( calculatedPorosity, MinimumPorosityNonLegacy );
         }
      }

      return calculatedPorosity;
   }

   bool ExponentialPorosity::isIncompressible () const {
      return m_compactionIncr == 0.0;
   }


   ///PorosityModel
   Porosity::Model ExponentialPorosity::model() const
   {
      return DataAccess::Interface::EXPONENTIAL_POROSITY;
   }

   ///FullCompThickness
   double ExponentialPorosity::fullCompThickness(const double maxVesValue,
                                                 const double thickness,
                                                 const double densitydiff,
                                                 const double vesScaleFactor,
                                                 const bool overpressuredCompaction) const
   {
      double c1;
      double c2;
      double solid_Thickness;

      if (m_compactionIncr == 0 || densitydiff <= 0.0 ) {
         solid_Thickness = thickness * (1.0 - m_depoPorosity);
      }
      else {
         c1 = AccelerationDueToGravity * densitydiff * m_compactionIncr;

         c2 = (1.0 - exp(-c1 * thickness)) * m_depoPorosity
            * exp(-m_compactionIncr * maxVesValue);

         solid_Thickness = log(1.0 - c2) / c1 + thickness;
      }

      return solid_Thickness;
   }

   ///CompactionCoefficent
   double ExponentialPorosity::compactionCoefficient() const {
      return m_compactionIncr;
   }

   ///PorosityDerivative
   double ExponentialPorosity::calculateDerivative( const double ves,
                                                    const double maxVes,
                                                    const bool includeChemicalCompaction,
                                                    const double chemicalCompactionTerm) const
   {
      //
      //  If there is NO chemical compaction
      //
      //  d Phi
      //  ----- = - cc * Phi             //Legacy
      //  d ves
      //                or
      //  d Phi
      //  ----- = - cc * (Phi -PhiMin)   //New rock property
      //  d ves
      //
      //  else if Phi is greater than MinimumPorosity
      //
      //  d Phi
      //  ----- = - cc * ( Phi - PhiMin - chemicalCompactionTerm )
      //  d ves
      //
      //  otherwise the derivative is zero.
      //

      double porosityDerivative;
      const double porosityValue = calculate(ves, maxVes, includeChemicalCompaction, chemicalCompactionTerm);

      // Chemical compaction equations also depends on VES, should we consider adding the chemical compaction derivative to 
      // these equations?

      //legacy behaviour
      if (m_isLegacy) 
      {
         if (includeChemicalCompaction)
         {
            if (porosityValue == MinimumPorosity)
            {
               porosityDerivative = 0.0;
            }
            else if (ves >= maxVes)
            {
               porosityDerivative = -m_compactionIncr * (porosityValue - m_minimumMechanicalPorosity - chemicalCompactionTerm);
            }
            else
            {
               porosityDerivative = -m_compactionDecr * (porosityValue - m_minimumMechanicalPorosity - chemicalCompactionTerm);
            }
         }
         else
         {
            if (ves >= maxVes)
            {
               porosityDerivative = -m_compactionIncr * porosityValue;
            }
            else
            {
               porosityDerivative = -m_compactionDecr * porosityValue;
            }
         }
      }
      // new rock property library behaviour
      else 
      {
         const double minimumMechanicalPorosity = NumericFunctions::Maximum( m_minimumMechanicalPorosity, MinimumPorosityNonLegacy );
         double localChemicalCompactionTerm;

         if (porosityValue == MinimumPorosityNonLegacy)
         {
            porosityDerivative = 0.0;
         }
         else if (ves >= maxVes)
         {
            porosityDerivative = -m_compactionIncr * (porosityValue - minimumMechanicalPorosity - (includeChemicalCompaction?chemicalCompactionTerm:0.0));
         }
         else
         {
            porosityDerivative = -m_compactionDecr * (porosityValue - minimumMechanicalPorosity - (includeChemicalCompaction?chemicalCompactionTerm:0.0));
         }
      }

      return porosityDerivative;
   }

}
