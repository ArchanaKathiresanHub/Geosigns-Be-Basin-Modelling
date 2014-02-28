// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#include <algorithm>
#include <cassert>
#include <cmath>
#include <map>
#include <vector>
#include <cstdio>
#include <limits>

#include "Exception.h"
#include "KrigingWeights.h"
#include "mcmcBase.h"
#include "MarginalProbDistr.h"
#include "McmcStatistics.h"
#include "MVNormalProbDistr.h"
#include "NumericUtils.h"
#include "PolynomialProxyModel.h"
#include "McmcProxy.h"
#include "RandomGenerator.h"
#include "StepProposer.h"

using std::vector;
using std::endl;

namespace SUMlib {

/// Counts the number of measurements that are to be included in a
/// statistical analysis. This is used to determine the degrees of freedom
/// for example.
/// @param ascs The list of activeset cells to count the set use flag for.
/// @return The number of measurement included.
unsigned int mcmcBase::CountUsed( vector<McmcProxy*> const & ascs )
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
double mcmcBase::GetMinusLogPosteriorProb( bool usePrior, double logP_lh, double logP_prior )
{
   return usePrior ? -logP_lh - logP_prior : -logP_lh;
}

void mcmcBase::CopySampleAndResponse( ParameterSet const& sample, vector<RealVector> const& y,
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
const unsigned int mcmcBase::numBestMatches = 100000;

mcmcBase::mcmcBase(
      RandomGenerator &rg,
      vector<McmcProxy*> const& proxies,
      unsigned int sampleSize,
      const ParameterPdf & parameterPdf,
      const ParameterBounds & constraints,
      McmcStatistics &statistics,
      unsigned int maxNbOfIterations ) :
   m_proxies( proxies ),
   m_userSampleSize( sampleSize ),
   m_pdf( parameterPdf ),
   m_constraints( constraints ),
   m_measurementDistribution( NormalDistribution ),
   m_parameterDistribution( NoPrior ),
   m_marginalDistribution( m_pdf.sizeCon(), MarginalProbDistr::Normal ),
   m_stddevfac(1),
   m_acceptanceRate(0),
   m_rg( rg ),
   m_stepProposer( new StepProposer(rg, m_constraints.sizeOrd(), m_constraints.low().ordinalPart(), m_constraints.high().ordinalPart(), 4 ) ),
   m_mvNormalDistr( new MVNormalProbDistr( m_pdf ) ),
   m_margDistr(new MarginalProbDistr( m_pdf ) ),
   m_stepMethod(MetropolisHasting),
   m_pSample_avg( m_pdf.sizeOrd() ),
   m_pSample_cov( m_pdf.sizeOrd(), vector<double>( m_pdf.sizeOrd() ) ),
   m_ySample_avg( m_proxies.size() ),
   m_bestMatches(),
   m_statistics( statistics ),
   m_KrigingUsage( NoMcmcKriging ),
   m_krigingType( DefaultKriging ),
   m_maxNbOfIterations( maxNbOfIterations )
{
   fillCatValuesAndWeights();
   doCatInit();
}

void mcmcBase::fillCatValuesAndWeights()
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

void mcmcBase::doCatInit()
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

   reset();
}

void mcmcBase::addCatValuesAndWeights( const vector< unsigned int >& usedValues,
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

mcmcBase::~mcmcBase()
{
   delete m_stepProposer;
   delete m_margDistr;
   delete m_mvNormalDistr;
}

void mcmcBase::setMeasurementDistributionType( MeasurementDistributionType method )
{
   m_measurementDistribution = method;
}

void mcmcBase::setParameterDistributionType( ParameterDistributionType parameterDistribution )
{
   if ( m_parameterDistribution != parameterDistribution )
   {
      m_parameterDistribution = parameterDistribution;
      doCatInit();
   }
}

void mcmcBase::setMarginalDistributionType( size_t parameterSeqnb, MarginalProbDistr::Type marginalDistributionType )
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

void mcmcBase::setMarginalDistributionType( const vector<MarginalProbDistr::Type> &marginalDistribution )
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

void mcmcBase::setKrigingType( KrigingType krigingType )
{
   m_krigingType = krigingType;
}

KrigingType mcmcBase::getKrigingType( ) const
{
   return m_krigingType;
}

ParameterSet mcmcBase::extendSampleToProxyCase( ParameterSet const& parset )
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

ParameterSet mcmcBase::extendSubSampleToProxyCase( ParameterSet const& parset )
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

Parameter mcmcBase::extendSampleToProxyCase( Parameter const& p, unsigned int i )
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

Parameter mcmcBase::extendSubSampleToProxyCase( Parameter const& p, unsigned int i )
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


void mcmcBase::calcModel( ParameterSet const & parset, vector<vector<double> >& y ) const
{
   // For all samples calculate the model results
   for ( size_t i = 0; i < parset.size(); ++i )
   {
      calcModel( parset[i], y[i] );
   }
}

void mcmcBase::calcModel( Parameter const& p, vector<double>& y ) const
{
   assert( p.size() == m_proxies[0]->size() );

   // Prepare Kriging stuff
   KrigingWeights krigingWeights;
   m_proxies[0]->calcKrigingWeights( p, m_krigingType, krigingWeights );

   // For all selected models
   for ( size_t j = 0; j < m_proxies.size(); ++j )
   {
      y[j] = m_proxies[j]->getProxyValue( krigingWeights, p, m_krigingType );
   }
}

void mcmcBase::calcLh( const vector<vector<double> >& y, vector<double>& lh ) const
{
   assert( y.size() == lh.size() );
   for ( size_t i = 0; i < y.size(); ++i )
   {
      lh[i] = calcLh( y[i] );
   }
}


// only MCSolver && MCMC
double mcmcBase::calcLh( const vector<double>& y ) const
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


void mcmcBase::adaptStepSize()
{
   m_stepProposer->adaptStepSize( m_acceptanceRate );
}

void mcmcBase::setStdDevFactor( double factor )
{
   // Don't allow the factor to go below 1e-9
   static const double minStdDevFactor = 1e-9;
   m_stddevfac = std::max<double>( factor, minStdDevFactor );
   // Recalculate log likelihoods as they depend on m_stddevfac
   if ( m_iterationCount > 0 )
   {
      calcLh( m_yImpr, m_logLh_impr );
      if ( m_KrigingUsage == SmartMcmcKriging ) calcLh( m_y, m_logLh );
      else m_logLh = m_logLh_impr;
   }
}

void mcmcBase::adaptStdDevFactor()
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

void mcmcBase::updateStatistics()
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

void mcmcBase::printStatistics( std::ostream &out ) const
{
   out << "acceptanceRate " << getAcceptanceRate() << endl;

   m_statistics.print( out, getStdDevFactor(), m_proxies );
}

void mcmcBase::normalPriorSample( )
{
   RealMatrix const& varmat = m_pdf.covariance();

   // Sample within bounds of the prior
   m_pSubSample.assign( m_subSampleSize, vector<double>( m_pdf.sizeOrd(), 0 ) );
   m_mvNormalDistr->setBounds( m_constraints.low().ordinalPart(), m_constraints.high().ordinalPart() );
   m_mvNormalDistr->sample( m_rg, m_pSubSample, getUsePrior() );
   assert( m_pSubSample[0].size() == m_pdf.sizeOrd() );

   // Initialise the step sizes
   vector<double> dp( m_pdf.sizeOrd() );
   for ( size_t i = 0; i < dp.size(); ++i )
   {
      if ( i < varmat.size() ) dp[i] = 0.5*sqrt( varmat[i][i] );
      else dp[i] = 1.0 / sqrt(12.0);
   }
   m_stepProposer->setStepSize( dp );

   // Calculate the prior probabilities of the new subsample
   m_priorProb = calcPriorProb( m_pSubSample );

   // Get the model results m_yImpr for the new subsample.
   // In addition, calculate the corresponding log likelihoods.
   m_yImpr.assign( m_subSampleSize, vector<double>( m_proxies.size(), 0 ) );
   if ( m_KrigingUsage == NoMcmcKriging )
   {
      setKrigingType( NoKriging ); //use cheap proxies (no Kriging)!
   }
   else
   {
      setKrigingType( m_proxyKrigingType ); // use proxy setting for Kriging
   }
   calcModel( extendSubSampleToProxyCase( m_pSubSample ), m_yImpr );
   calcLh( m_yImpr, m_logLh_impr );

   // As above but then for m_y and m_logLh.
   if ( m_KrigingUsage == SmartMcmcKriging )
   {
      m_y.assign( m_subSampleSize, vector<double>( m_proxies.size(), 0 ) );
      setKrigingType( NoKriging );
      calcModel( extendSubSampleToProxyCase( m_pSubSample ), m_y );
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
}

void mcmcBase::initialSample()
{
   m_searchStatus.assign( m_subSampleSize, Random );
   m_lastTornadoStep.assign( m_subSampleSize, 0 );
   normalPriorSample();
}

void mcmcBase::setStepMethodType( StepMethod stepMethod )
{
   m_stepMethod = stepMethod;
}

mcmcBase::StepMethod mcmcBase::getStepMethod() const
{
   return m_stepMethod;
}

bool mcmcBase::convergenceImpl_MCMC_MC( vector<vector<double> >& sampleVar, double& stddev, const double lambda )
{
   if ( m_tooBigAccRatio )
   {
      return false;
   }
   else
   {
      CalcCovariances( m_rSample, m_rSample_avg, sampleVar );
      stddev = sqrt( sampleVar[0][0] / m_sampleSize ); //stddev of mean acceptance ratio
      if ( fabs( m_rSample_avg[0] - 1.0 ) > lambda*stddev ) return false;
   }
   // Mimimum acceptance rate of 23 % required.
   // After: Roberts, Gelman and Gilks 1994
   if ( m_acceptanceRate < 23.0 ) return false;
   return true;
}

bool mcmcBase::convergence()
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

unsigned int mcmcBase::execute()
{
   while ( iterateOnce() ){ /* nothing */ }
   return m_iterationCount;
}

unsigned int mcmcBase::iterateOnce()
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

   // Construct sample results from subsample results
   m_tooBigAccRatio = false; //no acceptance ratios calculated yet in this iteration
   for ( unsigned int i = 0; i < nbOfCycles; ++i )
   {
      // Initialise acceptance ratios for a subsample
      vector<vector<double> > rSubSample( m_subSampleSize, vector<double>(1,0) );
      doCycle( rSubSample );
      for ( unsigned int j = 0; j < m_subSampleSize; ++j )
      {
         unsigned int k = j * nbOfCycles + i;
         m_pSample[k] = m_pSubSample[j];
         m_ySample[k] = m_yImpr[j];
         m_fSample[k] = m_fSubSample[j];
         m_rSample[k] = rSubSample[j];
      }
   }

   // Store the pairs {p[i], y[i]}
   CopySampleAndResponse( m_pSample, m_ySample, m_sample_copy );

   // Update public statistics
   updateStatistics();

   return ++m_iterationCount;
}

void mcmcBase::doCycle( vector<vector<double> >& accRatios )
{
   double sumOfRates = 0.0;
   for ( unsigned int i = 1; i <= nbOfSteps; ++i )
   {
      step( i, accRatios );
      sumOfRates += m_acceptanceRate;
   }
   m_acceptanceRate = sumOfRates / nbOfSteps;
   adaptStepSize(); //adapt step size based on the cycle-averaged acceptance rate
}

void mcmcBase::step( unsigned int stepCount, vector<vector<double> >& accRatios )
{
   unsigned int nbOfAcceptedProposals = 0; //counts number of accepted proposals for this step
   unsigned int nbOfNewProposals = m_subSampleSize; //counts total number of new proposals

   // Remember subsample data before proposing a new step (p = pOld)
   vector<vector<double> > pOldSubSample = m_pSubSample;
   vector<vector<double> > yOld = m_y;
   vector<double> oldPriorProb = m_priorProb;
   vector<double> oldLogLh = m_logLh;

   // Before creating many proposals, first set proxy type
   if ( m_KrigingUsage == FullMcmcKriging )
   {
      setKrigingType( m_proxyKrigingType ); //use proxy setting for Kriging
   }
   else
   {
      setKrigingType( NoKriging ); //cheap but "poor" proxy (up to 3rd order polynomial trend)
   }

   // Propose a new step, i.e. propose a new case being a new "dot" in the subsample
   vector<bool> foundNewParProp( m_subSampleSize, false ); //new parameter proposal not found yet
   proposeStep( accRatios, foundNewParProp );

   // A new step has been proposed now but pNew (= m_pSubSample) has not been accepted yet if
   // smart Kriging has been used (in which case yNew has been generated by "poor" proxies).
   if ( m_KrigingUsage == SmartMcmcKriging )
   {
      setKrigingType( m_proxyKrigingType ); //use proxy setting for Kriging
   }
   for ( size_t i = 0; i < m_subSampleSize; ++i )
   {
      // If pNew[i] = pOld[i], there is nothing new to test.
      // So update stuff if needed and continue with the next parameter from the subsample.
      if ( !foundNewParProp[i] )
      {
         if ( stepCount == nbOfSteps )
         {
            double RMSEkey = getRMSEkey( m_logLh_impr[i] );
            updateBestMatches( RMSEkey, extendSubSampleToProxyCase( m_pSubSample[i], i ) );
            accRatios[i][0] = 1.0; //by definition
         }
         nbOfNewProposals--;
         continue;
      }

      // Determine yNew as a function of pNew, and logLhNew as a function of yNew
      vector<double> yNew( m_yImpr[i].size() );
      double logLhNew;
      // Expensive calls only in case of smart Kriging and if measurements are involved (no MC)
      stepImpl( yNew, logLhNew, i );

      // Update of best matches independent of final acceptance of m_pSubSample[i]
      double RMSEkey = getRMSEkey( logLhNew );
      if ( stepCount == nbOfSteps ) updateBestMatches( RMSEkey, extendSubSampleToProxyCase( m_pSubSample[i], i ) );

      // Final acceptance test
      double logTransRatio = oldLogLh[i] - m_logLh[i] + oldPriorProb[i] - m_priorProb[i];
      double logAccRatio = logLhNew - m_logLh_impr[i] + m_priorProb[i] - oldPriorProb[i];
      if ( acceptProposal( logTransRatio, logAccRatio ) || ( m_KrigingUsage != SmartMcmcKriging ) )
      {
         m_yImpr[i] = yNew;
         m_logLh_impr[i] = logLhNew;
         m_fSubSample[i][0] = GetMinusLogPosteriorProb( getUsePrior(), logLhNew, m_priorProb[i] );
         nbOfAcceptedProposals++;
      }
      else
      {
         m_pSubSample[i] = pOldSubSample[i]; //rejected, so back to pOld
         m_y[i] = yOld[i]; //back to yOld(pOld)
         m_priorProb[i] = oldPriorProb[i]; //back to priorProb(pOld)
         m_logLh[i] = oldLogLh[i]; //back to logLh(yOld)
         if ( m_searchStatus[i] != Random )
         {
            m_lastTornadoStep[i]++; //otherwise the same Tornado step will be proposed in vain
            if ( m_lastTornadoStep[i] == 2*m_pdf.sizeOrd() ) m_lastTornadoStep[i] = 0; //cyclic!
         }
      }

      // Store the acceptance ratio at the last step of the cycle
      if ( stepCount == nbOfSteps )
      {
         accRatios[i][0] += logAccRatio; //update (log of) acceptance ratio first
         if ( accRatios[i][0] > m_logMaxAccRatio )
         {
            accRatios[i][0] = m_logMaxAccRatio;
            m_tooBigAccRatio = true;
         }
         accRatios[i][0] = exp( accRatios[i][0] );
      }
   }

   // Update the acceptance rate for this step
   if ( nbOfNewProposals > 0 )
   {
      m_acceptanceRate *= ( double(nbOfAcceptedProposals) / nbOfNewProposals );
   }
}

void mcmcBase::proposeStep( vector<vector<double> >& accRatios, vector<bool>& newPar )
{
   unsigned int nbOfNewPar = 0; //counts all proposals that are not (yet) rejected
   unsigned int nbOfProp = 0; //counts the total number of parameter proposals
   // For each Markov chain, create proposals until one of them is no longer rejected
   for ( size_t i = 0; i < m_subSampleSize; ++i )
   {
      unsigned int maxCount;
      if ( m_searchStatus[i] == Random ) maxCount = maxNbOfRandomTrials;
      else maxCount = 2 * m_pdf.sizeOrd();
      unsigned int count = 0; //counts the number of trials for each Markov chain
      while ( !newPar[i] && (count < maxCount) && (m_searchStatus[i] != Terminated) )
      {
         // Propose a new sample pStar, with transition likelihood
         // => pStar[i][] = T( p[i][] ) and transRatio[i] p*->p / p->p*
         vector<double> pStar(m_pSubSample[i]);
         double logTransRatio = 0;
         unsigned int proposedTornadoStep = 0;
         if ( m_searchStatus[i] == Random )
         {
            m_stepProposer->proposeRandomStep( pStar, logTransRatio );
         }
         else
         {
            proposedTornadoStep = m_stepProposer->proposeTornadoStep( pStar, count, maxCount, m_lastTornadoStep[i] );
         }

         // Calculate prior probability for the proposed parameter state
         double priorProbStar = calcPriorProb( pStar );

         // Initialise yStar and logLhStar
         vector<double> yStar( m_y[i] );
         double logLhStar = proposeStepImpl1( pStar, yStar, i );

         // Initialise the (log of) acceptance likelihood ratio
         double logAccRatio = 0;

         // Prior ratio only when used and calculated
         if ( getUsePrior() )
         {
            logAccRatio += (priorProbStar - m_priorProb[i]);
         }

         // Likelihood ratio from comparison with historical data
         logAccRatio += proposeStepImpl2( logLhStar, i );

         // Preliminary acceptance test based on cheap proxies if smart Kriging is used,
         // in which case the final test based on expensive proxies occurs in step().
         // Without smart Kriging, the below test is already decisive such that the
         // test in step() is always passed.
         if ( acceptProposal( logTransRatio, logAccRatio ) )
         {
            newPar[i] = true;
            m_lastTornadoStep[i] = proposedTornadoStep;
            nbOfNewPar++;

            // Copy calculated properties
            m_pSubSample[i] = pStar;
            m_y[i] = yStar;
            m_priorProb[i] = priorProbStar;
            m_logLh[i] = logLhStar;
         }

         // Collect (log of) an unbiased acceptance ratio independent of acceptance test
         count++;
         if ( count == 1 ) accRatios[i][0] = logAccRatio;

         // Adapt search status if needed
         if ( (count == maxCount) && !newPar[i] )
         {
            proposeStepImpl3( i );
         }
      }
      nbOfProp += count;
   }
   // Calculate the preliminary acceptance rate
   if ( nbOfProp > 0 ) m_acceptanceRate = (100.0 * nbOfNewPar) / nbOfProp;
   else m_acceptanceRate = 0.0;
}

RealVector mcmcBase::calcPriorProb( ParameterSet const& parSet ) const
{
   const unsigned int size( parSet.size() );
   RealVector priorProbVect( size );
   for ( unsigned int i = 0; i < size; ++i )
   {
      priorProbVect[i] = calcPriorProb( parSet[i] );
   }
   return priorProbVect;
}

double mcmcBase::calcPriorProb( Parameter const& p ) const
{
   double priorProb = 0.0;
   if ( getUsePrior() )
   {
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
         default:
         // do nothing
         break;
      }
   }
   return priorProb;
}


bool mcmcBase::acceptProposalImpl_MCMC_MC( double logTransRatio, double& logAccRatio ) const
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
      double rand = m_rg.uniformRandom();
      accept = logAccRatio > log(rand);
   }
   return accept;
}

void mcmcBase::getP10toP90( P10ToP90Values &values, P10ToP90Parameters &parameters )
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

std::vector< vector< unsigned int > > mcmcBase::getCatCombi()
{
   std::vector< vector< unsigned int > > catCombis;
   for (size_t i = 0; i < m_catValuesWeights.size(); ++i )
   {
      catCombis.push_back( m_catValuesWeights[i].first );
   }
   return catCombis;
}

double mcmcBase::sumOfSquaredErrors(const vector<double>& proxyResult) const
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


void mcmcBase::updateBestMatches( double key, Parameter const& p )
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

bool mcmcBase::uniqueMatch( const vector<double>& match ) const
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

const std::vector<std::vector<double> > mcmcBase::getSortedYSample() const
{ 
   std::vector< std::vector< double > > sortedYSample;
   ParameterRanking::const_iterator paramIter;
   for ( paramIter = m_bestMatches.begin(); paramIter != m_bestMatches.end(); ++paramIter )
   {
      std::vector<std::pair<Parameter, RealVector> >::const_iterator copyIter;
      for ( copyIter = m_sample_copy.begin(); copyIter != m_sample_copy.end(); ++copyIter )
      {
         if ( (*paramIter).second == (*copyIter).first )
         {
            sortedYSample.push_back( (*copyIter).second );
            break;
         }
      }
   }
   if ( sortedYSample.size() < m_bestMatches.size() )
   {
      // Precaution: Because floating point comparison may be inaccurate, we need
      // to clear this vector so ProxyResponse will still evaluate the Proxy to fill the caseResults.
      sortedYSample.clear(); 
   }
   return sortedYSample;
}

double mcmcBase::getRMSEkey( double logP_lh) const
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

void mcmcBase::reset()
{
   m_iterationCount = 0;
   m_continueOnConvergence = false;
}

} // namespace SUMlib
