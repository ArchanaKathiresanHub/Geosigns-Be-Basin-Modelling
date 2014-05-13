// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#include <algorithm>
#include <cassert>
#include <vector>

#include "BaseTypes.h"
#include "Exception.h"
#include "KrigingWeights.h"
#include "Proxy.h"
#include "NumericUtils.h"

using std::vector;

namespace SUMlib {

void Proxy::getProxyValues( ParameterSet const& parSet, TargetSet & targets ) const
{
   const size_t nCases( parSet.size() );
   targets.clear();
   targets.resize( nCases );

   for ( unsigned int i = 0; i < nCases; ++i )
   {
      targets[i] = getProxyValue( parSet[i] );
   }
}

double Proxy::getProxyValue( KrigingWeights const&, Parameter const& p, KrigingType krigingType ) const
{
   return getProxyValue( p, krigingType );
}

void Proxy::calcKrigingWeights( Parameter const& p, KrigingType, KrigingWeights& krigingWeights ) const
{
   krigingWeights.zeroWeights( p.size() );
}

} // namespace SUMlib
