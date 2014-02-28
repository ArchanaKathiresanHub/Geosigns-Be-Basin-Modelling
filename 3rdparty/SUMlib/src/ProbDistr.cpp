// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#include <cassert>
#include <cmath>
#include <algorithm>

#include "BaseTypes.h"
#include "ProbDistr.h"

namespace SUMlib
{

const double ProbDistr::MinInf = -1e300;
const double ProbDistr::CloseToZero = 1e-200;

void ProbDistr::calcLogPriorProb( ParameterSet const& parSet, RealVector &priorProb ) const
{
   const unsigned int size( parSet.size() );
   priorProb.resize( size );
   for ( unsigned int i = 0; i < size; ++i )
   {
      priorProb[i] = calcLogPriorProb( parSet[i] );
   }
}

double ProbDistr::calcLogWeight( double rel_p, RealVector const& weights ) const
{
   // Precaution: Truncate in case of extrapolation beyond bounds!
   if ( rel_p < 0.0 )
   {
      rel_p = 0.0;
   }
   else if ( rel_p > 1.0 )
   {
      rel_p = 1.0;
   }
   
   size_t nbOfIntervals = weights.size() - 1;
   assert( nbOfIntervals > 0 );

   // To interpolate between nearest weights, define corresponding indices and factor
   double interpolationFactor = nbOfIntervals * rel_p;
   size_t index1 = int( interpolationFactor ); //truncate
   if ( index1 == nbOfIntervals ) index1--; //avoid index2 = index1 + 1 out of bounds
   interpolationFactor -= index1; //factor now between 0 and 1
   size_t index2 = index1 + 1;

   // Interpolate between nearest weights, and update log-Likelihood
   double weight_p = ( 1.0 - interpolationFactor ) * weights[index1];
   weight_p += interpolationFactor * weights[index2];
   if ( weight_p < CloseToZero )
   {
      return MinInf;
   }
   else
   {
      return log( weight_p );
   }
}

} // namespace SUMlib
