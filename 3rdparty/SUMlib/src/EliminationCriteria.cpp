// Copyright 2015, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#include "EliminationCriteria.h"
#include "NumericUtils.h"
#include "ProxyBuilder.h"

#include <boost/none.hpp>
#include <cmath>
#include <iterator>
#include <algorithm>

namespace SUMlib {

EliminationCriterion::~EliminationCriterion()
{
}

NoElimination::~NoElimination()
{
}

boost::optional<unsigned int> NoElimination::operator()( ProxyBuilder const& )
{
   return boost::none;
}

BonferroniElimination::BonferroniElimination(double mult) : m_multiplier(mult)
{
   assert(mult >= 0);
}

BonferroniElimination::~BonferroniElimination()
{
}

boost::optional<unsigned int> BonferroniElimination::operator()( ProxyBuilder const& builder )
{
   if ( builder.baseVars().empty() ) return boost::none;

   // standard errors on the coefficients (including the intercept)
   std::vector<double> t = builder.calcStdErrors();

   // We are only interested in the t-statistics corresponding to the non-intercept coefficients.
   std::transform( builder.coefficients().begin(), builder.coefficients().end(), t.begin()+1, t.begin(), static_cast<double (*)(double, double)>( calculateTStatistic ) );

   // Vector t got overwritten with a vector of size smaller by 1. This is why at the end we pop the last element.
   t.pop_back();

   assert( ! t.empty() );

   std::transform(t.begin(), t.end(), t.begin(), static_cast<double (*)(double)>(std::fabs));

   std::vector<double>::iterator m = std::min_element(t.begin(), t.end());

   return ((*m) * (*m) < m_multiplier * 2 * std::log( t.size() + 1. )) ? boost::optional<unsigned int>( static_cast<unsigned int>(std::distance( t.begin(), m ))) : boost::none;
}

}
