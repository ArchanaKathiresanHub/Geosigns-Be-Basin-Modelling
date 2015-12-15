// Copyright 2014, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

//////////////////////////////////////////////////////////////////////////////
///
///   This is the formal interface of MCMC-like objects in SUMlib
///   This interface is not consistent nor properly designed
///   It basically contains all the existing **public** objects of the original McmcBase
///   If a function is missing here, it can be added, provided that it fits in the general design
///
//////////////////////////////////////////////////////////////////////////////
#ifndef SUMLIB_IMCMC_H
#define SUMLIB_IMCMC_H

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

/// @interface IMcmc implements the communication between multirun and sumlib
///                  for the MCMC algorithm(s)
class INTERFACE_SUMLIB IMcmc {

public:

   /// Enumerate the (three) types of Mcmc method
   enum McmcType
   {
      Mcmc_MCMC,
      Mcmc_MC,
      Mcmc_MCSolver
   };

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

   typedef std::multimap<double, const std::vector<double> > ParameterRanking;

   virtual ~IMcmc() {};

   /// Set the measurement error distristribution to be used when calculating
   /// the likelihood of a simulated result.
   ///
   /// Use the Mixed or Robust method first, until a reasonable convergence has
   /// been attained. Then analyse the match and eliminate the wrong data
   /// (outliers). Then continue with the Normal distribution on the good data.
   ///
   /// @param method The distribution type.
   virtual void setMeasurementDistributionType( MeasurementDistributionType method ) = 0;

   /// Set the parameter distribution to use in the MCMC acceptance criterium.
   ///
   /// When the prior is largely unknown, and determined only to provide an
   /// initial sampling, setting the distribution to NoPrior allows to ignore
   /// the prior distribution when applying the acceptance criterium in the
   /// MCMC step processes. The assumption then is that any parameter value
   /// within bounds is equally likely a priory.
   ///
   /// @param parameterDistribution The distribution.
   virtual void setParameterDistributionType( ParameterDistributionType parameterDistribution ) = 0;

   /// Set the parameter marginal distribution.
   ///
   /// @param distribution The marginal distribution.
   virtual void setMarginalDistributionType( size_t parameterSeqnb, MarginalProbDistr::Type marginalDistributionType ) = 0;

   /// Set the parameter marginal distribution.
   ///
   /// @param distribution The marginal distribution.
   virtual void setMarginalDistributionType( const std::vector<MarginalProbDistr::Type>& marginalDistribution ) = 0;


   /// Set the step method type.
   ///
   /// MetropolisHasting is used to obtain a sample of the posterior parameter
   /// distribution. The survival of the fittest is used to obtain the maximum
   /// likelihood parameter values.
   ///
   /// @param stepMethod The step method to use.
   virtual void setStepMethodType( StepMethod stepMethod ) = 0;

   /// Set Kriging usage: NoMcmcKriging, SmartMcmcKriging, or FullMcmcKriging.
   ///
   /// @param krUsage The indicator specifying Kriging usage.
   virtual void setKrigingUsage( KrigingUsage krUsage ) = 0;

   /// Setter for the kriging type to apply.
   /// @param [in] krigingType the new kriging type value
   virtual void setKrigingType( KrigingType krigingType ) = 0;
   
   /// Setter for the proxy kriging type to apply.
   /// @param [in] krigingType the new kriging type value to be used for the proxy
   virtual void setProxyKrigingType( KrigingType krigingType ) = 0;

   /// Set maximum number of iterations.
   ///
   /// @param maxNbOfIterations The maximum number of iterations.
   virtual void setMaxNbOfIterations( unsigned int maxNbOfIterations ) = 0;

   /// Set the standard deviation factor to be used.
   ///
   /// @param stdDevFactor The new standard deviation factor.
   virtual void setStdDevFactor( double stddevfac ) = 0;

   /// Get the global standard deviation factor.
   ///
   /// @return The global standard deviation factor.
   virtual double getStdDevFactor() const = 0;

   /// Adjust the standard deviation factor assuming a perfect fit.
   ///
   /// Adjust the standard deviation factor assuming a perfect fit, ie use the
   /// square-root of the current chi-square as the standard deviation factor.
   /// This will, for reasonable amounts of data, bring the goodness-of-fit
   /// close to a half. The goodness-of-fit indicator is subsequently not useful
   /// anymore. SUM does however calculate the goodness-of-fit without using the
   /// factor for diagnostic purposes.
   virtual void adaptStdDevFactor() = 0;

   /// Get the values of ySample
   /// @return the values of ySample
   virtual const std::vector<std::vector<double> >& getYSample() const = 0;

   /// Get the values of ySample sorted by RMSE (same order as getBestMatches)
   /// @return the values of ySample sorted by RMSE (same order as getBestMatches)
   virtual const std::vector<std::vector<double> > getSortedYSample() const = 0;
   
   /// Get the average values of pSample
   /// @return the average values of pSample
   virtual const std::vector<double>& getPSampleAvg() const = 0;

   /// Get the average values of pSample per categorical combination
   /// @return the average values of pSample per categorical combination
   virtual const std::vector< std::vector<double> >& getPCatSampleAvgs() const = 0;

   /// Get the covariance values of pSample per categorical combination
   /// @return the covariance values of pSample per categorical combination
   virtual const std::vector<std::vector<std::vector<double> > >& getPCatSampleCovs() const = 0;

   /// Get the best matches
   /// @return the best matches
   virtual const ParameterRanking& getBestMatches() const = 0;

   /// Get the iteration count
   /// @return the iteration count
   virtual unsigned int getIterationCount() = 0;

   /// Get the categorical combinations
   /// @return the categorical combination
   virtual std::vector< std::vector< unsigned int > > getCatCombi() = 0;


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
   virtual unsigned int execute() = 0;

   /// Performs a single iteration in the MCMC process. An iteration loops
   /// over a limited number of cycles each of which performing a limited
   /// number of steps. The step size is tuned such that the last steps of
   /// the cycles can be assumed uncorrelated. To establish a step, cheap
   /// "proposal steps" are generated first until one is accepted.
   ///
   /// @returns the iteration counter (0 if process has converged)
   virtual unsigned int iterateOnce() = 0;

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
   virtual void getP10toP90( P10ToP90Values& values, P10ToP90Parameters& parameters ) = 0;
};

} // namespace SUMlib

#endif // SUMLIB_IMCMC_H
