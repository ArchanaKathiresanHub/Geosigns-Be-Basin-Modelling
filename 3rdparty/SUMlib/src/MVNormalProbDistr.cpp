// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#include <cmath>
#include <cassert>

#include "Exception.h"
#include "MVNormalProbDistr.h"
#include "NumericUtils.h"
#include "ParameterPdf.h"
#include "RandomGenerator.h"

namespace SUMlib {

/**
 *  n dimensional Normal probability distribution
 *  with mean m and correlationmatrix c
 *
 *  p(x;m,c) = [(2.pi)^n det(c)]^(-0.5) exp[ -0.5 tr(x-m) . inv(c) . (x-m) ]
 *
 *  Stores
 *     m_cormat == c
 *     m_mean == m
 *
 *  Calculates (on initialization)
 *    m_a == a
 *    m_w == w
 *
 *  such that
 *    c = a.w.tr(a)
 *    inv(c) = a.(1/w).tr(a)
 *
 *  then we have
 *       x = A.sqrt(w).u   in nrg(0,c)   if u in nrg(0,I)
 *
 */

MVNormalProbDistr::MVNormalProbDistr( )
{
   // empty
}

MVNormalProbDistr::MVNormalProbDistr(
      const std::vector<double>& mean,
      const std::vector<std::vector<double> >& cormat,
      const std::vector<double>& min,
      const std::vector<double>& max )
{
   assert( mean.size() == cormat.size() );
   assert( mean.size() == max.size() );
   assert( min.size() == max.size() );
   initialise( mean, cormat, min, max );
}

MVNormalProbDistr::MVNormalProbDistr( ParameterPdf const& pdf )
{
   assert( pdf.covariance().size() == pdf.sizeCon() );
   m_disWeights = pdf.disWeights();
   assert( m_disWeights.size() == pdf.sizeDis() );
   initialise( pdf.scaledOrdinalBase(), pdf.covariance(), pdf.low().ordinalPart(), pdf.high().ordinalPart() );
}

MVNormalProbDistr::~MVNormalProbDistr( )
{
   // empty
}

/**
 *  Initialise the distribution by specifying the untruncated mean and
 *  variance matrix.
 *  The initialisation calculates m_a and m_w from the variance matrix.
 */
void MVNormalProbDistr::initialise(
      const std::vector<double>& base,
      const std::vector<std::vector<double> >& cormat,
      const std::vector<double>& min,
      const std::vector<double>& max )
{
   m_min = min;
   m_max = max;
   m_acceptedMin = m_min;
   m_acceptedMax = m_max;
   m_base = base;
   m_mean = m_base;
   m_mean.resize( cormat.size() ); //m_mean is continuous part of m_base

   // Check that the covariance matrix is OK
   ParameterPdf::checkCovarianceMatrix ( cormat );

   // Prepare for singular value decomposition of m_cormat
   // Clear the storage for SVD matrices m_a and m_w
   m_a.clear();
   m_a = cormat;

   m_w.clear();
   m_w.resize( cormat.size(), 0 );

   std::vector<std::vector<double> > v( cormat.size(), m_w ); // m_w is filled with zeroes

   // Use singular value decomposition: C = U . W . tr(V) with
   // C symmetric and positive definite
   // U.tr(U) = I       so inv(U) = tr(U)
   // V.tr(V) = I
   // U == V if diagonal elements of W are nonzero
   // W is diagonal

   // m_a -> U  ,  m_w -> w  ,  v -> v
   if ( !m_a.empty() && svdcmp( m_a, m_w, v ) != 0 )
   {
      throw "Error";
   }

   // Check the results
   check( v, cormat );
}

// Check singular value decomposition:
//      C == U . W . tr(V)
//      U == V

void MVNormalProbDistr::check(
      const std::vector<std::vector<double> >& v,
      const std::vector<std::vector<double> >& cormat ) const
{
   std::vector<std::vector<double> > res(cormat.size());

   for ( size_t i = 0; i < cormat.size(); ++i )
   {
      res[i].resize( cormat.size() );
      for ( size_t j = 0; j < cormat.size(); ++j )
      {
         res[i][j] = 0;
         for ( size_t k = 0; k < cormat.size(); ++k )
            res[i][j] += m_w[k] * m_a[i][k] * v[j][k];

         // Check C == U . W . tr(V)
         double tmp = ::fabs( res[i][j] - cormat[i][j] );
         if ( tmp > 1e-6 )
            throw "1. Singular value decomposition on variance matrix failed";

         // Check U == V
         tmp = 0;
         // No need to check if W[j][j] = 0
         if (m_w[j] > 1e-9)
            tmp = ::fabs( m_a[i][j] - v[i][j] );
         if ( tmp > 1e-6 )
            throw "2. Singular value decomposition on variance matrix failed";
      }
   }

   // Check m_a . (1/w) . tr(m_a) == inv(c)
   bool wzero = false;
   for ( size_t i = 0; i < cormat.size(); ++i )
   {
      for ( size_t j = 0; j < cormat.size(); ++j )
      {
         res[i][j] = 0;
         for ( size_t k = 0; k < cormat.size(); ++k )
            if (m_w[k] > 1e-9)
               res[i][j] += m_a[i][k] * m_a[j][k] / m_w[k];
            else
               wzero = true;
      }
   }
   if (!wzero)
   {
      for ( size_t i = 0; i < cormat.size(); ++i )
      {
         for ( size_t j = 0; j < cormat.size(); ++j )
         {
            double tmpij = 0;
            for ( size_t k = 0; k < cormat.size(); ++k )
               tmpij += res[i][k] * cormat[k][j];

               // This should be the unity matrix
            if ( ::fabs( tmpij - ((i==j)?1:0) ) > 1e-3 )
               throw "3. Singular value decomposition on variance matrix failed";
         }
      }
   }
}

void MVNormalProbDistr::setBounds( RealVector const& min, RealVector const& max )
{
   if ( min.size() != max.size() )
   {
      THROW2( DimensionMismatch, "Minimum values list and maximum values list must have equal size" );
   }
   if ( size() != min.size() )
   {
      THROW2( DimensionMismatch, "Boundary values lists do not match the size of the parameters in the distribution" );
   }
   m_acceptedMin = min;
   m_acceptedMax = max;
}

/**
 *  Sample from a Gaussian distribution. The number of samples equals the size
 *  of the vector p which receives the sample values. Sample values outside the
 *  given bounds are not stored, but are counted in the total number of
 *  needed draws. The total of needed draws to create the sample is returned.
 */

int MVNormalProbDistr::priorSample( RandomGenerator& rg, std::vector<double>& p ) const
{
   assert( p.size() == size() );

   // Counter for number of total draws
   int icount = 0;
   const int maxNbOfTotalDraws = 100*sizeCon();

   // Get a sample from the normal distribution N(mean,variance)
   for ( size_t i = 0; i < sizeCon(); ++i )
   {
      double variance = 0.0;
      for ( size_t j = 0; j < sizeCon(); ++j )
         variance += m_a[i][j]*m_a[i][j]*m_w[j];
      bool inbounds = false;
      while ( ! inbounds && ! ( icount > maxNbOfTotalDraws ) )
      {
         p[i] = m_mean[i] + sqrt( variance ) * rg.normalRandom();
         ++icount;

         // Check the bounds
         if ( ! ( p[i] < m_acceptedMin[i] ) && ! ( p[i] > m_acceptedMax[i] ) )
         {
            inbounds = true;
         }
      }
      if ( ! inbounds )
      {
         // Simple uniform sampling now to at least get a random value
         p[i] = uniformSample( rg, i );
      }
   }

   // For convenience, the discrete parameters are sampled uniformly
   for ( size_t i = sizeCon(); i < size(); ++i )
   {
      p[i] = uniformSample( rg, i );
   }
   icount += ( size() - sizeCon() );

   return icount;
}

int MVNormalProbDistr::noPriorSample( RandomGenerator& rg, std::vector<double>& p ) const
{
   assert( p.size() == size() );

   // Get a sample from a Uniform PDF
   for ( size_t i = 0; i < p.size(); ++i )
   {
      p[i] = uniformSample( rg, i );
   }

   return p.size();
}

double MVNormalProbDistr::uniformSample( RandomGenerator& rg, size_t k ) const
{
   double sample = m_acceptedMin[k] + ( m_acceptedMax[k] - m_acceptedMin[k] ) * rg.uniformRandom();

   return sample;
}

/**
 *  Sample the distribution. The number of samples equals the size of the
 *  vector p which receives the sample values. Sample values outside the
 *  given bounds are not stored, but are counted in the total number of
 *  needed draws. The total of needed draws to create the sample is returned.
 */

int MVNormalProbDistr::sample( RandomGenerator& rg,
                              std::vector<std::vector<double> >& p, bool usePrior ) const
{
   int count = 0;
   for ( size_t i = 0; i < p.size(); ++i )
   {
      if ( usePrior )
      {
         count += priorSample( rg, p[i] );
      }
      else
      {
         count += noPriorSample( rg, p[i] );
      }
   }
   return count;
}

/**
 *  Given the sample p, determine the logarithm of the
 *  probability density function up to a constant.
 *
 *  logLh(x) = -0.5 * chi2
 *           = -0.5 tr(p-m) inv(C) (p-m)     with m = 0 here
 *           = -0.5 tr(p-m) A . (1/w) . tr(A) (p-m)
 */

double MVNormalProbDistr::calcLogPriorProb( Parameter const& p ) const
{
   double lh = 0.0;

   if ( p.size() != size() )
   {
      THROW2( DimensionMismatch, "Sample p does not match size of MVNormalProbDistr" );
   }

   // Loop over the continuous parameters
   for ( size_t i = 0; i < sizeCon(); ++i )
   {
      double ui = 0;
      for ( size_t j = 0; j < sizeCon(); ++j )
         ui += m_a[j][i] * ( p[j] - m_mean[j] ) / sqrt( m_w[i] );

      lh += ui * ui;
   }
   lh *= -0.5;

   // Loop over the discrete parameters
   for ( size_t i = sizeCon(); i < size(); ++i )
   {
      // Define relative position of p[i] between 0 and 1
      size_t disIdx = i - sizeCon();
      double range = m_max[i] - m_min[i];
      assert( range > 0.0 );
      double rel_p = ( p[i] - m_min[i] ) / range;

      // Update lh
      std::vector<double> weights = m_disWeights[disIdx];
      lh += calcLogWeight( rel_p, weights );
   }

   return lh;
}

} // namespace SUMlib
