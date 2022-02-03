//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "ExponentialPorosity.h"

// std library
#include <algorithm>
#include <cassert>
#include <cmath>

// utilities library
#include "ConstantsPhysics.h"
using Utilities::Physics::AccelerationDueToGravity;

#include "GeoPhysicalConstants.h"
#include "NumericFunctions.h"

namespace GeoPhysics
{
   ///Parameters from constructor
   ExponentialPorosity::ExponentialPorosity(const double depoPorosity,
                                            const double minimumMechanicalPorosity,
                                            const double compactionIncr,
                                            const double compactionDecr,
                                            const bool   isLegacy) :
      Algorithm(depoPorosity,minimumMechanicalPorosity),
      m_compactionIncr((depoPorosity-minimumMechanicalPorosity>porosityTolerance) ? compactionIncr : 0.0),
      m_compactionDecr((depoPorosity-minimumMechanicalPorosity>porosityTolerance) ? compactionDecr : 0.0)
   {
      m_isLegacy = isLegacy;
   }

   ///FullCompThickness
   double ExponentialPorosity::fullCompThickness(const double maxVesValue,
                                                 const double thickness,
                                                 const double densitydiff,
                                                 const double ,
                                                 const bool ) const
   {
	   const double phi_0 = this->surfacePorosity();
	   const double d = thickness;
       double solidThickness = d * (1.0 - phi_0); ///< begins with the known thickness
       /**
        * Computes the zero-porosity thickness of a layer.
        * 
        * \param maxVesValue
        * \param thickness
        * \param densitydiff = rho_solid - rho_fluid
        * \param 
        * \param 
        * \return the zero-porosity thickness of the layer
        */
       if (densitydiff <= 0.0 || isIncompressible()) {
          return solidThickness;
       }
       else
       {
           double phi_m = m_minimumMechanicalPorosity;
           if (m_isLegacy)
               phi_m = 0.0;   
           const double phi_bar = topPorosity(phi_0, phi_m, maxVesValue, m_compactionIncr);
           const double beta = phi_bar - phi_m;
		   const double alpha = 1 - phi_m;
           const double gamma = AccelerationDueToGravity * densitydiff * m_compactionIncr;
           solidThickness =
                               //d + (1.0 / gamma ) * std::log(1.0 - phi_bar      * (1.0 - std::exp(-        gamma * d))) : ///old FullCompThickness formula*/
                         alpha * d + (1.0 / gamma ) * std::log(1.0 - beta / alpha * (1.0 - std::exp(-alpha * gamma * d)));
       }
       return solidThickness;
   }


   void ExponentialPorosity::calculate( const unsigned int n,
                                        ArrayDefs::ConstReal_ptr ves,
                                        ArrayDefs::ConstReal_ptr maxVes,
                                        const bool includeChemicalCompaction,
                                        ArrayDefs::ConstReal_ptr chemicalComp,
                                        ArrayDefs::Real_ptr porosities ) const
   {
      assert( ((uintptr_t)(const void *)(ves) % 32) == 0 );
      assert( ((uintptr_t)(const void *)(maxVes) % 32) == 0 );
      assert( ((uintptr_t)(const void *)(chemicalComp) % 32) == 0 );
      assert( ((uintptr_t)(const void *)(porosities) % 32) == 0 );

      for( size_t i = 0; i < n; ++i)
      {
         porosities[i] = computeSingleValue( ves[i], maxVes[i], includeChemicalCompaction, chemicalComp[i] );
      }
   }


   void ExponentialPorosity::calculate( const unsigned int n,
                                        ArrayDefs::ConstReal_ptr ves,
                                        ArrayDefs::ConstReal_ptr maxVes,
                                        const bool includeChemicalCompaction,
                                        ArrayDefs::ConstReal_ptr chemicalComp,
                                        ArrayDefs::Real_ptr porosities,
                                        ArrayDefs::Real_ptr porosityDers ) const
   {
      assert( ((uintptr_t)(const void *)(ves) % 32) == 0 );
      assert( ((uintptr_t)(const void *)(maxVes) % 32) == 0 );
      assert( ((uintptr_t)(const void *)(chemicalComp) % 32) == 0 );
      assert( ((uintptr_t)(const void *)(porosities) % 32) == 0 );
      assert( ((uintptr_t)(const void *)(porosityDers) % 32) == 0 );

      for( size_t i = 0; i < n; ++i)
      {
          porosities[i] = computeSingleValue( ves[i], maxVes[i], includeChemicalCompaction, chemicalComp[i] );
          porosityDers[i] = computeSingleValueDerivative( porosities[i], ves[i], maxVes[i], includeChemicalCompaction, chemicalComp[i] );
      }
   }


   double ExponentialPorosity::computeSingleValue( const double ves,
                                                   const double maxVes,
                                                   const bool includeChemicalCompaction,
                                                   const double chemicalCompactionTerm ) const
   {
      double poro = 0.0;

      //legacy behavior
      if (m_isLegacy)
      {
         if (includeChemicalCompaction)
         {
            if (ves >= maxVes)
            {
               poro = (m_depoPorosity - m_minimumMechanicalPorosity) * std::exp( -m_compactionIncr * ves ) + m_minimumMechanicalPorosity;
            }
            else
            {
               poro = (m_depoPorosity - m_minimumMechanicalPorosity) * std::exp( m_compactionDecr * (maxVes - ves) - m_compactionIncr * maxVes ) + m_minimumMechanicalPorosity;
            }

            poro += chemicalCompactionTerm;
            poro = std::max( poro, MinimumPorosity );
         }
         else
         {
            if (ves >= maxVes)
            {
               poro = m_depoPorosity * std::exp( -m_compactionIncr * ves );
            }
            else
            {
               poro = m_depoPorosity * std::exp( m_compactionDecr * (maxVes - ves) - m_compactionIncr * maxVes );
            }
         }
      }
      //New rock property library behavior
      else
      {
         if( ves >= maxVes )
         {
            poro = (m_depoPorosity - m_minimumMechanicalPorosity) * std::exp(-m_compactionIncr * ves) + m_minimumMechanicalPorosity;
         }
         else
         {
            poro = (m_depoPorosity - m_minimumMechanicalPorosity) * std::exp(m_compactionDecr * (maxVes - ves) - m_compactionIncr * maxVes) + m_minimumMechanicalPorosity;
         }

         if (includeChemicalCompaction)
         {
            poro += chemicalCompactionTerm;
            poro = std::max(poro, MinimumPorosityNonLegacy);
         }
      }

      return poro;
   }


   double ExponentialPorosity::computeSingleValueDerivative( const double porosity,
                                                             const double ves,
                                                             const double maxVes,
                                                             const bool includeChemicalCompaction,
                                                             const double chemicalCompactionTerm ) const
   {
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
      double poroDer = 0.0;

      //legacy behavior
      if (m_isLegacy)
      {
         if (includeChemicalCompaction)
         {
            if (porosity == MinimumPorosity)
            {
               poroDer = 0.0;
            }
            else if (ves >= maxVes)
            {
               poroDer = -m_compactionIncr * (porosity - m_minimumMechanicalPorosity - chemicalCompactionTerm);
            }
            else
            {
               poroDer = -m_compactionDecr * (porosity - m_minimumMechanicalPorosity - chemicalCompactionTerm);
            }
         }
         else
         {
            if (ves >= maxVes)
            {
               poroDer = -m_compactionIncr * porosity;
            }
            else
            {
               poroDer = -m_compactionDecr * porosity;
            }
         }
      }
      else
      {
         // new rock property library behavior
         if (porosity == MinimumPorosityNonLegacy)
         {
            poroDer = 0.0;
         }
         else if (ves >= maxVes)
         {
            poroDer = -m_compactionIncr * (porosity - m_minimumMechanicalPorosity - (includeChemicalCompaction?chemicalCompactionTerm:0.0) );
         }
         else
         {
            poroDer = -m_compactionDecr * (porosity - m_minimumMechanicalPorosity - (includeChemicalCompaction?chemicalCompactionTerm:0.0) );
         }
      }

      return poroDer;
   }

}
