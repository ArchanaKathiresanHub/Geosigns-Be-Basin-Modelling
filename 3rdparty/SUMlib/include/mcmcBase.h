// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#ifndef SUMLIB_MCMCBASE_H
#define SUMLIB_MCMCBASE_H

#include <cmath>
#include <iostream>
#include <vector>
#include <map>

#include "BaseTypes.h"
#include "MarginalProbDistr.h"
#include "McmcProxy.h"
#include "KrigingProxy.h"
#include "ParameterPdf.h"
#include "McmcStatistics.h"
#include "SUMlib.h"

namespace SUMlib {

class MarginalProbDistr;
class McmcStatistics;
class MVNormalProbDistr;
class RandomGenerator;
class StepProposer;

/// @class mcmcBase implements the base of all MC (Markov Chain) to obtain a sample
/// of the posterior probability distribution using the experimental design
/// response model and the active set data.
///
/// The parameter samples are ranked (best matches) according to the RMSE
class INTERFACE_SUMLIB mcmcBase {

public:

   /// Enumerates the types of distributions available for the measured data.
   ///
   /// Normal is a Gaussian distribution, Robust is a Laplace (double
   /// exponential) distribution and Mixed is a Normal distribution with Robust
   /// tails.
   enum MeasurementDistributionType {
      NoMeasurements,
      NormalDistribution,
      RobustDistribution,
      MixedDistribution
   };

   /// Enumerates the types of parameter distributions.
   enum ParameterDistributionType {
      NoPrior,
      MarginalDistribution,
      MultivariateGaussianDistribution
   };

   /// Enumerates the stepping algorithms.
   ///
   /// MetropolisHasting is the normal MCMC method to approximate the posterior
   /// distribution by a sample. SurvivalOfTheFittest is very much like
   /// MetropolisHasting, but accepts only proposals with higher likelihoods,
   /// thereby approximating the maximum likelihood solution(s).
   enum StepMethod {
      MetropolisHasting,
      SurvivalOfTheFittest
   };

   /// Enumerates the ways how Kriging is applied.
   ///
   /// NoMcmcKriging: Only polynomial proxies are used, so less accurate but fast.
   /// SmartMcmcKriging: Kriging is only applied at decisive moments (default).
   /// FullMcmcKriging: Kriging is always applied, so accurate but expensive.
   enum KrigingUsage {
      NoMcmcKriging,
      SmartMcmcKriging,
      FullMcmcKriging
   };

   /// Number of best matches to collect
   static const unsigned int numBestMatches;

   typedef std::multimap<double, const std::vector<double> > ParameterRanking;

protected:

   /// Enumerates the different statuses of the search process.
   /// For MC and MCMC it is always Random.
   /// For MCsolver (SurvivalOfTheFittest) all statuses are possible.
   enum SearchStatus {
      Random,
      Tornado,
      Terminated
   };

   /// The search status for every sample point.
   std::vector<SearchStatus> m_searchStatus;

   /// Keeps track of the last successful Tornado step for all sample points.
   /// To be applied if search status = Tornado.
   std::vector<unsigned int> m_lastTornadoStep;

   /// Iteration counter
   unsigned int m_iterationCount;

   /// Maximum number of iterations
   unsigned int m_maxNbOfIterations;

   /// Vector containing the historical data and the ED response model to be
   /// used by a statistical analysis.
   /// Note: the McmcProxys are not owned by mcmcBase, and hence are not destroyed.
   std::vector<McmcProxy*> m_proxies;

   /// The kriging type to use when evaluating proxies.
   KrigingType m_krigingType;
   
   /// The kriging type to be used for the proxy setting.
   KrigingType m_proxyKrigingType;

   /// The prior parameter distribution
   ParameterPdf const m_pdf;
   
   /// Additional parameter constraints
   ParameterBounds const m_constraints;

   /// The total sample size and the subsample size (= number of Markov chains)
   unsigned int m_userSampleSize, m_sampleSize, m_subSampleSize;

   /**
    * Indicator to specify which measurement error distribution to use: Normal,
    * Robust or Mixed.
    *
    * Initially use robust or Mixed when the data may contain outliers. To rely
    * on statistics, use Normal.
    */
   MeasurementDistributionType m_measurementDistribution;

   /**
    * Indicator to specify which parameter distribution to use: multivariateGaussian or marginal.
    */
   ParameterDistributionType m_parameterDistribution;

   /**
    * Indicator to specify how Kriging must be used: not, smartly, or always.
    */
   KrigingUsage m_KrigingUsage;

   /**
    * When m_parameterDistribution is MarginalDistribution, specifies which parameter marginal
    * distribution to use: LogNormal, Normal, Uniform or Triangular.
    */
   std::vector<MarginalProbDistr::Type> m_marginalDistribution;

   /// Statistics calculator
   McmcStatistics &m_statistics;

   /**
    * Standard deviation factor, multiplies all historical standard deviations.
    *
    * Future releases could introduce separate factors for groups of data (for
    * the wells, for the properties) to allow more control on how to apply the
    * reduction.
    */
   double m_stddevfac;

   /**
    * Percentage of proposals accepted in last MCMC iteration.
    */
   double m_acceptanceRate;

   /**
    * Log of maximum acceptance ratio above which MCMC convergence cannot occur.
    */
   double m_logMaxAccRatio;

   /**
    * Flag indicating whether maximum acceptance ratio has been exceeded.
    */
   bool m_tooBigAccRatio;

   /// Reference to a random number generator used to accept/reject proposals.
   RandomGenerator &m_rg;

   /**
    * Transition distribution manager to create proposals.
    */
   StepProposer *m_stepProposer;

   /**
    * Prior distributions
    */
   MarginalProbDistr *m_margDistr;
   MVNormalProbDistr *m_mvNormalDistr;

   /**
    * @todo Document
    */
   StepMethod m_stepMethod;

   // Update the best matches
   void updateBestMatches( double key, Parameter const & p );

   // Check whether a candidate match is unique, i.e. not too close to existing matches.
   bool uniqueMatch(const std::vector<double>& p ) const;

   /**
    * The parameter vector corresponding to a subsample of a single step.
    * Dimension: [nbOfMarkovChains][pSize]
    */
   std::vector<std::vector<double> > m_pSubSample;

   /**
    * The sample results vector m_y is calculated either by using cheap "trend" proxies
    * (no Kriging) or by using improved proxies (trend + global Kriging).
    * The vector m_yImpr is calculated by using the improved proxies if Kriging is applied.
    * The vectors are in the same order as m_pSubSample.
    * For each element the simulated results are in the same order as in the
    * McmcProxy vector ascs. Dimension: [nbOfMarkovChains][ascs.size()]
    */
   std::vector<std::vector<double> > m_y, m_yImpr;

   /**
    * Sample vectors containing (-log of) posterior probabilities.
    * Dimension: [nbOfMarkovChains][1] resp. [sSize][1]
    */
   std::vector<std::vector<double> > m_fSubSample, m_fSample;

   /**
    * Sample vector containing the acceptance ratios.
    * Dimension: [sSize][1]
    */
   std::vector<std::vector<double> > m_rSample;

   /**
    * The sample parameter vectors p[i] versus the corresponding proxy output y[i].
    * This datastructure is used for the P10 to P90 calculation only
    *
    * Element i of m_sample_copy contains a pair {p[i], y[i]}.
    * Dimension m_sample_copy: [sSize]; dimension p[i]: [pSize]; dimension y[i]: [ascs.size()].
    */
   std::vector<std::pair<Parameter, RealVector> > m_sample_copy;

   /**
    * The (log of the) prior probabilities vector, in the same order as the
    * parameter vectors each of which corresponding to a single Markov chain.
    *
    * Dimension: [nbOfMarkovChains]
    */
   std::vector<double> m_priorProb;

   /**
    * The log likelihoods vectors based on (improved) proxy results y, in the same order
    * as the parameter vectors each of which corresponding to a single Markov chain.
    *
    * Dimension: [nbOfMarkovChains]
    */
   std::vector<double> m_logLh, m_logLh_impr;

   /**
    * Sample parameter average of the previous iteration.
    * Necessary to evaluate the stop criterion.
    * Dimension: [pSize]
    */
   std::vector<double> m_pSample_oldAvg;

   /**
    * Sample entropy of the previous iteration.
    * Necessary to evaluate the stop criterion.
    * Dimension: [1]
    */
   std::vector<double> m_sampleEntropy_old;

   /**
    * Indicates whether to continue execution when the process has converged.
    */
   bool m_continueOnConvergence;

   /**
    * The sample parameter vector, each containing several parameter factors.
    * Dimension: [sSize][pSize]
    */
   std::vector<std::vector<double> > m_pSample;

   /**
    * The sample results vector, calculated by using "expensive" proxies (global Kriging)
    * if Kriging is used. The vector is in the same order as pSample.
    * For each element the simulated results are in the same order as in the
    * McmcProxy vector ascs. Dimension: [sSize][ascs.size()]
    */
   std::vector<std::vector<double> > m_ySample;

   /**
    * Sample parameter average.
    *
    * This is calculated when the updateStatistics() method is called.
    * Dimension: [pSize]
    */
   std::vector<double> m_pSample_avg;
   
   /**
    * Sample parameter average per categorical combination.
    *
    * This is calculated when the updateStatistics() method is called.
    * Dimension: [catCombis][pSizePerCatCombi]
    */
   std::vector< std::vector<double> > m_pCatSample_avgs;

   /**
    * Sample parameter covariance matrix.
    *
    * This is calculated when the updateStatistics() method is called.
    * Dimension: [pSize][pSize]
    */
   std::vector<std::vector<double> > m_pSample_cov;

   /**
    * Sample parameter covariance matrix per categorical combination.
    *
    * This is calculated when the updateStatistics() method is called.
    * Dimension: [catCombis][pSizePerCatCombi][pSizePerCatCombi]
    */
   std::vector< std::vector< std::vector<double> > > m_pCatSample_covs;

   /**
    * Sample results average of ySample.
    *
    * These should compare well with the historical values.
    *
    * Dimension: [ascs.size()]
    */
   std::vector<double> m_ySample_avg;


   /**
    * Sample entropy, necessary to evaluate the stop criterion.
    * Dimension: [1]
    */
   std::vector<double> m_sampleEntropy;

   /**
    * Mean acceptance ratio, necessary to evaluate the stop criterion.
    * Dimension: [1]
    */
   std::vector<double> m_rSample_avg;

   /**
    * The sample parameters with maximum posterior PDF value (best matches).
    * The best 100 matches are saved (arbitrary choice).
    * Key: a weighted (log of the) posterior PDF value of type double.
    * Mapped value: parameter p of type vector<double> with dimension pSize.
    */
   ParameterRanking m_bestMatches;


public:

   /// Constructor
   ///
   /// @param[in,out] ascs                list of proxies to use for mcmcBase
   /// @param[in]     sampleSize          number of "dots" in the sample
   /// @param[in]     pdf                 most likely prior, if available (parameterpdf mean)
   /// @param[in]     constraints         constraints on the parameter space
   /// @param[in]     statistics          statistics calculator
   /// @param[in]     maxNbOfIterations   maximum number of iterations to prevent an infinite execute loop
   mcmcBase( RandomGenerator &rg, std::vector<McmcProxy*> const& ascs, unsigned int sampleSize, const ParameterPdf & pdf,
         const ParameterBounds & constraints, McmcStatistics &statistics, unsigned int maxNbOfIterations = 100 );

   /// Destructor
   virtual ~mcmcBase();

   /// Set the measurement error distristribution to be used when calculating
   /// the likelihood of a simulated result.
   ///
   /// Use the Mixed or Robust method first, until a reasonable convergence has
   /// been attained. Then analyse the match and eliminate the wrong data
   /// (outliers). Then continue with the Normal distribution on the good data.
   ///
   /// @param method The distribution type.
   void setMeasurementDistributionType( MeasurementDistributionType method );

   /// Get the measurement error distribution used.
   ///
   /// @return The used measurement error distribution.
   MeasurementDistributionType getMeasurementDistributionType() const { return m_measurementDistribution; }

   /// Set the parameter distribution to use in the MCMC acceptance criterium.
   ///
   /// When the prior is largely unknown, and determined only to provide an
   /// initial sampling, setting the distribution to NoPrior allows to ignore
   /// the prior distribution when applying the acceptance criterium in the
   /// MCMC step processes. The assumption then is that any parameter value
   /// within bounds is equally likely a priory.
   ///
   /// @param parameterDistribution The distribution.
   void setParameterDistributionType( ParameterDistributionType parameterDistribution );

   /// Get the parameter distribution.
   ///
   /// @return The parameter distribution.
   ParameterDistributionType getParameterDistributionType() const { return m_parameterDistribution; }

   /// Set the parameter marginal distribution.
   ///
   /// @param distribution The marginal distribution.
   void setMarginalDistributionType( size_t parameterSeqnb, MarginalProbDistr::Type marginalDistributionType );

   /// Set the parameter marginal distribution.
   ///
   /// @param distribution The marginal distribution.
   void setMarginalDistributionType( const std::vector<MarginalProbDistr::Type> &marginalDistribution );

   /// Get the parameter marginal distribution.
   ///
   /// @return The parameter marginal distribution.
   const std::vector<MarginalProbDistr::Type> &
      getMarginalDistributionType() const { return m_marginalDistribution; }

   /// Set the step method type.
   ///
   /// MetropolisHasting is used to obtain a sample of the posterior parameter
   /// distribution. The survival of the fittest is used to obtain the maximum
   /// likelihood parameter values.
   ///
   /// @param stepMethod The step method to use.
   void setStepMethodType( StepMethod stepMethod );

   /// Set Kriging usage: NoMcmcKriging, SmartMcmcKriging, or FullMcmcKriging.
   ///
   /// @param krUsage The indicator specifying Kriging usage.
   void setKrigingUsage( KrigingUsage krUsage ) { m_KrigingUsage = krUsage; }

   /// Setter for the kriging type to apply.
   /// @param [in] krigingType the new kriging type value
   void setKrigingType( KrigingType krigingType );
   
   /// Setter for the proxy kriging type to apply.
   /// @param [in] krigingType the new kriging type value to be used for the proxy
   void setProxyKrigingType( KrigingType krigingType ) { m_proxyKrigingType = krigingType; }

   /// Getter for the kriging type to apply
   /// @returns the current kriging type value
   KrigingType getKrigingType( ) const;

   /// Set maximum number of iterations.
   ///
   /// @param maxNbOfIterations The maximum number of iterations.
   void setMaxNbOfIterations( unsigned int maxNbOfIterations ) { m_maxNbOfIterations = maxNbOfIterations; }

   /// Get the currently used step method type.
   ///
   /// @return The currently used step method type.
   ///
   StepMethod getStepMethod() const;

   /// Set the standard deviation factor to be used.
   ///
   /// @param stdDevFactor The new standard deviation factor.
   void setStdDevFactor( double stddevfac );

   /// Get the global standard deviation factor.
   ///
   /// @return The global standard deviation factor.
   double getStdDevFactor() const { return m_stddevfac; }

   /// Get the goodness of fit (GOF) to be displayed (in %).
   /// Preferably, the GOF should be larger than about 50%.
   /// @return The goodness of fit.
   double getGOF() const { return m_statistics.getGoodnessOfFitReduced(); }

   /// Adjust the standard deviation factor assuming a perfect fit.
   ///
   /// Adjust the standard deviation factor assuming a perfect fit, ie use the
   /// square-root of the current chi-square as the standard deviation factor.
   /// This will, for reasonable amounts of data, bring the goodness-of-fit
   /// close to a half. The goodness-of-fit indicator is subsequently not useful
   /// anymore. SUM does however calculate the goodness-of-fit without using the
   /// factor for diagnostic purposes.
   void adaptStdDevFactor();

   /// Set whether to use prior in the MCMC acceptance criterium.
   ///
   /// When the prior is largely unknown, and determined only to provide an
   /// initial sampling, this method allows to ignore the prior distribution
   /// when applying the acceptance criterium in the MCMC step processes. The
   /// assumption then is that any parameter value within bounds is equally
   /// likely a priori.

   /// Get whether the prior is used in MCMC acceptance criterium.
   ///
   /// @return @c true if the prior is used, @c false otherwise.
   bool getUsePrior() const { return m_parameterDistribution != NoPrior; }

   /// Get the number of active measurements.
   /// @returns the number of active measurements, or zero if using measurements is turned off
   unsigned int getNumActiveMeasurements( ) const { return CountUsed( m_proxies ); }

   /// Get the number of samples
   /// @return the number of samples
   unsigned int sampleSize() const { return m_sampleSize; }

   /// Take an initial sample using the specified method
   /// @param [in] method the method to use for the initial sample
   void initialSample( );

   /// Get the values of pSample
   /// @return the values of pSample
   const std::vector<std::vector<double> >& getPSample() const { return m_pSample; }

   /// Get the values of ySample
   /// @return the values of ySample
   const std::vector<std::vector<double> >& getYSample() const { return m_ySample; }

   /// Get the values of ySample sorted by RMSE (same order as getBestMatches)
   /// @return the values of ySample sorted by RMSE (same order as getBestMatches)
   const std::vector<std::vector<double> > getSortedYSample() const;
   
   /// Get the average values of pSample
   /// @return the average values of pSample
   const std::vector<double>& getPSampleAvg() const { return m_pSample_avg; }

   /// Get the average values of pSample per categorical combination
   /// @return the average values of pSample per categorical combination
   const std::vector< std::vector<double> >& getPCatSampleAvgs() const { return m_pCatSample_avgs; }

   /// Get the covariance values of pSample
   /// @return the covariance values of pSample
   const std::vector<std::vector<double> >& getPSampleCov() const { return m_pSample_cov; }

   /// Get the covariance values of pSample per categorical combination
   /// @return the covariance values of pSample per categorical combination
   const std::vector<std::vector<std::vector<double> > >& getPCatSampleCovs() const { return m_pCatSample_covs; }

   /// Get the average values of ySample
   /// @return the average values of ySample
   const std::vector<double>& getYSampleAvg() const { return m_ySample_avg; }

   /// Get the sample entropy
   /// @return the sample entropy
   const std::vector<double>& getSampleEntropy() const { return m_sampleEntropy; }

   /// Get the average values of rSample
   /// @return the average values of rSample
   const std::vector<double>& getRSampleAvg() const { return m_rSample_avg; }

   /// Get the best matches
   /// @return the best matches
   const ParameterRanking& getBestMatches() const { return m_bestMatches; }

   std::vector< unsigned int > m_CatIndexOfSample;

   std::vector< unsigned int > m_CatIndexOfSubSample;

   /// Get the iteration count
   /// @return the iteration count
   unsigned int getIterationCount() { return m_iterationCount; }

   /// Get the categorical combinations
   /// @return the categorical combination
   std::vector< vector< unsigned int > > getCatCombi();

   typedef std::vector< std::pair< std::vector< unsigned int >, double > > CatValuesWeights;
   CatValuesWeights m_catValuesWeights;
   
   std::vector<double> m_catLikelihoods;

protected:

   // Virtual functions

   /// Check whether parameter proposal is acceptable
   /// @param[in]       logTransRatio  logarithm of the transition pdf ratio
   /// @param[in,out]   logAccRatio    logarithm of the acceptance ratio
   /// @returns true if parameter proposal is accepted, and false otherwise
   virtual bool acceptProposal( double logTransRatio, double& logAccRatio ) const = 0;

   /// Calcutate (log of) likelihoods for simulated results y.
   ///
   /// @param[in]     y the simulation results
   /// @returns the log likelihood for the simulated results y
   virtual double calcLh( const std::vector<double>& y ) const;

   // Impl functions (TODO: Rename to usefull names)
   virtual bool convergenceImpl( vector<vector<double> >& sampleVar, double& stddev, const double lambda, const unsigned int maxNbOfTrialsPerCycle ) = 0;

   virtual void iterateOnceImpl(){};

   virtual void stepImpl( vector<double>& yNew, double& logLhNew, const size_t i ) = 0;

   virtual double proposeStepImpl1( const vector<double>&, vector<double>&, unsigned int ){ return 0.0; }

   virtual double proposeStepImpl2( const double, const size_t ){ return 0.0; }

   virtual void proposeStepImpl3( const size_t ){};

   virtual void updateBestMatchesImpl( double key, Parameter const& p ){ m_bestMatches.insert( std::pair<double, const std::vector<double> > ( key, p ) ); }

   // Helper Impl functions (only used by MCMC and MC classes) to avoid code duplication
   bool convergenceImpl_MCMC_MC( vector<vector<double> >& sampleVar, double& stddev, const double lambda );

   bool acceptProposalImpl_MCMC_MC( double logTransRatio, double& logAccRatio ) const;

   void doCatInit();
   
   void fillCatValuesAndWeights();

   void addCatValuesAndWeights( const std::vector< unsigned int >& usedCatValues,
        const std::vector< unsigned int >& vectorValues, const RealVector& vectorWeights );

   static unsigned int CountUsed( std::vector<McmcProxy*> const & ascs );

   static double GetMinusLogPosteriorProb( bool usePrior, double logP_lh, double logP_prior );
   
   static void CopySampleAndResponse( ParameterSet const& sample, std::vector<RealVector> const& y,
               std::vector<std::pair<Parameter, RealVector> > &out );
   
   /// Create initial sampling from the Parameter PDF
   ///
   /// This does not change the prior distribution.
   ///
   void normalPriorSample();

   /// Evaluate stop criterion
   /// @returns true if stop criterion holds, and false otherwise
   bool convergence();

   /// Called a few times by iterateOnce().
   /// Carry out a number of steps of which the last one is assumed not to
   /// correlate with steps from the previous cycle.
   /// @param[out]    accRatios  acceptance ratios for a subsample
   void doCycle( std::vector<std::vector<double> >& accRatios );

   /// Called by doCycle()
   /// @param[in]     stepCount  step counter of a cycle
   /// @param[out]    accRatios  acceptance ratios for a subsample
   void step( unsigned int stepCount, std::vector<std::vector<double> >& accRatios );

   /// Called by step()
   /// @param[out]  accRatios    acceptance ratios for a subsample
   /// @param[out]  newPar       indicates whether a new parameter has been found
   void proposeStep( std::vector<std::vector<double> >& accRatios, std::vector<bool>& newPar );

   /// Calcutate set of (log of) likelihoods for simulated results y.
   ///
   /// Calculates the (logarithm of) the likelihoods of the simulation results
   /// by comparing these results with historical data. The likelihoods are
   /// calculated for the whole sample.
   ///
   /// The simulation results are presented as an argument here, and do not
   /// necessarily need to be the simulation results available as a member array
   /// in this object. This allows this method to calculate likelihoods for the
   /// results for simulated parameter proposals.
   ///
   /// @param[in]     y  The simulation results.
   /// @param[in,out] lh The computed log likelihoods.
   void calcLh( const std::vector<std::vector<double> >& y, std::vector<double>& lh ) const;

   /// Calculate (log of) prior probability for each parameter from a given parameter set.
   ///
   /// @param[in]     parSet  parameter set
   /// @returns vector containing (log of) prior probabilities
   RealVector calcPriorProb( ParameterSet const& parSet ) const;

   /// Calculate (log of) prior probability for a given parameter.
   ///
   /// @param[in]     p  parameter
   /// @returns (log of) prior probability
   double calcPriorProb( Parameter const& p ) const;

   /// Calculate (ED) simulated results for given parameters.
   ///
   /// Assumes that y is dimensioned to hold responses for all proxies
   ///
   /// @param[in]     p parameter set for which to calculate responses
   /// @param[out]    y responses of all proxies for all parameters
   void calcModel( ParameterSet const& parset, std::vector<std::vector<double> >& y ) const;

   /// Calculate (ED) simulated results for given parameters.
   ///
   /// Assumes that y is dimensioned to hold responses for all proxies
   ///
   /// @param[in]     p parameter for which to calculate responses
   /// @param[out]    y responses of all proxies
   void calcModel( Parameter const& p, std::vector<double>& y ) const;

   /// Calculate RMSE key for the best matches multimap from the log likelihood
   /// @param[in]     logP_lh log likelihood
   double getRMSEkey( double logP_lh ) const;


   ParameterSet extendSampleToProxyCase( ParameterSet const& parset );

   ParameterSet extendSubSampleToProxyCase( ParameterSet const& parset );

   Parameter extendSampleToProxyCase( Parameter const& p, unsigned int i );

   Parameter extendSubSampleToProxyCase( Parameter const& p, unsigned int i );


public:

   /// Performs multiple iterations until the MCMC process converges.
   ///
   /// Depending on the value of m_stepMethod, accept proposals using the
   /// Metropolis Hasting algorithm or accept only proposals that increase
   /// the likelihood.
   ///
   /// Metropolis Hasting is used to converge the sample towards a sample
   /// of the posterior distribution when assessing the uncertainties.
   ///
   /// Survival of the Fittest is used to converge the sample towards maximum
   /// likelihood parameters. The parameters end up in (local) optima.
   /// Distinct history matches can be found this way.
   ///
   /// @returns the number of iterations needed for convergence
   unsigned int execute();

   /// Performs a single iteration in the MCMC process. An iteration loops
   /// over a limited number of cycles each of which performing a limited
   /// number of steps. The step size is tuned such that the last steps of
   /// the cycles can be assumed uncorrelated. To establish a step, cheap
   /// "proposal steps" are generated first until one is accepted.
   ///
   /// @returns the iteration counter (0 if process has converged)
   unsigned int iterateOnce();

   /// Adjust the transition step size by examining the current acceptance rate.
   ///
   /// This utility function calls the functionality in the StepProposer. The
   /// step size is increased when too high and decreased when too low.
   void adaptStepSize();

   /// Updates all of the available statistics, including the
   /// parameter averages and result averages, goodness-of-fit
   /// and so on. This method is called after each iteration.
   void updateStatistics();

   /**
    * Get fraction of accepted proposals in the last step.
    *
    * @returns The acceptance rate.
    */
   double getAcceptanceRate() const { return m_acceptanceRate; }

   /**
    * Print to standard output.
    *
    * For testing only.
    */
   void printStatistics( std::ostream &out ) const;

   /**
    * Returns for every McmcProxy the P10 to P90 values.
    * In addition the corresponding scenarios are returned.
    *
    * The rows of the matrices are in the same order as in the
    * McmcProxy vector ascs.
    * Dimension m: [ascs.size()][9]
    * Dimension s: [ascs.size()][9][pSize]
    */
   typedef std::vector<std::vector<double> > P10ToP90Values;
   typedef std::vector<std::vector<Parameter> > P10ToP90Parameters;
   void getP10toP90( P10ToP90Values & values, P10ToP90Parameters & parameters );

   /**
    * Calculate sum of squared errors based on proxy results and measurement values.
    * Dimension proxyResult: [ascs.size()]
    */
   double sumOfSquaredErrors(const std::vector<double>& proxyResult) const;

   /**
    * Resets the mcmcBase execution by setting the iteration counter to 0.
    */
   void reset();
};

} // namespace SUMlib

#endif // SUMLIB_MCMCBASE_H
