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
{
   m_size = parSet.size() ? parSet.front().size() : 0;

   m_parSet = parSet;
   m_scaledTargets = targetSet;
   m_vars  = vars;

   //TargetSet targets( targetSet );

   setMonomialCode( m_vars, m_code );

   calculateProxyData( m_parSet, m_proxyData, m_proxyMean );
   m_orthonormalU = m_proxyData;

   // Calculate the monomial coefficients by applying SVD
   int stat = Proxy::calculateSVD( m_orthonormalU, m_singularValues, m_orthonormalV );

   // Statistical scaling of estimation data (allProxyData is assumed to be scaled to [-1:1])
   VectorScaleToMean( m_scaledTargets, m_targetMean );

   // Calculate the coefficients for the specified target set
   Proxy::calculateCoefficients( stat, m_orthonormalU, m_singularValues, m_orthonormalV, m_scaledTargets, m_coefficients );
}


ProxyBuilder::~ProxyBuilder()
{
   // empty
}


CubicProxy *ProxyBuilder::create() const
{
   CubicProxy *proxy = new CubicProxy;

   proxy->initialise( m_size, m_vars, m_code, m_proxyMean, m_targetMean, m_coefficients );

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
   unsigned int varPos = std::distance( vars.begin(), varIt );

   if ( varIt != vars.end() )
   {
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
      varPos = std::distance( vars.begin(), varIt );
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
   Proxy::calculateCoefficients( 0, orthonormalU(), singularValues(), orthonormalV(), c, p );
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
      int stat = Proxy::calculateSVD( orthU, singVal, orthV );
      Proxy::calculateCoefficients( stat, orthU, singVal, orthV, targets(), coef );
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

unsigned int ProxyBuilder::size() const
{
   return m_size;
}

} // namespace SUMlib
