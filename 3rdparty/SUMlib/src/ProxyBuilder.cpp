// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#include <algorithm>
#include <cassert>
#include <vector>
#include <numeric>

#include "BaseTypes.h"
#include "CubicProxy.h"
#include "EliminationCriteria.h"
#include "DataStructureUtils.h"
#include "NumericUtils.h"
#include "ProxyBuilder.h"

using std::vector;

namespace SUMlib {

ProxyBuilder::ProxyBuilder(
      ParameterSet const&  parSet,
      TargetSet const&     targetSet,
      IndexList const&     vars
      )
   : m_size( parSet.size() ? parSet.front().size() : 0 ),
     m_parSet( parSet ),
     m_scaledTargets( targetSet ),
     m_vars( vars )
{
   // Statistical scaling of estimation data (allProxyData is assumed to be scaled to [-1:1])
   VectorScaleToMean( m_scaledTargets, m_targetMean );
   fit();
}

ProxyBuilder::~ProxyBuilder()
{
   // empty
}

unsigned int ProxyBuilder::eliminate( EliminationCriterion& criterion )
{
   unsigned int eliminated = 0;

   while ( boost::optional<unsigned int> idx = criterion( *this ) ) {
      m_vars.erase(m_vars.begin() + *idx);
      fit();
      ++eliminated;
   }
   return eliminated;
}

CubicProxy *ProxyBuilder::create() const
{
   CubicProxy *proxy = new CubicProxy;

   proxy->initialise( m_size, m_vars, m_code, m_proxyMean, m_targetMean, m_coefficients, calcStdErrors(), calcDesignMatrixRank() );

   return proxy;
}

CubicProxy *ProxyBuilder::create( unsigned int varIndx ) const
{
   CubicProxy *proxy = new CubicProxy;

   VarList           vars         = m_vars;
   vector<IndexList> code         = m_code;
   RealVector        proxyMean    = m_proxyMean;
   RealVector        coefficients;

   VarList::iterator varIt = std::find( vars.begin(), vars.end(), varIndx );

   if ( varIt != vars.end() )
   {
      const unsigned int varPos = std::distance( vars.begin(), varIt );
      // Reduce model by removing column that corresponds to the supplied var
      vars.erase( varIt );
      const unsigned int nrOfVars = vars.size();

      // Construct coefficients corresponding to the new vars
      coefficients = calcReducedCoeff( nrOfVars, varPos );

      // Update proxy members: code and proxyMean
      for ( unsigned int i = 0; i <= nrOfVars; ++i )
      {
         if ( i > varPos )
         {
            code[i-1] = code[i];
            proxyMean[i-1] = proxyMean[i];
         }
      }
      code.resize( nrOfVars );
      proxyMean.resize( nrOfVars );
   }
   else
   {
      // Augment model by adding column that corresponds to the supplied var
      IndexListInsert( vars, varIndx );
      const unsigned int nrOfVars = vars.size();
      IndexList parIndx;
      RealVector col; //column to be added to the model
      double colMean = addVarColumn( varIndx, parIndx, col );

      // Construct coefficients corresponding to the new vars
      RealVector basisCoef;
      double addedCoef;
      calcAugmentedCoeff( col, basisCoef, addedCoef );

      // Update proxy members: code, proxyMean and coefficients
      code.resize( nrOfVars );
      proxyMean.resize( nrOfVars );
      coefficients.resize( nrOfVars );
      varIt = std::find( vars.begin(), vars.end(), varIndx );
      const int varPos = std::distance( vars.begin(), varIt );
      for ( int i = nrOfVars - 1; i >= 0 ; --i )
      {
         if ( i > varPos )
         {
            coefficients[i] = basisCoef[i-1];
            code[i] = code[i-1];
            proxyMean[i] = proxyMean[i-1];
         }
         else if ( i == varPos )
         {
            coefficients[i] = addedCoef;
            code[i] = parIndx;
            proxyMean[i] = colMean;
         }
         else
         {
            coefficients[i] = basisCoef[i];
         }
      }
   }

   proxy->initialise( size(), vars, code, proxyMean, targetMean(), coefficients );
   return proxy;
}

void ProxyBuilder::fit()
{
   m_code.clear();
   m_proxyMean.clear();
   m_proxyData.clear();
   m_singularValues.clear();
   m_orthonormalV.clear();

   setMonomialCode( m_vars, m_code );

   calculateProxyData( m_parSet, m_proxyData, m_proxyMean );
   m_orthonormalU = m_proxyData;

   // Calculate the monomial coefficients by applying SVD
   int stat = calculateSVD( m_orthonormalU, m_singularValues, m_orthonormalV );

   // Calculate the coefficients for the specified target set
   CubicProxy::calculateCoefficients( stat, m_orthonormalU, m_singularValues, m_orthonormalV, m_scaledTargets, m_coefficients );
}

void ProxyBuilder::setMonomialCode( VarList const& vars, std::vector<IndexList> &code ) const
{
   // Generate the monomial coding matrix
   CubicProxy::monomial_code( size(), vars, code );
}

void ProxyBuilder::calculateProxyData( ParameterSet const& parSet, ProxyData& proxyData, RealVector &mean ) const
{
   proxyData.reserve(parSet.size());
   for ( ParameterSet::const_iterator pp = parSet.begin(); pp != parSet.end(); ++pp )
   {
      RealVector data;
      data.reserve(m_code.size());
      for (vector<IndexList>::const_iterator ii = m_code.begin(); ii != m_code.end(); ++ii )
      {
         data.push_back(CubicProxy::monomial( *ii, *pp ));
      }
      proxyData.push_back(data);
   }
   // Statistical scaling of estimation data (allProxyData is assumed to be scaled to [-1:1])
   MatrixScaleToMean( proxyData, mean );
}

double ProxyBuilder::addVarColumn( unsigned int varIndx, IndexList& parIndx, RealVector& v ) const
{
   VarList singleVar( 1, varIndx );
   vector<IndexList> singleCode( 1 );
   CubicProxy::monomial_code( size(), singleVar, singleCode );
   parIndx = singleCode[0];
   for ( unsigned int i = 0; i < parSet().size(); ++i )
   {
      v.push_back( CubicProxy::monomial( parIndx, parSet()[i] ) );
   }
   double vMean;
   VectorScaleToMean( v, vMean );
   return vMean;
}

void ProxyBuilder::calcAugmentedCoeff( RealVector const& c, RealVector& r, double& s ) const
{
   const unsigned int nPar = c.size();
   const unsigned int nVar = proxyData().front().size();
   r = coefficients();
   s = 0.0;

   // column c can be written as proxyData*p + q, where p is the projection vector
   // and q is the innovation vector such that transpose(proxyData)*q = 0.
   RealVector p( nVar ), q( nPar );
   CubicProxy::calculateCoefficients( 0, orthonormalU(), singularValues(), orthonormalV(), c, p );
   if ( nVar > 0 )
   {
      MatrixVectorProduct( proxyData(), p, q );
   }
   assert( q.size() == nPar );
   for ( unsigned int i = 0; i < nPar; ++i )
   {
      q[i] = c[i] - q[i];
   }
   double qNorm = VectorL2Norm( q );
   double cNorm = VectorL2Norm( c );
   if ( qNorm > 1e-3*cNorm && cNorm > 1e-3 )
   {
      s = std::inner_product( q.begin(), q.end(), targets().begin(), 0.0 )/(qNorm*qNorm);
      for ( unsigned int i = 0; i < nVar; ++i )
      {
         r[i] -= s*p[i];
      }
   }
}

RealVector ProxyBuilder::calcReducedCoeff( unsigned int nVars, unsigned int pos ) const
{
   RealVector coef( nVars );
   unsigned int i, j;
   for ( i = 0; i <= nVars; ++i )
   {
      if ( i < pos )
      {
         coef[i] = coefficients()[i];
      }
      else if ( i > pos )
      {
         coef[i-1] = coefficients()[i];
      }
   }

   // Test whether transpose(proxyData)*proxyData is (nearly) singular.
   double minSV = *(std::min_element( singularValues().begin(), singularValues().end() ) );
   double maxSV = *(std::max_element( singularValues().begin(), singularValues().end() ) );
   bool singular = ( ( minSV < nVars*MachineEpsilon()*maxSV ) || ( maxSV < MachineEpsilon() ) );

   if ( singular ) //should not happen by design but just in case...
   {
      RealMatrix orthU = proxyData();
      assert( orthU.front().size() == nVars + 1 );
      const unsigned int nRows = orthU.size();
      for ( i = 0; i < nRows; ++i )
      {
         for ( j = pos; j < nVars; ++j )
         {
            orthU[i][j] = orthU[i][j+1];
         }
         orthU[i].resize( nVars );
      }
      RealMatrix orthV;
      RealVector singVal;
      int stat = calculateSVD( orthU, singVal, orthV );
      CubicProxy::calculateCoefficients( stat, orthU, singVal, orthV, targets(), coef );
   }
   else
   {
      double qTq = 0.0;
      RealVector vec( nVars+1 );
      for ( j = 0; j <= nVars; ++j )
      {
         vec[j] = orthonormalV()[pos][j]/singularValues()[j];
         qTq += vec[j]*vec[j];
      }
      qTq = 1.0/qTq; //the denominator is strictly positive!
      RealVector p( nVars );
      for ( i = 0; i <= nVars; ++i )
      {
         if ( i != pos )
         {
            unsigned int k = (i < pos)? i : i-1;
            for ( j = 0; j <= nVars; ++j )
            {
               p[k] += vec[j] * orthonormalV()[i][j]/singularValues()[j];
            }
            p[k] *= -qTq;
         }
      }
      double s = coefficients()[pos];
      for ( i = 0; i < nVars; ++i )
      {
         coef[i] += s*p[i];
      }
   }

   return coef;
}

double ProxyBuilder::mse() const
{
   const size_t N = m_parSet.size();

   if (!N) return 0.0;

   double s = 0.0;
   for ( size_t i=0; i < N; ++i )
   {
      double value = 0;
      for ( size_t j = 0; j < m_code.size(); ++j )
      {
         value += m_coefficients[j] * ( CubicProxy::monomial( m_code[j], m_parSet[i] ) - m_proxyMean[j] );
      }
      double error = value - m_scaledTargets[i];
      s += error * error;
   }
   return s / N;
}

std::vector<double> ProxyBuilder::calcStdErrors() const
{
   // n = number of cases
   // p = number of polynomial terms excluding intercept
   const std::size_t n = proxyData().size();
   const std::size_t p = baseVars().size();

   // rss = residual sum of squares
   const double rss = mse() * n;

   // var = unbiased estimation of variance of the error
   // Prevent division by zero.
   const double var = rss / std::max< std::size_t >( n - p - 1, 1 );

   // a = ( Xs' * Xs )^-1 = V * ( S' * S )^-1 * V', where Xs is SUMlib's design matrix
   RealMatrix a( p, RealVector( p, 0.0 ) );
   const RealVector& s = singularValues();
   const RealMatrix& v = orthonormalV();
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
   assert( proxyData().empty() && n == 0 || proxyData().front().size() == p ); // check #columns
   assert( proxyMean().size() == p ); // check #rows of mean vector
   for ( std::size_t i = 0; i < p; ++i )
   {
      for ( std::size_t j = 0; j < n; ++j )
      {
         b[i] += proxyData()[j][i] + proxyMean()[i];
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

unsigned int ProxyBuilder::calcDesignMatrixRank() const
{
   unsigned int rank = 0;
   const RealVector& s = singularValues();
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

unsigned int ProxyBuilder::size() const
{
   return m_size;
}

} // namespace SUMlib
