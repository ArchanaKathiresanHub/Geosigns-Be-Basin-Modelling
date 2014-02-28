// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#include <cassert>
#include <cmath>
#include <vector>

#include "MCMC.h"

namespace SUMlib {


// Three cheap (without Kriging) pre-iterations are sufficient to estimate the
// redistribution of the Markov chains over the categorical combinations.
const size_t nrOfInitIters = 3;

void MCMC::initialise()
{
   if ( m_pdf.sizeCat() > 0 )
   {
      for ( size_t i = 0; i < nrOfInitIters; ++i )
      {
         iterateOnce();
      }
      calcCatLikelihoods();
      doCatInit();
   }
}


void MCMC::calcCatLikelihoods()
{
   const size_t nbOfCatCases = m_catValuesWeights.size();
   vector<double> weightFactor( nbOfCatCases, 0.0 );
   double scaleFactor = 0.0; //just the sum of all weight factors

   assert( m_fSample.size() == m_sampleSize );

   // Calculate the factors needed to correct the categorical weight for each CAT case
   size_t sampleIndex = 0;
   for ( size_t i = 0; i < nbOfCatCases; ++i )
   {
      size_t nbOfSamples = 0;
      while ( ( sampleIndex < m_sampleSize ) && ( m_CatIndexOfSample[sampleIndex] == i ) )
      {
         weightFactor[i] += exp( -m_fSample[sampleIndex++][0] );
         nbOfSamples++;
      }
      if ( nbOfSamples ) weightFactor[i] /= nbOfSamples;
      scaleFactor += weightFactor[i];
   }
   assert( sampleIndex == m_sampleSize );

   // Correct the categorical weight for each CAT case
   if ( scaleFactor > 0 )
   {
      for ( size_t i = 0; i < nbOfCatCases; ++i )
      {
         m_catLikelihoods[i] = ( weightFactor[i] / scaleFactor );
      }
   }
}



bool MCMC::convergenceImpl( vector<vector<double> >& sampleVar, double& stddev, const double lambda, const unsigned int )
{
   return mcmcBase::convergenceImpl_MCMC_MC( sampleVar, stddev, lambda );
}


void MCMC::stepImpl( vector<double>& yNew, double& logLhNew, const size_t i )
{
   if ( m_KrigingUsage == SmartMcmcKriging )
   {
      calcModel( extendSubSampleToProxyCase( m_pSubSample[i], i ), yNew ); //calculate expensive yNew
      logLhNew = calcLh( yNew ); //calculate corresponding log likelihood
   }
   else
   {
      yNew = m_y[i]; //because here m_y = m_yImpr by definition
      logLhNew = m_logLh[i]; //copy corresponding log likelihood
   }
}

double MCMC::proposeStepImpl1( const vector<double>& pStar, vector<double>& yStar, unsigned int i )
{
   calcModel( extendSubSampleToProxyCase( pStar, i ), yStar ); // not needed for MC
   return calcLh( yStar ); // not needed for MC
}

}
