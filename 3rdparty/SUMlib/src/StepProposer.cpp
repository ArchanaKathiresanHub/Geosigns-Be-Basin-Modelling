// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#include <algorithm>
#include <cmath>

#include "Exception.h"
#include "RandomGenerator.h"
#include "StepProposer.h"


namespace SUMlib {

// Tornado step size = rangeFraction * range. The rangeFraction should be a bit
// smaller than the mult = 0.01 in CalculateTolerance( pdf, eps ).
const double rangeFraction = 0.009;

StepProposer::StepProposer(
      RandomGenerator& trg,
      const size_t pSize,
      std::vector<double> const& min,
      std::vector<double> const& max,
      unsigned int nbSteps  ) :
   m_rg( trg ),
   m_min( min ),
   m_max( max ),
   m_minDp( pSize ),
   m_maxDp( pSize ),
   m_dp( pSize ),
   m_nbSteps( nbSteps )
{
   check();
   for ( size_t i = 0; i < m_dp.size(); ++i )
   {
      double range = m_max[i] - m_min[i];
      m_minDp[i] = 1e-3 * range / sqrt( double( pSize ) );
      m_maxDp[i] = 0.1 * range / sqrt( double( pSize ) );
   }
}


StepProposer::~StepProposer()
{
   // empty
}

void StepProposer::check() const
{
   if ( m_min.size() != m_max.size() )
   {
      THROW2( DimensionMismatch, "StepProposer::m_min and StepProposer::m_max" );
   }
   if ( m_min.size() != m_dp.size() )
   {
      THROW2( DimensionMismatch, "StepProposer::m_min and StepProposer::m_dp" );
   }
   if ( m_nbSteps == 0 )
   {
      THROW2( ValueOutOfBounds, "StepProposer::m_nbSteps must be positive" );
   }
}


/**
 *  Propose a new set of parameters p2 using a transition probability distribution q.
 *  Here we propose new parameters in such a way that the probability of a
 *  transition p1->p2 is the same as for p2->p1. Given the constraints on the
 *  values a parameter may have, this can only be achieved by utilizing the so-called
 *  reflection rule at the bounds. As a result, the probability ratio 'tr' of this
 *  symmetric transition is always one: q(p1->p2) = q(p2->p1).
 *
 *  Here, q is set to a Uniform distribution within a small box of size 2*dp for each
 *  dimension, centered around the current parameter values.
 *
 *  The reflection principle must be applied if the proposed p2 violates one or more
 *  parameter bounds; if a bound is exceeded by an absolute value, say ds <= dp, then
 *  the bound must serve as a reflection wall by bouncing back the proposed value such
 *  that the final value is within range at a value ds of the reflection bound.
 */

void StepProposer::proposeStep(
   std::vector<std::vector<double> >& p,    // In: p1, Out: p2
   std::vector<double>& tr ) const
{
   if( p.size() != tr.size() )
   {
      THROW2( DimensionMismatch, "p and tr" );
   }

   // For all parameters in the set, calculate new proposed values
   // and return log( tr ) = 0.
   for ( size_t i = 0; i < p.size(); ++i )
   {
      proposeRandomStep( m_rg, p[i], tr[i], static_cast<unsigned int>(i));
   }
}

/**
 *  Propose new parameter values 'p' depending on:
 *  - the old parameter values 'p',
 *  - the parameter value bounds 'min' and 'max'.
 */

void StepProposer::proposeRandomStep(
            RandomGenerator& rg,
            std::vector<double>& p,    // In: p1, Out: p2
            double& tr,
            unsigned int iChain ) const
{
   // Perform a few steps
   for ( size_t steps = 0; steps < m_nbSteps; ++steps )
   {
      // For all parameter values
      for ( size_t i = 0; i < p.size(); ++i )
      {
         // Propose a new value by drawing a random number within
         // a step size dp of the current p[i] value.
         p[i] = p[i] - m_dp[i][iChain] + rg.uniformRandom() * ( 2 * m_dp[i][iChain] );
         if ( p[i] < m_min[i] )
         {
            p[i] = m_min[i] + ( m_min[i] - p[i] ); //bounce back
         }
         else if ( p[i] > m_max[i] )
         {
            p[i] = m_max[i] - ( p[i] - m_max[i] ); //bounce back
         }
      }
   }

   // Log( transition probability( p1->p2 ) / transition probability( p2 -> p1 ) )
   tr = 0.0; //by design
}

/**
 *  Propose new parameter values 'p' depending on:
 *  - the old parameter values 'p',
 *  - the parameter value bounds 'min' and 'max'.
 */

unsigned int StepProposer::proposeTornadoStep( std::vector<double>& p, unsigned int step,
   unsigned int nbSteps, unsigned int lastStep ) const
{
   unsigned int proposedStep = lastStep + step; //Tornado shift
   if ( proposedStep >= nbSteps )
   {
      proposedStep -= nbSteps; //cyclic determination of Tornado shift
   }
   bool positiveShift = true;
   if ( proposedStep % 2 == 0 ) positiveShift = false;
   unsigned int j = proposedStep / 2; //deliberate truncation to get parameter index!
   double shiftLength = rangeFraction * (m_max[j] - m_min[j]);
   if ( positiveShift )
   {
      if ( p[j] + 0.5*shiftLength > m_max[j] )
      {
         p[j] -= shiftLength;
      }
      else
      {
         p[j] = std::min<double>( p[j] + shiftLength, m_max[j] );
      }
   }
   else
   {
      if ( p[j] - 0.5*shiftLength < m_min[j] )
      {
         p[j] += shiftLength;
      }
      else
      {
         p[j] = std::max<double>( p[j] - shiftLength, m_min[j] );
      }
   }
   return proposedStep;
}

void StepProposer::adaptStepSize( std::vector<double>& acceptanceRate )
{
   // The acceptance rate should be between 23 and 45 %
   // After: Roberts, Gelman and Gilks 1994
   size_t nChain = acceptanceRate.size();
   for ( size_t i = 0; i < m_dp.size(); ++i )
   {
      for ( size_t j = 0; j < nChain; ++j )
      {
         if ( acceptanceRate[j] < 23.0 )
         {
            m_dp[i][j] = std::max<double>( 0.9 * m_dp[i][j], m_minDp[i] );
         }
         else if ( acceptanceRate[j] > 45.0 )
         {
            m_dp[i][j] = std::min<double>( 1.1 * m_dp[i][j], m_maxDp[i] );
         }
      }
   }
}

void StepProposer::setStepSize( const std::vector<double>& dp, unsigned int chainSize )
{
   for ( unsigned int i = 0; i < dp.size(); ++i )
   {
      double value  = std::max<double>( dp[i], m_minDp[i] );
      value = std::min<double>( value, m_maxDp[i] );
      m_dp[i].resize( chainSize, value );
   }
}

} // namespace SUMlib
