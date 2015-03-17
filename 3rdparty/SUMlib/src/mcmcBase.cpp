// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#include <algorithm>
#include <cassert>
#include <climits>
#include <cmath>
#include <cstdio>
#include <functional>
#include <limits>
#include <map>
#include <vector>

#include "Exception.h"
#include "KrigingWeights.h"
#include "McmcBase.h"
#include "MarginalProbDistr.h"
#include "McmcStatistics.h"
#include "MVNormalProbDistr.h"
#include "NumericUtils.h"
#include "McmcProxy.h"
#include "RandomGenerator.h"
#include "StepProposer.h"

#ifdef USE_OPENMP
#include <omp.h>
#endif

using std::vector;
using std::endl;

namespace SUMlib {

/// Counts the number of measurements that are to be included in a
/// statistical analysis. This is used to determine the degrees of freedom
/// for example.
/// @param ascs The list of activeset cells to count the set use flag for.
/// @return The number of measurement included.
unsigned int McmcBase::CountUsed( vector<McmcProxy*> const & ascs )
{
   unsigned int count (0);
   for ( vector<McmcProxy*>::const_iterator it = ascs.begin(); it != ascs.end(); ++it )
   {
      if ( (*it)->isUsed() )
      {
         ++count;
      }
   }
   return count;
}

/// Calculates minus log( posterior P )
double McmcBase::GetMinusLogPosteriorProb( bool usePrior, double logP_lh, double logP_prior )
{
   return usePrior ? -logP_lh - logP_prior : -logP_lh;
}

void McmcBase::CopySampleAndResponse( ParameterSet const& sample, vector<RealVector> const& y,
   vector<std::pair<Parameter, RealVector> > &out )
{
   // Store the pairs {p[i], y[i]}
   unsigned int size( sample.size() );
   out.resize( size );
   for ( unsigned int i = 0; i < size; ++i )
   {
      out[i] = make_pair( sample[i], y[i] );
   }
}


const unsigned int iterMultiple = 10; //multiple of >1 iterations before checking convergence
const unsigned int nbOfCycles = 5; //about 5 cycles per iteration
const unsigned int nbOfSteps = 5; //about 5 steps per cycle to avoid correlation
const unsigned int maxNbOfRandomTrials = 10; //maximum of 10 random propositions per step per chain

/// Number of best matches to collect
const unsigned int McmcBase::numBestMatches = 100000;

McmcBase::McmcBase(
      int seed,
      vector<McmcProxy*> const& proxies,
      unsigned int sampleSize,
      const ParameterPdf & parameterPdf,
      const ParameterBounds & constraints,
      McmcStatistics &statistics,
      unsigned int maxNbOfIterations ) :
   m_maxNbOfIterations( maxNbOfIterations ),
   m_proxies( proxies ),
   m_krigingType( DefaultKriging ),
   m_pdf( parameterPdf ),
   m_constraints( constraints ),
   m_userSampleSize( sampleSize ),
   m_measurementDistribution( NormalDistribution ),
   m_parameterDistribution( NoPrior ),
   m_krigingUsage( NoMcmcKriging ),
   m_marginalDistribution( m_pdf.sizeCon(), MarginalProbDistr::Normal ),
   m_statistics( statistics ),
   m_stddevfac(1),
   m_acceptanceRate(0),
   m_rg( seed ),
   m_margDistr(new MarginalProbDistr( m_pdf ) ),
   m_mvNormalDistr( new MVNormalProbDistr( m_pdf ) ),
   m_stepMethod(MetropolisHasting),
   m_pSample_avg( m_pdf.sizeOrd() ),
   m_pSample_cov( m_pdf.sizeOrd(), vector<double>( m_pdf.sizeOrd() ) ),
   m_ySample_avg( m_proxies.size() ),
   m_bestMatches()
{
   assert( m_pdf.sizeOrd() == m_constraints.sizeOrd() );
   m_margDistr->setSamplingBounds( m_constraints.low().ordinalPart(), m_constraints.high().ordinalPart() );
   m_stepProposer = new StepProposer( m_rg, m_constraints.sizeOrd(), m_constraints.low().ordinalPart(), m_constraints.high().ordinalPart(), 1 );
   fillCatValuesAndWeights();
   doCatInit();
}

void McmcBase::fillCatValuesAndWeights()
{
   m_catValuesWeights.clear();

   // Initialize catValues and weights
   assert( m_constraints.catValues().size() == m_pdf.catValues().size() );
   for ( size_t i = 0; i < m_pdf.catValues().size(); ++i )
   {
      addCatValuesAndWeights( m_constraints.catValues()[i], m_pdf.catValues()[i], m_pdf.catWeights()[i] );
   }
   m_catLikelihoods.assign( m_catValuesWeights.size(), 1.0 );
}

void McmcBase::doCatInit()
{
   // Prepare the initializion of the sample size if categorical parameters are involved
   m_CatIndexOfSample.clear();
   m_CatIndexOfSubSample.clear();
   const unsigned int maxNbOfSamples = m_userSampleSize * 1.1; // Maximum number of samples when autoscaled for CAT values
   unsigned int tmpSubSampleSize = std::max( int(m_userSampleSize/nbOfCycles), 1 );
   unsigned int maxNrOfSubSamples = std::max( int(maxNbOfSamples/nbOfCycles), 1 );
   unsigned int newNrSubSample = 0;
   unsigned int minNrOfSubSamples = 0;
   if ( m_pdf.sizeCat() > 0 )
   {
      assert( !m_catValuesWeights.empty() );
      assert( m_catValuesWeights.size() == m_catLikelihoods.size() );

      // Get weights, total sum of weights, and min weight
      vector<double> weights( m_catLikelihoods );
      double weightsSum = std::numeric_limits< double >::epsilon(); //precaution to avoid division by zero
      double minWeight = std::numeric_limits< int >::max();
      for ( size_t i = 0; i < m_catValuesWeights.size(); ++i )
      {
         if ( getUsePrior() ) weights[i] *= m_catValuesWeights[ i ].second;
         weightsSum += weights[i];
         minWeight = ( weights[i] < minWeight && weights[i] > std::numeric_limits< double >::epsilon() ) ? weights[i] : minWeight;
      }

      // Normalize weights
      for ( size_t i = 0; i < m_catValuesWeights.size(); ++i )
      {
         weights[ i ] /= minWeight;
         minNrOfSubSamples += int( weights[ i ]  + .5 );
      }
      weightsSum /= minWeight;

      // Make sure the number of subSamples is within minNrOfSubSamples and maxNrOfSubSamples
      tmpSubSampleSize = std::min< unsigned int >( maxNrOfSubSamples, std::max< unsigned int >( tmpSubSampleSize, minNrOfSubSamples ) );

      // Determine the number of Markov chains for each CAT case by using the normalized weights
      for ( size_t i = 0; i < m_catValuesWeights.size(); ++i )
      {
         // Set the value (make sure that each CAT case corresponds to at least 1 chain)
         unsigned int nrOfChainsPerCAT = std::max< unsigned int >(1, int( ( tmpSubSampleSize * weights[ i ] / weightsSum ) + .5 ) );
         m_CatIndexOfSample.insert( m_CatIndexOfSample.end(), nrOfChainsPerCAT * nbOfCycles, i );
         m_CatIndexOfSubSample.insert( m_CatIndexOfSubSample.end(), nrOfChainsPerCAT, i );
         newNrSubSample += nrOfChainsPerCAT;
      }
   }

   // Set the sample size to the size that will be used for MC(MC)
   m_subSampleSize = ( m_pdf.sizeCat() == 0 ) ? tmpSubSampleSize : newNrSubSample;
   m_sampleSize = nbOfCycles * m_subSampleSize;
   m_logMaxAccRatio = 5 + log(static_cast<double>( m_sampleSize) );
   m_pSample = RealMatrix( m_sampleSize, RealVector( m_pdf.sizeOrd(), 0 ) );
   m_ySample = RealMatrix( m_sampleSize, RealVector( m_proxies.size(), 0 ) );
   m_fSample = RealMatrix( m_sampleSize, RealVector(1,0) );
   m_rSample = RealMatrix( m_sampleSize, RealVector(1,0) );
   m_priorProb = RealVector( m_subSampleSize, 0.0 );
   m_logLh = RealVector( m_subSampleSize, 0.0 );
   m_logLh_impr = RealVector( m_subSampleSize, 0.0 );

   m_nAccepted.resize( m_subSampleSize, 0 );
   m_nProposed.resize( m_subSampleSize, 0 );

   reset();
}

void McmcBase::addCatValuesAndWeights( const vector< unsigned int >& usedValues,
     const vector< unsigned int >& values, const RealVector& weights )
{
   assert( values.size() == weights.size() );
   assert( usedValues.size() <= values.size() );
   CatValuesWeights newCatValuesWeights;

   for ( size_t i = 0; i < usedValues.size(); ++i )
   {
      unsigned int index;
      unsigned int value = usedValues[ i ];
      for ( index = 0; index < values.size(); ++index )
      {
         if ( value == values[index] ) break; //look for corresponding index of values
      }
      assert( index < values.size() ); //values must contain all used values
      double weight = weights[ index ];
      if ( m_catValuesWeights.size() > 0 )
      {
         for ( size_t j = 0; j < m_catValuesWeights.size(); ++j )
         {
            vector< unsigned int > tmpVect = m_catValuesWeights[j].first;
            tmpVect.push_back( value );
            double tmpWeight = weight * m_catValuesWeights[j].second;
            newCatValuesWeights.push_back( std::make_pair( tmpVect, tmpWeight ) );
         }
      }
      else
      {
         // The first one is just a copy of the vector with its weight
         vector< unsigned int > tmpVect( 1, value );
         newCatValuesWeights.push_back( std::make_pair( tmpVect, weight ) );
      }
   }

   m_catValuesWeights = newCatValuesWeights;
}

McmcBase::~McmcBase()
{
   delete m_stepProposer;
   delete m_margDistr;
   delete m_mvNormalDistr;
}

void McmcBase::setMeasurementDistributionType( MeasurementDistributionType method )
{
   m_measurementDistribution = method;
}

void McmcBase::setParameterDistributionType( ParameterDistributionType parameterDistribution )
{
   if ( m_parameterDistribution != parameterDistribution )
   {
      m_parameterDistribution = parameterDistribution;
      doCatInit();
   }
}

void McmcBase::setMarginalDistributionType( size_t parameterSeqnb, MarginalProbDistr::Type marginalDistributionType )
{
   const size_t nbVarConPars = m_pdf.sizeCon();
   assert( m_marginalDistribution.size() == nbVarConPars );
   assert( nbVarConPars <= m_pdf.varParIdx().size() );
   for ( size_t i = 0; i < nbVarConPars; ++i )
   {
      if ( parameterSeqnb == m_pdf.varParIdx()[i] )
      {
         m_marginalDistribution[i] = marginalDistributionType;
         break;
      }
   }
   m_margDistr->setTypes( m_marginalDistribution );
}

void McmcBase::setMarginalDistributionType( const vector<MarginalProbDistr::Type> &marginalDistribution )
{
   const size_t nbVarConPars = m_pdf.sizeCon();
   const size_t nbConPars = marginalDistribution.size();
   assert( m_marginalDistribution.size() == nbVarConPars );
   assert( nbVarConPars <= m_pdf.varParIdx().size() );
   for ( size_t i = 0; i < nbVarConPars; ++i )
   {
      size_t varParIndex = m_pdf.varParIdx()[i];
      if ( varParIndex < nbConPars )
      {
         m_marginalDistribution[i] = marginalDistribution[varParIndex];
      }
      else break;
   }
   m_margDistr->setTypes( m_marginalDistribution );
}

void McmcBase::setKrigingType( KrigingType krigingType )
{
   m_krigingType = krigingType;
}

KrigingType McmcBase::getKrigingType( ) const
{
   return m_krigingType;
}

ParameterSet McmcBase::extendSampleToProxyCase( ParameterSet const& parset ) const
{
   if ( m_CatIndexOfSample.size() > 0 )
   {
      // There are CAT-values
      assert( parset.size() == m_CatIndexOfSample.size() );
      ParameterSet newParSet;
      for ( size_t i = 0; i < parset.size(); ++i )
      {
         newParSet.push_back( extendSampleToProxyCase( parset[i], i ) );
      }
      return newParSet;
   }
   else
   {
      // There are no CAT-values
      return parset;
   }
}

ParameterSet McmcBase::extendSubSampleToProxyCase( ParameterSet const& parset ) const
{
   if ( m_CatIndexOfSubSample.size() > 0 )
   {
      // There are CAT-values
      assert( parset.size() == m_CatIndexOfSubSample.size() );
      ParameterSet newParSet;
      for ( size_t i = 0; i < parset.size(); ++i )
      {
         newParSet.push_back( extendSubSampleToProxyCase( parset[i], i ) );
      }
      return newParSet;
   }
   else
   {
      // There are no CAT-values
      return parset;
   }
}

Parameter McmcBase::extendSampleToProxyCase( Parameter const& p, unsigned int i ) const
{
   if ( m_CatIndexOfSample.size() > 0 )
   {
      // There are CAT-values
      assert( i < m_CatIndexOfSample.size() );
      Parameter newPar( p );
      m_pdf.extendToProxyCase( m_catValuesWeights[ m_CatIndexOfSample[ i ] ].first, newPar );
      return newPar;
   }
   else
   {
      // There are no CAT-values
      return p;
   }
}

Parameter McmcBase::extendSubSampleToProxyCase( Parameter const& p, unsigned int i ) const
{
   if ( m_CatIndexOfSubSample.size() > 0 )
   {
      // There are CAT-values
      assert( i < m_CatIndexOfSubSample.size() );
      Parameter newPar( p );
      m_pdf.extendToProxyCase( m_catValuesWeights[ m_CatIndexOfSubSample[ i ] ].first, newPar );
      return newPar;
   }
   else
   {
      // There are no CAT-values
      return p;
   }
}


void McmcBase::calcModel( ParameterSet const & parset, vector<vector<double> >& y, KrigingType proxyKriging ) const
{
   // For all samples calculate the model results
   for ( size_t i = 0; i < parset.size(); ++i )
   {
      calcModel( parset[i], y[i], proxyKriging );
   }
}

void McmcBase::calcModel( Parameter const& p, vector<double>& y, KrigingType proxyKriging ) const
{
   assert( p.size() == m_proxies[0]->size() );

   // Prepare Kriging stuff
   KrigingWeights krigingWeights;
   m_proxies[0]->calcKrigingWeights( p, proxyKriging, krigingWeights );

   // For all selected models
   for ( size_t j = 0; j < m_proxies.size(); ++j )
   {
      y[j] = m_proxies[j]->getProxyValue( krigingWeights, p, proxyKriging );
   }
}

void McmcBase::calcLh( const vector<vector<double> >& y, vector<double>& lh ) const
{
   assert( y.size() == lh.size() );
   for ( size_t i = 0; i < y.size(); ++i )
   {
      lh[i] = calcLh( y[i] );
   }
}


// only MCSolver && MCMC
double McmcBase::calcLh( const vector<double>& y ) const
{
   double log_lh = 0;
   for ( size_t j = 0; j < m_proxies.size(); ++j )
   {
      if ( ! m_proxies[j]->isUsed() )
      {
         continue;
      }

      // Reduced deviation
      double dyRed = fabs( m_proxies[j]->getScaledError( y[j] ) )/m_stddevfac;

      // Measurement distribution type
      switch ( m_measurementDistribution )
      {
         case MixedDistribution: // Mixed mode, Gauss for close points, Laplace for outliers
            log_lh += ( dyRed < 2.0*sqrt(2.0) ? -0.5*dyRed*dyRed : -sqrt(2.0)*dyRed );
            break;

         case NormalDistribution: // Gaussian or normal measurement error distribution
            log_lh += -0.5*dyRed*dyRed;
            break;

         case RobustDistribution: // Laplace or double exponenial measurement error distribution
            log_lh += -sqrt(2.0)*dyRed;
            break;

         default:
            assert( ! "Unknown distribution" );
            break;
      }
   }
   return log_lh;
}


void McmcBase::adaptStepSize()
{
   double totNom = 0.0;
   double totDenom = 0.0;
   std::vector<double> acceptanceRate( m_subSampleSize, 0.0 );

   for ( size_t i = 0; i < m_subSampleSize; ++i )
   {
      if ( m_nProposed[i] > 0 )
      {
         acceptanceRate[i] = (100.0 * m_nAccepted[i]) / m_nProposed[i];
      }
      else
      {
         acceptanceRate[i] = 0.0;
      }
      totNom += m_nAccepted[i];
      totDenom += m_nProposed[i];
   }

   m_acceptanceRate = totDenom > 0.0 ? ( 100 * totNom / totDenom ) : 0.0;

   m_stepProposer->adaptStepSize( acceptanceRate );
   std::fill( m_nAccepted.begin(), m_nAccepted.end(), 0 );
   std::fill( m_nProposed.begin(), m_nProposed.end(), 0 );
}

void McmcBase::setStdDevFactor( double factor )
{
   // Don't allow the factor to go below 1e-9
   static const double minStdDevFactor = 1e-9;
   m_stddevfac = std::max<double>( factor, minStdDevFactor );
   // Recalculate log likelihoods as they depend on m_stddevfac
   if ( m_iterationCount > 0 )
   {
      calcLh( m_yImpr, m_logLh_impr );
      if ( m_krigingUsage == SmartMcmcKriging ) calcLh( m_y, m_logLh );
      else m_logLh = m_logLh_impr;
   }
}

void McmcBase::adaptStdDevFactor()
{
   assert( m_iterationCount > 0 );

   const unsigned int nbDegFreedom( getNumActiveMeasurements() );

   // Calculate chi2, goodness of fit. Reduced values are calculated with new sqrt(chi2) as std. dev. factor
   m_statistics.calcChi2Distribution( m_ySample, m_proxies, nbDegFreedom );

   // Calculate stddev factor using goodness of fit = 100% (or chi2 = 1)
   setStdDevFactor( sqrt( m_statistics.getChi2() ) );

   // Best matches are no longer up to date after changing the standard deviation
   m_bestMatches.clear();

   // MCMC needs to find a new equilibrium, so restart the iteration count
   m_iterationCount = 1;
}

void McmcBase::updateStatistics()
{
   // Calculate chi2 characteristics, sample average and correlation matrix. Use current std. dev. factor
   m_statistics.update( m_pSample, m_ySample, getNumActiveMeasurements(), getStdDevFactor(), m_proxies, m_CatIndexOfSample );

   m_statistics.getSampleParameterAverage( m_pSample_avg );
   m_statistics.getSampleParameterCovarianceMatrix( m_pSample_cov );
   m_statistics.getSampleResponseAverage( m_ySample_avg );

   m_statistics.getCatSampleParameterAverages( m_pCatSample_avgs );
   m_statistics.getCatSampleParameterCovarianceMatrix( m_pCatSample_covs );

   // Calculate sample entropy and mean acceptance ratio
   CalcAverages( m_fSample, m_sampleEntropy );
   CalcAverages( m_rSample, m_rSample_avg );
}

void McmcBase::printStatistics( std::ostream &out ) const
{
   out << "acceptanceRate " << getAcceptanceRate() << endl;

   m_statistics.print( out, getStdDevFactor(), m_proxies );
}

void McmcBase::normalPriorSample( )
{
   RealMatrix const& varmat = m_pdf.covariance();

   // Sample within bounds of the prior
   m_pSubSample.assign( m_subSampleSize, vector<double>( m_pdf.sizeOrd(), 0 ) );
   m_mvNormalDistr->setSamplingBounds( m_constraints.low().ordinalPart(), m_constraints.high().ordinalPart() );
   m_mvNormalDistr->sample( m_rg, m_pSubSample, getUsePrior() );
   assert( m_pSubSample[0].size() == m_pdf.sizeOrd() );

   // Initialise the step sizes to 0.25 * sigma / sqrt( dimension of parameter space )
   vector<double> dp( m_pdf.sizeOrd() );
   for ( size_t i = 0; i < dp.size(); ++i )
   {
      if ( i < varmat.size() ) dp[i] = 0.25 * sqrt( varmat[i][i] / m_pdf.sizeOrd() );
      else dp[i] = 0.5 / sqrt( 12.0 * m_pdf.sizeOrd() );
   }
   m_stepProposer->setStepSize( dp, m_subSampleSize );

   // Calculate the prior probabilities of the new subsample
   m_priorProb = calcPriorProb( m_pSubSample );

   // Get the model results m_yImpr for the new subsample.
   // In addition, calculate the corresponding log likelihoods.
   m_yImpr.assign( m_subSampleSize, vector<double>( m_proxies.size(), 0 ) );
   KrigingType proxyKriging = m_krigingUsage == NoMcmcKriging ? NoKriging : m_proxyKrigingType;
   calcModel( extendSubSampleToProxyCase( m_pSubSample ), m_yImpr, proxyKriging );
   calcLh( m_yImpr, m_logLh_impr );

   // As above but then for m_y and m_logLh.
   if ( m_krigingUsage == SmartMcmcKriging )
   {
      m_y.assign( m_subSampleSize, vector<double>( m_proxies.size(), 0 ) );
      calcModel( extendSubSampleToProxyCase( m_pSubSample ), m_y, NoKriging );
      calcLh( m_y, m_logLh );
   }
   else //copy if Kriging is either always or never applied
   {
      m_y = m_yImpr;
      m_logLh = m_logLh_impr;
   }

   // Create the subsample of (-log) posterior probabilities
   m_fSubSample.assign( m_subSampleSize, vector<double>(1,0) );
   for ( unsigned int i = 0; i < m_subSampleSize; ++i )
   {
      m_fSubSample[i][0] = GetMinusLogPosteriorProb( getUsePrior(), m_logLh_impr[i], m_priorProb[i] );
   }

   // Calculate parameter average and entropy from the initial subsample
   // in order to initialise m_pSample_oldAvg and m_sampleEntropy_old.
   CalcAverages( m_pSubSample, m_pSample_oldAvg );
   CalcAverages( m_fSubSample, m_sampleEntropy_old );

   // Initialise the random number generators
   for ( unsigned int i = 0; i < m_subSampleSize; ++i )
   {
      int seed = INT_MAX * m_rg.uniformRandom();
      RandomGenerator* rng = new RandomGenerator( seed );
      m_rngs.push_back( *rng );
   }
}

void McmcBase::initialSample()
{
   m_searchStatus.assign( m_subSampleSize, Random );
   m_lastTornadoStep.assign( m_subSampleSize, 0 );
   normalPriorSample();
}

void McmcBase::setStepMethodType( StepMethod stepMethod )
{
   m_stepMethod = stepMethod;
}

McmcBase::StepMethod McmcBase::getStepMethod() const
{
   return m_stepMethod;
}

bool McmcBase::convergenceImpl_MCMC_MC( vector<vector<double> >& sampleVar, double& stddev, const double lambda )
{
   bool isConverged = false;
   if ( !m_tooBigAccRatio )
   {
      CalcCovariances( m_rSample, m_rSample_avg, sampleVar );
      stddev = sqrt( sampleVar[0][0] / m_sampleSize ); //stddev of mean acceptance ratio
      if ( fabs( m_rSample_avg[0] - 1.0 ) <= lambda*stddev )
      {
         // Mimimum acceptance rate of 23 % required.
         // After: Roberts, Gelman and Gilks 1994
         if ( m_acceptanceRate >= 23.0 )
         {
            isConverged = true;
         }
      }
   }
   return isConverged;
}

bool McmcBase::convergence()
{
   // Test whether maximal number of iterations have been performed
   if ( m_iterationCount > 0 && ( m_iterationCount % m_maxNbOfIterations ) == 0 ) return true;

   // Test whether a multiple of 'iterMultiple' iterations have been performed
   if ( m_iterationCount % iterMultiple != 0 ) return false;

   // Test convergence of parameter average
   for ( unsigned int i = 0; i < m_pdf.sizeOrd(); ++i )
   {
      // Standard deviation of a single sample parameter average
      double stddev = sqrt( m_pSample_cov[i][i] / m_sampleSize );
      double pAvgDiff = fabs( m_pSample_avg[i] - m_pSample_oldAvg[i] );
      if ( pAvgDiff > 3.0*stddev ) return false; //3*stddev to eventually pass for ALL i
   }

   // Test convergence of entropy
   const double lambda = 1.28; //stddev multiplier = 1.28 yields a confidence interval of 80%
   vector<vector<double> > sampleVar; //1X1 matrix with the sample variance
   CalcCovariances( m_fSample, m_sampleEntropy, sampleVar );
   double stddev = sqrt( sampleVar[0][0] / m_sampleSize ); //stddev of entropy estimator
   double entropyDiff = fabs( m_sampleEntropy[0] - m_sampleEntropy_old[0] );
   if ( entropyDiff > lambda*stddev ) return false;

   // Test convergence of mean acceptance ratio (only for MetropolisHasting!).
   // Also test on critical bounds of the acceptance rate.
   unsigned int maxNbOfTrialsPerCycle = std::max<int>( maxNbOfRandomTrials, 2 * m_pdf.sizeOrd() );
   maxNbOfTrialsPerCycle *= nbOfSteps * m_subSampleSize;

   return convergenceImpl( sampleVar, stddev, lambda, maxNbOfTrialsPerCycle );
}

unsigned int McmcBase::execute()
{
   while ( iterateOnce() ){ /* nothing */ }
   return m_iterationCount;
}

unsigned int McmcBase::iterateOnce()
{
   // In the first iteration an initial sample needs to be drawn
   if ( m_iterationCount == 0 )
   {
      initialSample();
   }
   // Check whether stop criterion holds in all subsequent iterations
   else if ( convergence() && !m_continueOnConvergence )
   {
      m_continueOnConvergence = true;
      iterateOnceImpl();

      // Reset iteration counter
      return 0;
   }
   // Update old info needed for next convergence check after each 'iterMultiple' iterations
   else if ( m_iterationCount % iterMultiple == 0 )
   {
      m_pSample_oldAvg = m_pSample_avg; //store parameter average of previous iteration
      m_sampleEntropy_old = m_sampleEntropy; //store sample entropy of previous iteration
   }

   m_continueOnConvergence = false;

   // Clear the best matches prior to each iteration
   m_bestMatches.clear();

   // Set the number of accepted proposals in this iteration
   std::fill( m_nAccepted.begin(), m_nAccepted.end(), 0 );
   std::fill( m_nProposed.begin(), m_nProposed.end(), 0 );

   std::vector<RmseCasePair> bestMatchValues( m_sampleSize );
   // Construct sample results from subsample results
   m_tooBigAccRatio = false; //no acceptance ratios calculated yet in this iteration

   // Run this piece in parallel
   // Use OpenMP. Some chain calculations are faster than others, so use a guided schedule
#ifdef USE_OPENMP
   #pragma omp parallel for schedule(guided)
#endif
   for ( unsigned int iChain = 0; iChain < m_subSampleSize; iChain++ )
   {
      for ( unsigned int i = 0; i < nbOfCycles; ++i )
      {
         // Initialise acceptance ratios for a subsample
         vector<double> rSubSample( 1, 0.0 );
         unsigned int k = iChain * nbOfCycles + i;
         doCycle( iChain, rSubSample, bestMatchValues[k] );
         m_pSample[k] = m_pSubSample[iChain];
         m_ySample[k] = m_yImpr[iChain];
         m_fSample[k] = m_fSubSample[iChain];
         m_rSample[k] = rSubSample;
      }
   }

   for ( unsigned int i = 0; i < m_sampleSize; ++i )
   {
      updateBestMatches( bestMatchValues[i].first, bestMatchValues[i].second );
   }

   // Stop adaptation of step size after 80% of max number of iterations (if not yet converged)
   if ( m_iterationCount < 4 * m_maxNbOfIterations / 5 ) adaptStepSize();

   // Store the pairs {p[i], y[i]}
   CopySampleAndResponse( m_pSample, m_ySample, m_sample_copy );

   // Update public statistics
   updateStatistics();

   return ++m_iterationCount;
}

void McmcBase::doCycle( unsigned int iChain, vector<double>& accRatios, RmseCasePair& bestMatch )
{
   for ( unsigned int i = 1; i <= nbOfSteps; ++i )
   {
      step( iChain, i, accRatios, bestMatch );
   }
}

void McmcBase::step( unsigned int iChain, unsigned int stepCount, vector<double>& accRatios, RmseCasePair& bestMatch )
{
   // Remember subsample data before proposing a new step (p = pOld)
   vector<double> pOldSubSample = m_pSubSample[iChain];
   vector<double> yOld = m_y[iChain];
   double oldPriorProb = m_priorProb[iChain];
   double oldLogLh = m_logLh[iChain];

   // Propose a new step, i.e. propose a new case being a new "dot" in the subsample
   bool foundNewParProp( false ); //new parameter proposal not found yet
   proposeStep( iChain, accRatios, foundNewParProp );

   // A new step has been proposed now but pNew (= m_pSubSample) has not been accepted yet if
   // smart Kriging has been used (in which case yNew has been generated by "poor" proxies).
   if ( m_krigingUsage == SmartMcmcKriging )
   {
      setKrigingType( m_proxyKrigingType ); //use proxy setting for Kriging
   }

   // If pNew[i] = pOld[i], there is nothing new to test.
   // So update stuff if needed and continue with the next parameter from the subsample.
   if ( !foundNewParProp )
   {
      if ( stepCount == nbOfSteps )
      {
         bestMatch.first = getRMSEkey( m_logLh_impr[iChain] );
         bestMatch.second = extendSubSampleToProxyCase( m_pSubSample[iChain], iChain );
         accRatios[0] = 1.0; //by definition
      }
      return;
   }

   // Determine yNew as a function of pNew, and logLhNew as a function of yNew
   vector<double> yNew( m_yImpr[iChain].size() );
   double logLhNew;
   // Expensive calls only in case of smart Kriging and if measurements are involved (no MC)
   stepImpl( yNew, logLhNew, iChain );

   // Update of best matches independent of final acceptance of m_pSubSample[i]
   if ( stepCount == nbOfSteps )
   {
      bestMatch.first = getRMSEkey( logLhNew );
      bestMatch.second = extendSubSampleToProxyCase( m_pSubSample[iChain], iChain );
   }

   // Final acceptance test
   double logTransRatio = oldLogLh - m_logLh[iChain] + oldPriorProb - m_priorProb[iChain];
   double logAccRatio = logLhNew - m_logLh_impr[iChain] + m_priorProb[iChain] - oldPriorProb;
   if ( ( m_krigingUsage != SmartMcmcKriging ) || acceptProposal( logTransRatio, logAccRatio, m_rngs[iChain] ) )
   {
      m_yImpr[iChain] = yNew;
      m_logLh_impr[iChain] = logLhNew;
      m_fSubSample[iChain][0] = GetMinusLogPosteriorProb( getUsePrior(), logLhNew, m_priorProb[iChain] );
      m_nAccepted[iChain] += 1;
   }
   else
   {
      m_pSubSample[iChain] = pOldSubSample; //rejected, so back to pOld
      m_y[iChain] = yOld; //back to yOld(pOld)
      m_priorProb[iChain] = oldPriorProb; //back to priorProb(pOld)
      m_logLh[iChain] = oldLogLh; //back to logLh(yOld)
      if ( m_searchStatus[iChain] != Random )
      {
         m_lastTornadoStep[iChain]++; //otherwise the same Tornado step will be proposed in vain
         if ( m_lastTornadoStep[iChain] == 2*m_pdf.sizeOrd() ) m_lastTornadoStep[iChain] = 0; //cyclic!
      }
   }

   // Store the acceptance ratio at the last step of the cycle
   if ( stepCount == nbOfSteps )
   {
      accRatios[0] += logAccRatio; //update (log of) acceptance ratio first
      if ( accRatios[0] > m_logMaxAccRatio )
      {
         accRatios[0] = m_logMaxAccRatio;
         m_tooBigAccRatio = true;
      }
      accRatios[0] = exp( accRatios[0] );
   }
}

void McmcBase::proposeStep( unsigned int iChain, vector<double>& accRatios, bool& newPar )
{
   // For each Markov chain, create proposals until one of them is no longer rejected
   unsigned int maxCount;
   if ( m_searchStatus[iChain] == Random ) maxCount = maxNbOfRandomTrials;
   else maxCount = 2 * m_pdf.sizeOrd();
   unsigned int count = 0; //counts the number of trials for each Markov chain
   while ( !newPar && (count < maxCount) && (m_searchStatus[iChain] != Terminated) )
   {
      // Propose a new sample pStar, with transition likelihood
      // => pStar[i][] = T( p[i][] ) and transRatio[i] p*->p / p->p*
      vector<double> pStar(m_pSubSample[iChain]);
      double logTransRatio = 0;
      unsigned int proposedTornadoStep = 0;
      if ( m_searchStatus[iChain] == Random )
      {
         m_stepProposer->proposeRandomStep( m_rngs[iChain], pStar, logTransRatio, iChain );
      }
      else
      {
         proposedTornadoStep = m_stepProposer->proposeTornadoStep( pStar, count, maxCount, m_lastTornadoStep[iChain] );
      }

      // Calculate prior probability for the proposed parameter state
      double priorProbStar = calcPriorProb( pStar );

      // Initialise yStar and logLhStar
      vector<double> yStar( m_y[iChain] );
      double logLhStar = proposeStepImpl1( pStar, yStar, iChain );

      // Initialise the (log of) acceptance likelihood ratio
      double logAccRatio = (priorProbStar - m_priorProb[iChain]);

      // Likelihood ratio from comparison with historical data
      logAccRatio += proposeStepImpl2( logLhStar, iChain );

      // Preliminary acceptance test based on cheap proxies if smart Kriging is used,
      // in which case the final test based on expensive proxies occurs in step().
      // Without smart Kriging, the below test is already decisive such that the
      // test in step() is always passed.
      if ( acceptProposal( logTransRatio, logAccRatio, m_rngs[iChain] ) )
      {
         newPar = true;
         m_lastTornadoStep[iChain] = proposedTornadoStep;

         // Copy calculated properties
         m_pSubSample[iChain] = pStar;
         m_y[iChain] = yStar;
         m_priorProb[iChain] = priorProbStar;
         m_logLh[iChain] = logLhStar;
      }

      // Collect (log of) an unbiased acceptance ratio independent of acceptance test
      count++;
      if ( count == 1 )
      {
         accRatios[0] = logAccRatio;
      }

      // Adapt search status if needed
      if ( (count == maxCount) && !newPar )
      {
         proposeStepImpl3( iChain );
      }
   }

   m_nProposed[iChain] += count;
}

RealVector McmcBase::calcPriorProb( ParameterSet const& parSet ) const
{
   const unsigned int size( parSet.size() );
   RealVector priorProbVect( size );
   for ( unsigned int i = 0; i < size; ++i )
   {
      priorProbVect[i] = calcPriorProb( parSet[i] );
   }
   return priorProbVect;
}

double McmcBase::calcPriorProb( Parameter const& p ) const
{
   double priorProb = 0.0;
   switch ( m_parameterDistribution )
   {
      case MultivariateGaussianDistribution:
      {
         priorProb = m_mvNormalDistr->calcLogPriorProb( p );
      }
      break;
      case MarginalDistribution:
      {
         priorProb = m_margDistr->calcLogPriorProb( p );
      }
      break;
      case NoPrior:
      {
         priorProb = m_margDistr->correctForDisBounds( p );
      }
      break;
      default:
      // do nothing
      break;
   }
   return priorProb;
}


bool McmcBase::acceptProposalImpl_MCMC_MC( double logTransRatio, double& logAccRatio, RandomGenerator& rg ) const
{
   // Add logarithm of transition probability ratio
   logAccRatio += logTransRatio;

   // Decide to accept the proposal or not
   bool accept = logAccRatio >= 0.0; //zero must be accepted here!

   if ( ! accept )
   {
      // Still accept when ratio larger than random number from U[0,1].
      // Only done when logAccRatio less than zero to avoid
      // unnecessary random number generation.
      double rand = rg.uniformRandom();
      accept = logAccRatio > log(rand);
   }
   return accept;
}

void McmcBase::getP10toP90( P10ToP90Values &values, P10ToP90Parameters &parameters )
{
   if ( m_sample_copy.empty() )
   {
      return;
   }

   size_t   numProxies(m_sample_copy.front().second.size());

   values.resize(numProxies);
   parameters.resize(numProxies);

   for (size_t i = 0; i < numProxies; i++)
   {
      size_t                                   numSamples(m_sample_copy.size());
      vector<std::pair<double, size_t > > proxyValues(numSamples);

      // Store the proxy values and the index to the corresponding parameter
      for (size_t k = 0; k < numSamples; k++)
      {
         proxyValues[k] = std::make_pair( m_sample_copy[k].second[i], k );
      }

      // sort on pair::first, i.e. the proxy value
      std::sort(proxyValues.begin(), proxyValues.end());

      vector<double>            &value(values[i]);
      vector<Parameter>         &parameter(parameters[i]);

      value.resize(9); // 9 bins
      parameter.resize(9); // 9 bins

      for (size_t j = 0; j < 9; j++)
      {
         size_t   key( std::min( numSamples, static_cast<size_t>((j + 1) * numSamples / 10.0)) );

         value[j]     = proxyValues[key].first;
         parameter[j] = extendSampleToProxyCase( m_sample_copy[ proxyValues[key].second ].first, key );
      } // for all P
   } // for all mcmcProxy
}

std::vector< vector< unsigned int > > McmcBase::getCatCombi()
{
   std::vector< vector< unsigned int > > catCombis;
   for (size_t i = 0; i < m_catValuesWeights.size(); ++i )
   {
      catCombis.push_back( m_catValuesWeights[i].first );
   }
   return catCombis;
}

double McmcBase::sumOfSquaredErrors(const vector<double>& proxyResult) const
{
   double sumOfSquares = 0.0;
   for ( size_t i = 0; i < m_proxies.size(); ++i )
   {
      if ( m_proxies[i]->isUsed() )
      {
         const double error = m_proxies[i]->getScaledError( proxyResult[i] )/m_stddevfac;
         sumOfSquares += error*error;
      }
   }
   return sumOfSquares;
}

namespace {

void CalculateTolerance( ParameterPdf const& pdf, vector<double> &eps )
{
   // vector of tolerances that determines uniqueness
   const double mult = 0.01; // arbitrary(?) multiplier < 0.1
   size_t nbPar = pdf.sizeOrd(); //number of ordinal parameters
   for ( size_t i = 0; i < pdf.catValues().size(); ++i )
   {
      nbPar += ( pdf.catValues()[i].size() - 1 ); //add number of dummy parameters for each CAT parameter
   }
   eps.resize( nbPar, 2.0 ); //the number 2.0 is inspired by scaled range = 1.0 - (-1.0) = 2.0
   std::transform( eps.begin(), eps.end(), eps.begin(), std::bind2nd( std::multiplies<double>(), mult ) );
}

} // anon. namespace


void McmcBase::updateBestMatches( double key, Parameter const& p )
{
   updateBestMatchesImpl( key, p );

   // remove the worst match from the map
   while ( m_bestMatches.size() > numBestMatches )
   {
      m_bestMatches.erase( --m_bestMatches.end() );
   }
}

namespace {

/// Function object for comparing parameters
struct ParameterIsClose
{
   vector<double> eps;

   ParameterIsClose( ParameterPdf const& pdf )
   {
      CalculateTolerance( pdf, eps );
   }

   /// Function operator.
   /// @returns true if all elements of p1 and p2 are close to each other,
   /// compared to the parameter element ranges defined by the ParameterPdf
   bool operator()( Parameter const& p1, Parameter const& p2 ) const
   {
      const unsigned int size = p1.size();
      bool ok( p2.size() == size );
      for ( size_t i = 0; ok && i < size; ++i )
      {
         ok = ( fabs( p1[i] - p2[i] ) <= eps[i] );
      }
      return ok;
   }
};

} // anon. namespace

bool McmcBase::uniqueMatch( const vector<double>& match ) const
{
   bool unique(true);

   ParameterIsClose isClose( m_pdf );

   // Find out if match is close to any of the current best matches. If not, it is unique
   std::multimap<double, const vector<double> >::const_iterator it;
   for ( it = m_bestMatches.begin(); unique && it != m_bestMatches.end(); ++it )
   {
      unique = ! isClose( match, it->second );
   }
   return unique;
}

bool McmcBase::isSameCase( const std::vector<double>& case1, const std::vector<double>& case2, unsigned int key ) const
{
   assert ( case2.size() <= case1.size() );  // case2 misses the categorical part

   // Compare the continuous parts
   size_t i = 0;
   for (  ; i < case2.size(); ++i )
   {
      if ( !IsEqualTo( case1[i], case2[i]) )
         return false;
   }

   // Now compare the categorical part
   // Inlined the extendSampleToProxyCase
   if ( m_CatIndexOfSample.size() > 0 )
   {
      // There are CAT-values
      assert( key < m_CatIndexOfSample.size() );
      Parameter newPar( case2 );
      m_pdf.extendToProxyCase( m_catValuesWeights[ m_CatIndexOfSample[ key ] ].first, newPar );
      for (  ; i < newPar.size(); ++i )
      {
         if ( !IsEqualTo( case1[i], newPar[i]) )
            return false;
      }
   }
   return true;
}

const std::vector<std::vector<double> > McmcBase::getSortedYSample() const
{
   std::vector< std::vector< double > > sortedYSample;
   size_t nBest = m_bestMatches.size();
   sortedYSample.resize( nBest );
   std::vector< std::pair< double, std::vector< double > > > flatRanking;
   flatRanking.reserve( nBest );
   ParameterRanking::const_iterator paramIter;
   for ( paramIter = m_bestMatches.begin(); paramIter != m_bestMatches.end(); ++paramIter )
   {
      flatRanking.push_back( std::pair<double, std::vector< double > >( paramIter->first, paramIter->second ) );
   }
   assert( flatRanking.size() == nBest );
#ifdef USE_OPENMP
      #pragma omp parallel for
#endif
   for ( size_t i = 0; i < nBest; i++ )
   {
      std::vector< double >& paramCase = flatRanking[i].second;
      std::vector<std::pair<Parameter, RealVector> >::const_iterator copyIter;
      unsigned int key = 0;
      for ( copyIter = m_sample_copy.begin(); copyIter != m_sample_copy.end(); ++copyIter )
      {
         if ( isSameCase( paramCase, copyIter->first, key ) )
         {
            sortedYSample[i] = copyIter->second;
            break;
         }
         key = key + 1;
      }
   }
   return sortedYSample;
}

double McmcBase::getRMSEkey( double logP_lh) const
{
   unsigned int n = getNumActiveMeasurements();
   if ( n == 0 )
   {
      return 0.0;
   }
   else
   {
      return sqrt( -2.0 * logP_lh / n );
   }
}

void McmcBase::reset()
{
   m_iterationCount = 0;
   m_continueOnConvergence = false;
}

} // namespace SUMlib
