// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

#include "BaseTypes.h"
#include "McmcProxy.h"
#include "McmcStatistics.h"
#include "NumericUtils.h"
#include "SerializerUtils.h"

using std::vector;
using std::ostream;
using std::endl;

namespace SUMlib {

McmcStatistics::McmcStatistics() :
   m_chi2(0.0),
   m_gof(0.0),
   m_chi2Red(0.0),
   m_gofRed(0.0),
   m_numDegFreedom(0)
{
   // empty
}

McmcStatistics::~McmcStatistics()
{
   // empty
}

void McmcStatistics::update(
      ParameterSet const&                 pSample,
      vector<TargetSet> const&            ySample,
      unsigned int                        nbDegFreedom,
      double                              stdDevFactor,
      std::vector<McmcProxy*> const&      proxies,
      std::vector< unsigned int > const&  catIndexOfSample )
{
   // Calculate new sample parameter average
   // boundary influence on the distribution is given by m_mlMu[k] := m_pSampleAvg[k]
   CalcAverages( pSample, m_pSampleAvg );

   // Calculate new sample parameter variancematrix
   // boundary influence on the distribution is given by m_mlSd[k] := sqrt(m_pSampleCovMat[k][k])
   CalcCovariances( pSample, m_pSampleAvg, m_pSampleCovMat );

   // Calulate the averages per categorical combination
   calcCatAverages( pSample, catIndexOfSample );

   // Calculate the convariance matricec per categorical combination
   calcCatCovariances( pSample, catIndexOfSample );

   // Calculate chi2 characteristics
   calcChi2Distribution( ySample, proxies, nbDegFreedom, stdDevFactor );

   // Average of simulated results. Since the results are expected to
   // be normally distributed N(mu,var), the averages over the sample
   // also have a normal distribution, N(mu,var/samplesize)
   CalcAverages( ySample, m_ySampleAvg );

   // Scale averaged results to N(0,1) distribution using
   // the measured value and measurement standard deviation
   m_scaledDev.resize( m_ySampleAvg.size(), 0.0 );
   for ( size_t i = 0; i < m_ySampleAvg.size(); ++i )
   {
      if ( proxies[i]->isUsed() )
      {
         m_scaledDev[i] = proxies[i]->getScaledError( m_ySampleAvg[i] )/stdDevFactor;
      }
   }
}

double McmcStatistics::calcGoF( double chi2, unsigned int numDegFreedom )
{
   double gof;

   // The goodnessOfFit (now independent of chi2) should be at least 50%
   gammp( 0.5*numDegFreedom, 0.5*chi2*numDegFreedom, &gof);

   return MapGOF(1 - gof);
}

void McmcStatistics::calcChi2Distribution(
      vector<TargetSet> const&         ySample,
      std::vector<McmcProxy*> const&   proxies,
      unsigned int                     numDegFreedom,
      double                           stdDevFactor )
{
   m_numDegFreedom = numDegFreedom;

   if ( m_numDegFreedom > 0 )
   {
      // Normalised chi2 expected to be 1. This does not take
      // an existing stdDevFactor into account.
      m_chi2 = SumChiSquared( ySample, proxies ) / m_numDegFreedom;

      // Use supplied or chi2-based factor
      double factor = stdDevFactor > 0 ? stdDevFactor : sqrt(getChi2());

      // Chi2 with the factor taken into account (normally 1)
      m_chi2Red = m_chi2 / (factor*factor);

      // Calculate the goodness of fit from the given chi2
      m_gof = calcGoF(m_chi2, m_numDegFreedom );
      m_gofRed = calcGoF(m_chi2Red, m_numDegFreedom );
   }
   else
   {
      m_chi2    = 1.0;
      m_chi2Red = 1.0;
      m_gof     = 100;
      m_gofRed  = 100;
   }
}

ostream& McmcStatistics::print( ostream& out, double stdDevFactor, vector<McmcProxy*> const& proxies ) const
{
   const unsigned int size( m_pSampleAvg.size());
   for ( size_t i = 0; i < size; ++i )
   {
      out << "avg_p[" << i << "]" << m_pSampleAvg[i] << " ~ " << sqrt(m_pSampleCovMat[i][i]) << endl;
   }

   for ( size_t i = 0; i < size; ++i )
   {
      out << "varmat_p[" << i << "]";
      for ( size_t j = 0; j < size; ++j )
      {
        out <<" "<< m_pSampleCovMat[i][j];
      }
      out << endl;
   }

   for ( size_t i = 0; i < size; ++i )
   {
      out << "lik[" << i <<"]"<< m_pSampleAvg[i] << " " << sqrt(m_pSampleCovMat[i][i]) << endl;
   }

   const unsigned int num ( m_ySampleAvg.size() );
   const double sd( 1/sqrt(1.0*num) );
   for ( size_t i = 0; i < num; ++i )
   {
      out <<"avg y["<<i<<"]=" << m_ySampleAvg[i] << " ~ " << sd;
      if ( proxies[i]->isUsed() )
      {
         out << " from N(0,1)" << endl;
      }
      else
      {
         out << " from N(0,inf)" << endl;
      }
   }

   out << "chi2   =" << getChi2()    << " goodnessOfFit   ="<< getGoodnessOfFit()    << " stdfac=" <<  1.0              << endl;
   out << "chi2Red=" << getChi2Reduced() << " goodnessOfFitRed="<< getGoodnessOfFitReduced() << " stdfac=" << stdDevFactor << endl;
   out << "chi2One=" << 1.0          << " goodnessOfFitOne="<< 0.5                   << " stdfac=" << sqrt( getChi2() ) << endl;

   return out;
}

double SumChiSquared( vector<vector<double> > const& y, vector<McmcProxy*> const& proxies )
{
   double chi2 = 0;
   std::vector<double> meany;
   // Calculate average response for all proxies
   SUMlib::CalcAverages( y, meany );

   // Only for the active proxies, calculate the chi2
   for ( size_t j = 0; j < meany.size(); ++j )
   {
      if ( proxies[j]->isUsed() )
      {
         double yred = proxies[j]->getScaledError( meany[j] );
         chi2 += yred*yred;
      }
   }
   return chi2;
}

double MapGOF(double GOF)
{
   double GOFcrit = fabs(GOF-0.5); // good if 0.05 < GOF < 0.95
   if (GOFcrit < 0.45)
   {
      return 100*(1 - GOFcrit/0.9); // good if > 50%
   }
   else
   {
      return std::max<double>(0.0, 1000*(0.5 - GOFcrit)); // bad if <= 50%
   }
}

void McmcStatistics::getSampleParameterAverage( ParameterData &avg ) const
{
   avg = m_pSampleAvg;
}

void McmcStatistics::getSampleParameterCovarianceMatrix( RealMatrix &covmat ) const
{
   covmat = m_pSampleCovMat;
}

void McmcStatistics::getSampleResponseAverage( ParameterData &avg ) const
{
   avg=m_ySampleAvg;
}

void McmcStatistics::getTruncatedDistributionMaximumLikelihoodParameter( Parameter &mlMu ) const
{
   mlMu = m_pSampleAvg;
}

void McmcStatistics::getTruncatedDistributionMaximumLikelihoodVariance( Parameter &mlSd ) const
{
   const unsigned int size ( m_pSampleCovMat.size() );
   mlSd.resize( size );
   for ( unsigned int i = 0; i < size; ++i )
   {
      mlSd[i] = sqrt( m_pSampleCovMat[i][i] );
   }
}

void McmcStatistics::getCatSampleParameterAverages( RealMatrix &avg ) const
{
   avg = m_pCatSampleAvgs;
}

void McmcStatistics::getCatSampleParameterCovarianceMatrix( std::vector< RealMatrix > &covmat ) const
{
   covmat = m_pCatSampleCovMat;
}

void McmcStatistics::calcCatAverages( const RealMatrix& pSample, const std::vector< unsigned int > &catIdx )
{
   m_pCatSampleAvgs.clear();
   // The number of combinations of CAT is the last value of catIdx. If no CAT is present, this one is 1.
   unsigned int catCombSize = ( catIdx.size() > 0 ) ? catIdx[ catIdx.size() - 1 ] + 1 : 1;
   std::vector< RealMatrix > pCatSamples( catCombSize );

   // If CAT varmodels, split the samples into their combinations.
   if ( catIdx.size() > 0 )
   {
      for ( unsigned int i = 0; i < catIdx.size(); ++i )
      {
         pCatSamples[ catIdx[i] ].push_back( pSample[i] );
      }
   }
   else
   {
      pCatSamples[0] = pSample;
   }

   // Calculate the different averages
   for ( unsigned int i = 0; i < pCatSamples.size(); ++i )
   {
      RealVector pCatSampleAvg;
      CalcAverages( pCatSamples[i], pCatSampleAvg );
      m_pCatSampleAvgs.push_back( pCatSampleAvg );
   }
}

void McmcStatistics::calcCatCovariances( const RealMatrix& pSample, const std::vector< unsigned int > &catIdx )
{
   m_pCatSampleCovMat.clear();
   // The number of combinations of CAT is the last value of catIdx. If no CAT is present, this one is 1.
   unsigned int catCombSize = ( catIdx.size() > 0 ) ? catIdx[ catIdx.size() - 1 ] + 1 : 1;
   std::vector< RealMatrix > pCatSamples( catCombSize );

   // If CAT varmodels, split the samples into their combinations.
   if ( catIdx.size() > 0 )
   {
      for ( unsigned int i = 0; i < catIdx.size(); ++i )
      {
         pCatSamples[ catIdx[i] ].push_back( pSample[i] );
      }
   }
   else
   {
      pCatSamples[0] = pSample;
   }

   // Calculate the different cov matrices
   for ( unsigned int i = 0; i < pCatSamples.size(); ++i )
   {
      RealMatrix pCatSampleCov;
      CalcCovariances( pCatSamples[i], m_pCatSampleAvgs[i], pCatSampleCov );
      m_pCatSampleCovMat.push_back( pCatSampleCov);
   }
}

bool McmcStatistics::load( IDeserializer* deserializer, unsigned int )
{
   bool ok = true;
   ok = ok & deserialize( deserializer, m_chi2 );
   ok = ok & deserialize( deserializer, m_gof );
   ok = ok & deserialize( deserializer, m_chi2Red );
   ok = ok & deserialize( deserializer, m_gofRed );
   ok = ok & deserialize( deserializer, m_numDegFreedom );
   ok = ok & deserialize( deserializer, m_lhMu );
   ok = ok & deserialize( deserializer, m_lhSd );
   ok = ok & deserialize( deserializer, m_pSampleAvg );
   ok = ok & deserialize( deserializer, m_pCatSampleAvgs );
   ok = ok & deserialize( deserializer, m_pSampleCovMat );
   ok = ok & deserialize( deserializer, m_pCatSampleCovMat );
   ok = ok & deserialize( deserializer, m_ySampleAvg );
   ok = ok & deserialize( deserializer, m_scaledDev );

   return ok;
}

bool McmcStatistics::save( ISerializer* serializer, unsigned int ) const
{
   bool ok = true;
   ok = ok & serialize( serializer, m_chi2 );
   ok = ok & serialize( serializer, m_gof );
   ok = ok & serialize( serializer, m_chi2Red );
   ok = ok & serialize( serializer, m_gofRed );
   ok = ok & serialize( serializer, m_numDegFreedom );
   ok = ok & serialize( serializer, m_lhMu );
   ok = ok & serialize( serializer, m_lhSd );
   ok = ok & serialize( serializer, m_pSampleAvg );
   ok = ok & serialize( serializer, m_pCatSampleAvgs );
   ok = ok & serialize( serializer, m_pSampleCovMat );
   ok = ok & serialize( serializer, m_pCatSampleCovMat );
   ok = ok & serialize( serializer, m_ySampleAvg );
   ok = ok & serialize( serializer, m_scaledDev );

   return ok;
}

} // namespace SUMlib
