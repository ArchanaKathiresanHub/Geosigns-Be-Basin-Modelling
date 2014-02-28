// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#include <assert.h>

#include "KrigingData.h"
#include "KrigingWeights.h"
#include "NumericUtils.h"

namespace SUMlib {

KrigingWeights::KrigingWeights() :
   m_indexes(),
   m_weights(),
   m_sumOfWeights( 0.0 )
{
   // empty
}

// Replace negative weights by 0 and sum the weights afterwards.
void KrigingWeights::sumPositiveWeights( )
{
   static const double eps = 1e-6;

   m_sumOfWeights = 0.0;
   m_indexes.clear();
   const unsigned int size = m_weights.size();
   for ( unsigned int i = 0; i < size; ++i )
   {
      if ( m_weights[i] > eps )
      {
         m_indexes.push_back( i );
         m_sumOfWeights += m_weights[i];
      }
      else m_weights[i] = 0.0; //weights cannot be negative!
   }
}

// MatrixVector Product for sparse v: M[][]*v[] = sum{ v[i]*M[][i] } over i only where v[i] != 0.
void KrigingWeights::economicMatrixVectorProduct( RealMatrix const& M, RealVector const& v, IndexList const& i_v, RealVector& w )
{
   assert( M.front().size() == v.size() );
   w.resize( M.size(), 0.0 );
   for ( unsigned i = 0; i < i_v.size(); ++i )
   {
      for ( unsigned j = 0; j < M.size(); ++j )
      {
         w[j] += v[i_v[i]]*M[j][i_v[i]];
      }
   }
}

void KrigingWeights::zeroWeights( unsigned int size )
{
   m_weights.resize( size, 0.0 );
   m_indexes.clear();
   m_sumOfWeights = 0.0;
}

void KrigingWeights::calcGlobalWeights( ParameterSet const& parSet, KrigingData const& kr, Parameter const& p )
{
   m_weights.resize( parSet.size(), 0.0 );
   const unsigned int m = m_weights.size();
   RealVector cov_p( m, 0.0 );
   if ( m > 0 )
   {
      for ( unsigned i = 0; i < m; ++i )
      {
         double d_i = KrigingDistance( p, parSet[i], kr.sizeOrd() ); //distance between p and base point i
         //calculate corresponding covariance
         cov_p[i] = kr.calcCovariance( d_i, kr.globalCorLength() );
      }
      MatrixVectorProduct( kr.globalInvCov(), cov_p, m_weights ); //calculate weights
   }
   sumPositiveWeights(); //calculate sum of weights and relevant indices
}

// Below is cheaper than calcGlobalKrigingWeights because:
// 1. Only distances between p and nearby (within local correlation range) base points need to be calculated.
// 2a. Only a subset of the columns (or rows) of m_localInvCov is part of the weight calculation.
// 2b. Covariance matrix = identity matrix I if m_minDistance >= m_localCorLength; then also m_localInvCov = I.
void KrigingWeights::calcLocalWeights( ParameterSet const& parSet, KrigingData const& kr, Parameter const& p )
{
   m_weights.resize( parSet.size(), 0.0 );
   const unsigned int m = m_weights.size();
   const unsigned int refIndex = kr.refIndex();
   const double eps = 1e-6;
   const double localCorLength = kr.localCorLength();
   RealVector cov_p( m_weights );
   double m_sumOfWeights = 0.0;
   if ( m > 0 )
   {
      double dRef = KrigingDistance( p, parSet[refIndex], kr.sizeOrd() ); //distance between p and reference point
      cov_p[refIndex] = kr.calcCovariance( dRef, localCorLength ); //calculate corresponding covariance
      if ( cov_p[refIndex] > eps )
      {
         m_sumOfWeights += cov_p[refIndex];
         m_indexes.push_back( refIndex );
      }
      double dNear( dRef ); //initialise distance between p and nearest base point
      double dFar( dRef ); //initialise distance between p and furthest base point
      unsigned int iNear( refIndex ); //initialise index of nearest base point relative to p
      unsigned int iFar( refIndex ); //initialise index of furthest base point relative to p
      for ( unsigned i = 0; i < m; ++i )
      {
         RealVector const& distances( kr.distances()[i] );

         if ( i != refIndex ) //already handled this base point as it is identical to the reference point
         {
            const double dRef_i = distances[refIndex]; //distance between base point i and reference point
            if ( (dRef_i > dRef - localCorLength) && (dRef_i < dRef + localCorLength) ) //can base point i lie within correlation range?
            {
               const double dNear_i = distances[iNear]; //distance between base point i and nearest base point to p
               const double dFar_i = distances[iFar]; //distance between base point i and furthest base point to p

               // Base point i must be sufficiently far from base point iFar and sufficiently close to base point iNear
               if ( (dFar_i > dFar - localCorLength) && (dNear_i < dNear + localCorLength) )
               {
                  double d_i = KrigingDistance( p, parSet[i], kr.sizeOrd() ); //distance between p and base point i
                  cov_p[i] = kr.calcCovariance( d_i, localCorLength ); //calculate corresponding covariance
                  if ( cov_p[i] > eps )
                  {
                     m_sumOfWeights += cov_p[i];
                     m_indexes.push_back( i ); //add new relevant index
                  }
                  if ( d_i < dNear )
                  {
                     dNear = d_i; //found new base point that is closer to p
                     iNear = i; //update index of nearest base point relative to p
                  }
                  if ( d_i > dFar )
                  {
                     dFar = d_i; //found new base point that is further from p
                     iFar = i; //update index of furthest base point relative to p
                  }
               }
            }
         }
      }
      if ( kr.minDistance() < localCorLength )
      {
         economicMatrixVectorProduct( kr.localInvCov(), cov_p, m_indexes, m_weights ); //calculate weights
         sumPositiveWeights( ); //recalculate sum of weights and relevant indices
      }
      else
      {
         m_weights = cov_p; //because m_localInvCov = I
      }
   }
}

} // namespace SUMlib

