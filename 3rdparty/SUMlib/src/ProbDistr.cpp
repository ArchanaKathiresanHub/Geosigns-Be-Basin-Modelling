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
   const size_t size( parSet.size() );
   priorProb.resize( size );
   for ( size_t i = 0; i < size; ++i )
   {
      priorProb[i] = calcLogPriorProb( parSet[i] );
   }
}

double ProbDistr::calcLogWeight( double rel_p, RealVector const& weights ) const
{
   size_t nbOfIntervals = weights.size() - 1;
   assert( nbOfIntervals > 0 );
   
   size_t nearestIndex = int( nbOfIntervals * rel_p + 0.5 ); //round to nearest index
   assert( nearestIndex <= nbOfIntervals );
   
   double weight_p = weights[nearestIndex]; //determine corresponding weight
   if ( weight_p < CloseToZero ) //not likely to happen, just a precaution
   {
      return MinInf;
   }
   else if ( ( nearestIndex == 0 ) || ( nearestIndex == nbOfIntervals ) )
   {
      return log( 2 * weight_p ); //factor 2 to correct for the smaller boundary intervals
   }
   else
   {
      return log( weight_p );
   }
}

} // namespace SUMlib
