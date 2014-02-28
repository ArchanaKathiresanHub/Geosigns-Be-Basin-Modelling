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

namespace{
struct AbsoluteLessThan{
   AbsoluteLessThan(double const& t)
   : value( t )
   {
      // empty
   }
   bool operator()( double const& val ){ return fabs(val)<value; }
   double value;
};
}

int Proxy::calculateSVD( vector<vector<double> >& a, vector<double>& w, vector<vector<double> >& v )
{
   const size_t nRows( a.size() );
   const size_t nCols( nRows ? a.front().size() : 0 );
   if (  nRows == 0 || nCols == 0 )
   {
      return 0;
   }
   // Clearing not necessary
   w.resize( nCols );
   v.resize( nCols, RealVector( nCols ) );

   // Do the singular value decomposition by calling svdcmp of which the current implementation
   // does not fully support underdetermined (nRows < nCols) systems of equations.
   // Note: In the context of polynomial proxies, nRows = nCols also refers to an underdetermined
   // system as the column that corresponds to the intercept has been eliminated from matrix a.
   assert( nRows > nCols );
   int stat = svdcmp( a, w, v );
   assert( a.size() == nRows );
   assert( a.front().size() == nCols );
   return stat;
}

void Proxy::calculateCoefficients( int stat, vector<vector<double> > const& a, vector<double> const& w,
            vector<vector<double> > const& v, vector<double> const& b, vector<double> & coef )
{
   const size_t nRows( a.size() );
   const size_t nCols( nRows ? a.front().size() : 0 );
   if ( nRows == 0 || nCols == 0 )
   {
      coef.clear();
      return;
   }
   assert( w.size() == nCols );
   assert( v.size() == nCols );
   assert( v.front().size() == nCols );
   assert( b.size() == nRows );

   // Singular values w smaller than tolerance*wmax are discarded
   const double tolerance = nRows * MachineEpsilon();
   const double wmax(*(std::max_element( w.begin(), w.end() ) ) );
   const double threshold( wmax*tolerance );

   // Set diagonal terms to zero for insensitive elements
   vector<double> SV( w );
   std::replace_if( SV.begin(), SV.end(), AbsoluteLessThan(threshold), 0.0 );

   // Backsubstitution to obtain the coefficients
   // coef does not have to be cleared
   coef.resize( nCols );
   if ( stat == 0 )
   {
      svbksb( a, SV, v, b, coef, threshold );
   }
   else
   {
      coef.assign( nCols, 0.0 );
   }
}

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
