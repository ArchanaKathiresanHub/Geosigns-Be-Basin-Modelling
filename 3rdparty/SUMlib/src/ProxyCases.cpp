// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#include <algorithm>
#include <cassert>
#include <numeric>
#include <vector>

#include "BaseTypes.h"
#include "CubicProxy.h"
#include "DataStructureUtils.h"
#include "Exception.h"
#include "NumericUtils.h"
#include "ProxyCases.h"
#include "ProxyBuilder.h"
#include "RandomGenerator.h"

using std::vector;

namespace {

/// Very small positive real value
static const double Tiny = 1e-36;

} // anonymous namespace

namespace SUMlib {

ProxyCases::ProxyCases( void )
   : m_builder( NULL )
{
   // empty
}

ProxyCases::ProxyCases( ParameterSet const& parSet, TargetSet const& targetSet )
   : m_builder( NULL )
{
   initialise( parSet, targetSet, CaseList( parSet.size(), true ) );
}


ProxyCases::ProxyCases( ParameterSet const& parSet, TargetSet const& targetSet, CaseList const& cases )
   : m_builder( NULL )
{
   initialise( parSet, targetSet, cases );
}


ProxyCases::~ProxyCases( void )
{
   delete m_builder;
}

void ProxyCases::initialise( ParameterSet const&parSet, TargetSet const& targetSet, CaseList const& cases )
{
   const size_t nCases( parSet.size() );

   if ( targetSet.size() != nCases )
   {
      THROW2( DimensionMismatch, "Target set is not of same size as parameter set" );
   }
   if ( !cases.empty() && cases.size() != nCases )
   {
      THROW2( DimensionMismatch, "Case list is not of same size as parameter set" );
   }

   // Calculate sample variance of the total target set.
   double sumOfSquares = 0.0;
   double targetAvg = VectorMean( targetSet );
   for ( unsigned int i = 0; i < targetSet.size(); ++i )
   {
      double targetDev = targetSet[i] - targetAvg;
      sumOfSquares += ( targetDev * targetDev );
   }
   int degreesOfFreedom = targetSet.size() - 1;
   m_targetVariance = sumOfSquares / std::max<int>( degreesOfFreedom, 1 );
   double targetStdDev = sqrt( m_targetVariance );
   if ( targetStdDev < Tiny || targetStdDev < 1e-6*fabs( targetAvg ) )
   {
      m_targetVariance = 0.0; //to avoid "noise" modelling
   }

   // Prepare for applying setCaseList.
   m_tunePars = parSet;
   m_tuneTargets = targetSet;
   m_cases.assign( nCases,true );

   if ( ! cases.empty() && ! std::equal( m_cases.begin(), m_cases.end(), cases.begin() ) )
   {
      setCaseList( cases );
   }
}

namespace{
struct increment
{
   increment():val(0){}
   unsigned int operator()() { return val++; }
   unsigned int val;
};

// Function object wrapping the SUMlib random generator for use in std::random_shuffle
struct RNG
{
   RNG(unsigned int seed ) : m_rg(seed) { /* empty */ }
   unsigned int operator()( unsigned int N )
   {
      unsigned int val = static_cast<unsigned int>( m_rg.uniformRandom() * N );
      return val;
   }
   RandomGenerator m_rg;
};

// A local instance of the random generator, with an arbitrary seed
static RNG s_rg( 1966-12-10 );

}// anon. namespace

void ProxyCases::RNGseed( int seed )
{
   s_rg.m_rg.initialise( seed );
}


void ProxyCases::shuffle( CaseList& caseList )
{
   const size_t nTotal( caseList.size() );
   const size_t nActive( std::count(caseList.begin(), caseList.end(), true) );

   // Generate a random shuffle of index numbers
   IndexList index(nTotal);
   std::generate_n( index.begin(), nTotal, increment() );
   std::random_shuffle( index.begin(), index.end(), s_rg );
   //std::random_shuffle( index.begin(), index.end() );

   std::fill_n(caseList.begin(), nTotal,false );

   for ( unsigned int k = 0; k < nActive; ++k )
   {
      caseList[index[k]]=true;
   }
}

unsigned int ProxyCases::setCaseList( CaseList const& cases )
{
   // The current proxy builder is not valid after setting the new caselist
   delete m_builder;
   m_builder = NULL;

   const unsigned int nCases( m_cases.size() );

   // Construct the original parameter set and target set
   ParameterSet parSet;
   TargetSet targetSet;
   parSet.reserve( nCases );
   targetSet.reserve( nCases );
   unsigned int k(0), l(0);
   for ( size_t i = 0; i < nCases; ++i )
   {
      if ( m_cases[i] )
      {
         targetSet.push_back( m_tuneTargets[k] );
         parSet.push_back( m_tunePars[k] );
         ++k;
      }
      else
      {
         targetSet.push_back( m_testTargets[l] );
         parSet.push_back( m_testPars[l] );
         ++l;
      }
   }
   assert( k == m_tunePars.size() );
   assert( l == m_testPars.size() );

   // Now apply the new index list
   m_cases = cases;

   const unsigned int nActiveCases ( static_cast<unsigned int>( std::count( m_cases.begin(), m_cases.end(), true ) ) );

   m_tunePars.clear();
   m_tunePars.reserve( nActiveCases );
   m_tuneTargets.clear();
   m_tuneTargets.reserve( nActiveCases );

   m_testPars.clear();
   m_testPars.reserve( nActiveCases );
   m_testTargets.clear();
   m_testTargets.reserve( nActiveCases );

   for ( size_t i = 0; i < nCases; ++i )
   {
      if ( m_cases[i] )
      {
         m_tuneTargets.push_back( targetSet[i] );
         m_tunePars.push_back( parSet[i] );
      }
      else
      {
         m_testTargets.push_back( targetSet[i] );
         m_testPars.push_back( parSet[i] );
      }
   }
   return nActiveCases;
}

unsigned int ProxyCases::numTotalCases() const
{
   return m_tunePars.size() + m_testPars.size();
}

unsigned int ProxyCases::numTuneCases() const
{
   return m_tunePars.size();
}

unsigned int ProxyCases::caseSize() const
{
   if ( ! numTuneCases() )
   {
      THROW2( InvalidState, "Case set cannot be empty" );
   }
   return m_tunePars.front().size();
}

void ProxyCases::createProxyBuilder( VarList const& vars )
{
   if ( m_builder )
   {
      delete m_builder;
   }
   m_builder = new ProxyBuilder( m_tunePars, m_tuneTargets, vars );
}

CubicProxy *ProxyCases::createProxy( ) const
{
   if ( ! m_builder )
   {
      THROW2( InvalidState, "ProxyBuilder not initialised yet" );
   }
   return m_builder->create( );
}

CubicProxy *ProxyCases::createProxy( unsigned int varIndx ) const
{
   if ( ! m_builder )
   {
      THROW2( InvalidState, "ProxyBuilder not initialised yet" );
   }
   return m_builder->create( varIndx );
}

double ProxyCases::calculateMSE( CubicProxy const *proxy, ParameterSet const& par, TargetSet const& target )
{
   if ( par.empty() )
   {
      return 0.0;
   }

   const unsigned int N = par.size();

   // Calculate mean-squared-error of proxy response w.r.t. supplied targets
   RealVector response;
   response.reserve( par.size() );
   for ( unsigned i = 0; i < N; ++i )
   {
      response.push_back( proxy->getValue( par[i] ) );
   }
   return MeanSquaredError( target, response );
}

double ProxyCases::testMSE( CubicProxy const * proxy ) const
{
   return calculateMSE( proxy, m_testPars, m_testTargets );
}

double ProxyCases::tuneMSE( CubicProxy const * proxy ) const
{
   return calculateMSE( proxy, m_tunePars, m_tuneTargets );
}

void ProxyCases::test( CubicProxy const * proxy, unsigned int nrOfUsedVars, double& tuneRMSE,
                       double& testRMSE, double& totalRMSE, double& adjustedR2 ) const
{
   if ( proxy == NULL )
   {
      THROW2( InvalidValue, "No proxy supplied" );
   }

   // Calculate mean-squared-error of proxy response to targets
   const double mseTest = testMSE( proxy );
   const double mseTune = tuneMSE( proxy );

   // Calculate RMSE value for tune and test set
   tuneRMSE = sqrt( mseTune );
   testRMSE = sqrt( mseTest );

   // Calculate adjusted RMSE value for total set
   const unsigned int nTune = m_tunePars.size();
   const unsigned int nTest = m_testPars.size();
   assert( nTune > nrOfUsedVars ); //underdetermined systems of equations are not supported
   const unsigned int degreesOfFreedom = nTune + nTest - nrOfUsedVars - 1;
   if ( degreesOfFreedom > 0 )
   {
      const double totalMSE = ( mseTune * nTune + mseTest * nTest ) / degreesOfFreedom;
      totalRMSE = sqrt( totalMSE );
      if ( m_targetVariance < Tiny )
      {
         adjustedR2 = 1.0; //by convention because intercept as model gives perfect fit
      }
      else
      {
         adjustedR2 = 1.0 - totalMSE / m_targetVariance;
      }
   }
   else
   {
      adjustedR2 = -1.0; //adjusted R^2 is not defined here but -1 is poor enough
      totalRMSE = sqrt( 2 * std::max<double>( m_targetVariance, 1.0 ) ); //poor enough too
   }
}

unsigned int ProxyCases::getDesignMatrixRank() const
{
   unsigned int rank = 0;
   const RealVector& s = m_builder->singularValues();
   if ( ! s.empty() )
   {
      const double sTiny = s.size() * *std::max_element( s.begin(), s.end() ) * MachineEpsilon();
      for ( RealVector::const_iterator it = s.begin(); it != s.end(); ++it )
      {
         if ( *it > sTiny )
         {
            ++rank;
         }
      }
   }

   // We've now calculated the rank of SUMlib's design matrix (without the intercept column, and with the column means
   // subtracted from the other columns). We want to return the rank of the original design matrix, so we have to add 1.
   ++rank;

   return rank;
}

std::vector<double> ProxyCases::calcLeverages() const
{
   // Calculate leverage scores = diagonal of hat matrix = diagonal of U*U'
   const RealMatrix& u = m_builder->orthonormalU();
   const std::size_t n = numTuneCases();
   const std::size_t p = m_builder->baseVars().size();
   assert( u.size() == n ); // check #rows of u
   assert( u.empty() && p == 0 || u.front().size() == p ); // check #columns of u
   std::vector<double> leverages( n );
   for ( std::size_t i = 0; i < n; ++i )
   {
      leverages[i] = std::inner_product( u[i].begin(), u[i].end(), u[i].begin(), 0.0 );
      // SUMlib removes the intercept column from the design matrix, and for the remaining columns the column mean is
      // subtracted. As a consequence, SUMlib's hat matrix is different from the traditional hat matrix: when the SUMlib
      // hat matrix is subtracted from the traditional hat matrix, the result is 1/n for each cell, where n is the
      // number of cases. We compensate for that here, to let the leverages be the diagonal of the traditional hat
      // matrix. See also "Regression Diagnostics: Identifying Influential Data and Sources of Collinearity" by David A.
      // Belsley, Edwin Kuh, and Roy E. Welsch.
      leverages[i] += 1.0 / numTuneCases();
   }
   return leverages;
}

std::vector<double> ProxyCases::calcStdErrors() const
{
   // n = number of cases
   // p = number of polynomial terms excluding intercept
   const std::size_t n = m_tunePars.size() + m_testPars.size();
   const std::size_t p = m_builder->baseVars().size();

   // rss = residual sum of squares
   CubicProxy* proxy = createProxy();
   const double rss = tuneMSE( proxy ) * m_tunePars.size() + testMSE( proxy ) * m_testPars.size();
   delete proxy;

   // var = unbiased estimation of variance of the error
   // Prevent division by zero.
   const double var = rss / std::max< std::size_t >( n - p - 1, 1 );

   // a = ( Xs' * Xs )^-1 = V * ( S' * S )^-1 * V', where Xs is SUMlib's design matrix
   RealMatrix a( p, RealVector( p, 0.0 ) );
   const RealVector& s = m_builder->singularValues();
   const RealMatrix& v = m_builder->orthonormalV();
   assert( s.size() == p ); // check #rows of S
   assert( v.size() == p ); // check #rows of V
   assert( v.empty() && p == 0 || v.front().size() == p ); // check #columns of V
   const double sTiny = ! s.empty() ? s.size() * *std::max_element( s.begin(), s.end() ) * MachineEpsilon() : 0.0;
   for ( std::size_t i = 0; i < p; ++i )
   {
      for ( std::size_t j = 0; j < p; ++j )
      {
         for ( std::size_t k = 0; k < p; ++k )
         {
            const double sTrunc = std::max( s[k], sTiny );
            a[i][j] += v[i][k] * v[j][k] / ( sTrunc * sTrunc );
         }
      }
   }

   // b(i) = X1(1,i) + ... + X1(p,i), where X1 is the original design matrix without the first column
   RealVector b( p, 0.0 );
   assert( m_builder->proxyData().size() == n ); // check #rows of design matrix
   assert( m_builder->proxyData().empty() && n == 0 || m_builder->proxyData().front().size() == p ); // check #columns
   assert( m_builder->proxyMean().size() == p ); // check #rows of mean vector
   for ( std::size_t i = 0; i < p; ++i )
   {
      for ( std::size_t j = 0; j < n; ++j )
      {
         b[i] += m_builder->proxyData()[j][i] + m_builder->proxyMean()[i];
      }
   }

   // c = i' * X1 * ( Xs' * Xs )^-1 * X1' * i = b(i) * b(j) * a(i,j)
   double c = 0;
   for ( std::size_t i = 0; i < p; ++i )
   {
      for ( std::size_t j = 0; j < p; ++j )
      {
         c += b[i] * b[j] * a[i][j];
      }
   }

   // Allocate memory for standard errors. First element is for intercept, rest is for other coefficients.
   std::vector<double> stdErrors( p + 1 );

   // Calculate standard error of intercept:
   // standard_error = sqrt( var / n^2 * ( n + i' * X1 * ( Xs' * Xs )^-1 * X1' i ) ) = sqrt( var / n^2 * ( n + c ) )
   stdErrors[ 0 ] = std::sqrt( var / ( n * n ) * ( n + c ) );

   // Calculate standard errors of coefficients other than intercept:
   // standard_error(i) = sqrt( var * ( ( Xs' * Xs )^-1 )(i,i) = sqrt( var * a(i,i) )
   for ( std::size_t i = 0; i < p; ++i )
   {
      stdErrors[i + 1] = std::sqrt( var * a[i][i] );
   }

   return stdErrors;
}


} // namespace SUMlib
