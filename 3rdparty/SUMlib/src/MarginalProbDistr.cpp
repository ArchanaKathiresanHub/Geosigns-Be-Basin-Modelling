// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#include <cassert>
#include <cmath>
#include <vector>

#include "Exception.h"
#include "Case.h"
#include "MarginalProbDistr.h"
#include "ParameterPdf.h"

using std::vector;

namespace SUMlib {

MarginalProbDistr::MarginalProbDistr(
      const vector<double>& base,
      const vector<vector<double> >& cov,
      const vector<double>& pdfMin,
      const vector<double>& pdfMax,
      const vector<Type>& types, /* vector<Type>() */
      unsigned int logNormalInfo /* 1 */
      )
{
   assert( base.size() == cov.size() );
   assert( base.size() == pdfMax.size() );
   assert( pdfMin.size() == pdfMax.size() );
   initialise( base, cov, pdfMin, pdfMax, types, logNormalInfo );
}

MarginalProbDistr::MarginalProbDistr(
      ParameterPdf const& pdf,
      const vector<Type>& types, /* vector<Type>() */
      unsigned int logNormalInfo /* 1 */
      )
{
   assert( pdf.covariance().size() == pdf.sizeCon() );
   m_disWeights = pdf.disWeights();
   assert( m_disWeights.size() == pdf.sizeDis() );

   vector<double> pdfMin = pdf.lowestNonFrozenOrdParams();
   vector<double> pdfMax = pdf.highestNonFrozenOrdParams();
   initialise( pdf.scaledOrdinalBase(), pdf.covariance(), pdfMin, pdfMax, types, logNormalInfo );
}

void MarginalProbDistr::initialise(
      const vector<double>& base,
      const vector<vector<double> >& cov,
      const vector<double>& pdfMin,
      const vector<double>& pdfMax,
      const vector<Type>& types, /* vector<Type>() */
      unsigned int logNormalInfo /* 1 */
      )
{
   if( ! types.empty() && types.size() != cov.size() )
   {
      THROW2( DimensionMismatch, "array of types has unexpected size" );
   }

   // Check that the covariance matrix is OK
   ParameterPdf::checkCovarianceMatrix ( cov );

   m_base = base;
   m_mean = m_base;
   m_mean.resize( cov.size() ); //m_mean is continuous part of m_base
   m_pdfMin = pdfMin;
   m_pdfMax = pdfMax;
   m_sampleMin = m_pdfMin;
   m_sampleMax = m_pdfMax;
   m_types = types.empty() ? vector<Type>( cov.size(), Uniform ) : types;
   m_stdev.resize( cov.size() );
   for ( size_t i = 0; i < cov.size(); ++i )
      m_stdev[i] = sqrt(cov[i][i]);
   m_mu.resize( cov.size() );
   m_sigma.resize( cov.size() );

   if ( logNormalInfo == 0 )
   {
      m_mu = m_mean;
      m_sigma = m_stdev;
   }
   else if ( logNormalInfo == 1 )
   {
      initLogNormal1( m_mean, m_stdev, m_pdfMin, m_mu, m_sigma );
   }
   else
   {
      initLogNormal2( m_mean, m_stdev, m_pdfMin, m_mu, m_sigma );
   }
}

MarginalProbDistr::~MarginalProbDistr()
{
}

void MarginalProbDistr::setTypes( vector<MarginalProbDistr::Type> const& types )
{
   m_types = types;
}

void MarginalProbDistr::setSamplingBounds( const std::vector<double>& min, const std::vector<double>& max )
{
   if ( min.size() != max.size() )
   {
      THROW2( DimensionMismatch, "Minimum values list and maximum values list must have equal size" );
   }
   if ( m_sampleMin.size() != min.size() )
   {
      THROW2( DimensionMismatch, "Boundary values lists do not match the size of the parameters in the distribution" );
   }
   m_sampleMin = min;
   m_sampleMax = max;
}

void MarginalProbDistr::initLogNormal2(
      const vector<double>& median, const vector<double>& stdev,
      const vector<double>& low, vector<double>& mu, std::vector<double>& sigma )
{
   for ( size_t i = 0; i < median.size(); ++i )
   {
      // stdev == 0 means a fixed parameter element
      if ( stdev[i] > sqrt( CloseToZero ) )
      {
         double M = median[i] - low[i]; // translation to a positve median
         if ( M > CloseToZero )
         {
            mu[i] = log(M);
            double L = stdev[i]/M; // temporary constant
            sigma[i] = sqrt( log( 0.5 + 0.5*sqrt(1.0+4*L*L) ) );
         }
         else
         {
            mu[i] = log(CloseToZero);
            sigma[i] = sqrt( log(stdev[i]) - log(CloseToZero) );
         }
      }
   }
}

void MarginalProbDistr::initLogNormal1(
            const vector<double>& mode, const vector<double>& stdev,
            const vector<double>& low, vector<double>& mu, vector<double>& sigma )
{
   for ( size_t i = 0; i < mode.size(); ++i )
   {
      // stdev == 0 means a fixed parameter element
      if ( stdev[i] > sqrt(CloseToZero) )
      {
         double M = mode[i] - low[i]; // translation to a positve mode
         if ( M > CloseToZero )
         {
            double L = stdev[i]/M; // temporary constant
            double x = 1.0 + L/2; // initial guess for x = exp{sigma^2}
            // Solve x^4 - x^3 - L^2 = 0
            for ( size_t j = 0; j < 5; ++j ) // 5 Newton-Raphson iterations suffice
            {
               x = x - (x*x - x - (L/x)*(L/x))/(4*x - 3);
            }
            mu[i] = log(M*x);
            sigma[i] = sqrt(log(x));
         }
         else
         {
            mu[i] = 0.5 * (log(stdev[i]) + log(CloseToZero));
            sigma[i] = sqrt( 0.5 * (log(stdev[i]) - log(CloseToZero)) );
         }
      }
   }
}

double MarginalProbDistr::calcLogMarginalNormal( double p, double mean, double stdev ) const
{
   double tmp = (p - mean)/stdev;
   return -tmp*tmp/2;
}

double MarginalProbDistr::calcLogMarginalLogNormal( double p, double low, double mu, double sigma ) const
{
   double p_tr = p - low; // translation as in initLogNormal!!!
   if ( p_tr > CloseToZero )
   {
      double tmp = (log(p_tr) - mu)/sigma;
      return -log(p_tr) - tmp*tmp/2;
   }
   else return MinInf; // minus infinity at zero probability
}

double MarginalProbDistr::calcLogMarginalTriangular( double p, double base, double low, double high ) const
{
   if ( (p - low < CloseToZero) || (high - p < CloseToZero) )
   {
      return MinInf; // minus infinity at zero probability
   }
   else if ( p < base )
   {
      return log((p - low)/(base - low));
   }
   else
   {
      return log((high - p)/(high - base));
   }
}

double MarginalProbDistr::calcLogPriorProb( Parameter const& p ) const
{
   assert( p.size() == m_base.size() );

   // Accumulate the contributions to the log of the probability for all
   // parameter elements
   double logProb = 0.0;

   // Loop over the continuous parameters
   for ( size_t i = 0; i < m_mean.size(); ++i )
   {
      // The contribution by parameter element i
      double val = 0.0;

      // for a fixed parameter element (m_stdev is zero), the value should not
      // influence the decision of accepting the sample, hence no contribution to
      // log probability.
      if ( m_stdev[i] > CloseToZero )
      {
         switch ( m_types[i] )
         {
            case LogNormal :
               val = calcLogMarginalLogNormal( p[i], m_pdfMin[i], m_mu[i], m_sigma[i] );
               break;
            case Normal :
               val = calcLogMarginalNormal( p[i], m_base[i], m_stdev[i] );
               break;
            case Triangular :
               val = calcLogMarginalTriangular( p[i], m_base[i], m_pdfMin[i], m_pdfMax[i] );
               break;
            case Uniform :
               // Uniform distribution adds log(1) (i.e. zero) to logProb
               break;
            default:
               THROW2( InvalidValue, "Unknown marginal distribution type" );
               break;
         }
      }
      logProb += val;
   }

   calcLogDisWeights( p, true, logProb );

   return logProb;
}

double MarginalProbDistr::correctForDisBounds( Parameter const& p ) const
{
   assert( p.size() == m_base.size() );

   double logCorr = 0.0;
   calcLogDisWeights( p, false, logCorr );

   return logCorr;
}

void MarginalProbDistr::calcLogDisWeights( Parameter const& p, bool prior, double& logProb ) const
{
   // Loop over the discrete parameters   
   for ( size_t i = m_mean.size(); i < p.size(); ++i )
   {
      // First determine boundary indices i1 and i2 that correspond to the sampling bounds
      size_t disIdx = i - m_mean.size();
      size_t nW = m_disWeights[disIdx].size() - 1;
      assert( nW > 0 );
      double totRange = m_pdfMax[i] - m_pdfMin[i];
      assert( totRange > 0.0 );
      size_t i1 = int( ( m_sampleMin[i] - m_pdfMin[i] ) * nW / totRange + 0.5 );
      size_t i2 = int( ( m_sampleMax[i] - m_pdfMin[i] ) * nW / totRange + 0.5 );
      if ( i2 > i1 )
      {
         vector<double> weights( i2 - i1 + 1 );
         for ( size_t j = 0; j < weights.size(); ++j )
         {
            // Only consider unit weights if prior = false
            weights[j] = prior ? m_disWeights[disIdx][i1 + j] : 1.0;
         }
         // Now define relative position of p[i] between 0 and 1, and update logProb
         double sampleRange = m_sampleMax[i] - m_sampleMin[i];
         assert( sampleRange > 0.0 );
         double rel_p = ( p[i] - m_sampleMin[i] ) / sampleRange;
         logProb += calcLogWeight( rel_p, weights );
      }
      // else nothing: p[i] is effectively frozen -> logProb is also frozen and cancels out
   }
}
} // namespace SUMlib
