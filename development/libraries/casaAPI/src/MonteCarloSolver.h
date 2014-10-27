// 
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file MonteCarloSolver.h
/// @brief This file keeps declaration API for performing Monte Carlo simulation

#ifndef CASA_API_MONTE_CARLO_SOLVER_H
#define CASA_API_MONTE_CARLO_SOLVER_H

#include "ErrorHandler.h"

#include <vector>

/// @page CASA_MonteCarloSolverPage Monte Carlo solver
///
/// @link casa::MonteCarloSolver Monte Carlo @endlink simulation generates a set of random parameters vectors:
/// @f$ \left\{ \vec{P_{j}}\right\} =\left\{ \left(p_{1},p_{2},...,p_{n}\right)_j\right\} @f$
/// inside the given parameter's range. For each such randomly generated parameters vector it evaluates the
/// response surface proxy and calculates values for each observable.
/// 
/// Markov Chain Monte Carlo is a more sophisticated algorithm that iteratively converges to the  posterior 
/// probability distribution function. Again, the result is the (narrower) Probability Density Function (PDF) of the observables in terms of 
/// the quantiles. In other words MCMC algorithm generates parameter vectors during sampling only in the areas 
/// of the response surface, which match the reference value of the observables.
///
/// @TODO Describe Monte Carlo based local solver
///
/// SUMlib uses a regression technique (Singular Value Decomposition) to determine a polynomial expression for each observable. This implies 
/// that, generally, the polynomial does not exactly fit the simulated observable output for the run cases. In other words, the polynomial contains
/// regression errors. As these regression errors are known for the run cases, SUMlib uses another technique (Kriging) to interpolate between these 
/// known regression errors. As a result, you have the option to add a Kriging correction on top of the polynomial (can be 0-order only). So any
/// proxy evaluation involves a polynomial evaluation and may involve a Kriging evaluation depending on the setting.
///
/// User can set how the Kriging is used for running the algorithm. If Global_Kriging is set, every step of the algorithm will involve a Kriging 
/// evaluation. If Smart_Kriging is set, some steps of the algorithm will involve a Kriging evaluation.
/// If NoKriging is set (default), all the steps of the algorithm will involve polynomial evaluation only, i.e. no Kriging. Note that this 
/// differs from the Kriging setting of the proxy itself. If Kriging setting of the proxy and proxy usage setting of the algorithm do not match (i.e. 
/// one of them is set to no Kriging, while the other is set to Kriging) both settings will be set to no Kriging. 
///

namespace casa
{  
   class ObsSpace;
   class VarSpace;
   class RSProxy;
   class RunCase;

   /// @brief Monte Carlo solver
   class MonteCarloSolver : public ErrorHandler
   {
   public:
      /// @brief MC sampling point results
      typedef std::pair<double,RunCase*>  MCSamplingPoint;

      /// @brief Set of pairs - (TotRMSE,RunCase) for each Monte Carlo sampling point
      typedef std::vector< MCSamplingPoint > MCResults;

      /// @brief Type of Monte Carlo algorithms implemented in casa::MonteCarloSolver
      enum Algorithm
      {
         MonteCarlo,  ///< (Default) Monte Carlo. No any reference values for observable
         MCMC,        ///< Markov Chain Monte Carlo. At least some observables have reference value
         MCLocSolver  ///< Monte-Carlo based local solver. This algorithm is much faster than MCMC, but does not provide statistical output.
      };

      /// @brief Type of Kriging interpolation which could be used in Monte Carlo
      enum KrigingType
      {
         NoKriging,    ///< (Default) Do not use Kriging. Only polynomial proxies are used, so less accurate but fast sampling.
         SmartKriging, ///< Some steps of the algorithm will involve a Kriging evaluation.
         GlobalKriging ///< Every step of the algorithm will involve a Kriging evaluation.
      };

      /// @brief Distrubution type for measurements
      enum MeasurementDistribution
      {
         NoMeasurements, 
         Normal,         /**< Measurements are assumed to be Gaussian distributed (default). This setting is required
                              if used in combination with the GOF (Goodness Of Fit) indicator, and therefore recommended
                              for the final uncertainty analysis. */
         Robust,         /**< Measurements are assumed to be distributed according to a Laplace distribution which results
                              into less sensitivity to outliers */
         Mixed           /**< Measurements are assumed to be Gaussian distributed for good measurement fits, and according 
                              to a Laplace distribution for outliers. */
      };

      /// @brief How to use parameters PDF
      enum PriorDistribution
      {
         NoPrior,           /**< No prior preference for any value between the bounds of the variable parameter. 
                                 Equivalent to a uniform distribution for all variable parameters.*/
         MarginalPrior,     /**< The prior probability distribution for a single variable parameter independent 
                                 of other variable parameters */
         MultivariatePrior  /**< not supported yet */
      };

      /// @brief Destructor
      virtual ~MonteCarloSolver() {;}

      /// @brief Get Monte Carlo Solver algorithm
      /// @return algorithm ID
      virtual Algorithm algorithm() const = 0;

      /// @brief Get type of kriging interpolation
      /// @return kriging interpolation ID
      virtual KrigingType kriging() const = 0;

      /// @brief Get how to measurements are distributed
      /// @return measurements distribution type
      virtual MeasurementDistribution measurementDistrib() const = 0;

      /// @brief Get how to variable paramters PDF are treated in MC
      /// @return prior distribution type
      virtual PriorDistribution priorDistribution() const = 0;

      /// @brief For MCMC algorithm - get the goodness of fit (GOF) to be displayed (in %). 
      ///        Preferably, the GOF should be larger than about 50%.
      /// @return The goodness of fit.
      virtual double GOF() const = 0;

      /// @brief For MCMC algorithm get standard deviation factor value
      /// @return standard deviation factor value
      virtual double stdDevFactor() const = 0;

      /// @brief For MCMC algorithm get proposed standard deviation factor to keep GOF > 50%
      /// @return proposed standard deviation factor value
      virtual double proposedStdDevFactor() const = 0;
 
      /// @brief Perform Monte Carlo simulation and collect results in one go.
      /// @return ErrorHandler::NoError on success, error code otherwise
      virtual ErrorHandler::ReturnCode runSimulation( RSProxy        & proxy              ///< A response Surface proxy object
                                                    , const VarSpace & proxyVsp           ///< A set of variable parameters which were used to build proxy
                                                    , const VarSpace & mcmcVsp            ///< Additional restrictions on variable parameters range
                                                    , const ObsSpace & obs                ///< Observables set
                                                    , unsigned int     numOfSamples       ///< number of Monte Carlo points to sample
                                                    , unsigned int     maxNumSteps        ///< maximal number of steps
                                                    , double           stdDevFactor = 1.0 ///< used in MCMC to scale std. dev. of observables for better GOF value
                                                    ) = 0;

      /// @brief Perform all neccessary steps for Monte Carlo simulation but do not run calculation itself 
      /// @return ErrorHandler::NoError on success, error code otherwise
      virtual ErrorHandler::ReturnCode prepareSimulation( RSProxy        & proxy              ///< A response Surface proxy object
                                                        , const VarSpace & proxyVsp           ///< A set of variable parameters which were used to build proxy
                                                        , const VarSpace & mcmcVsp            ///< Additional restrictions on variable parameters range
                                                        , const ObsSpace & obs                ///< Observables set
                                                        , unsigned int     numOfSamples       ///< number of Monte Carlo points to sample
                                                        , unsigned int     maxNumSteps        ///< maximal number of steps
                                                        , double           stdDevFactor = 1.0 ///< used in MCMC to scale std. dev. of observables for better GOF value
                                                        ) = 0;

      /// @brief Make one Monte Carlo iteration
      /// @param[out] itNum on return keeps iteration number
      /// @return ErrorHandler::NoError on success, error code otherwise
      virtual ErrorHandler::ReturnCode iterateOnce( int & itNum ) = 0;

      /// @brief Collect all results of Monte Carlo solver run
      /// @return ErrorHandler::NoError on success, error code otherwise
      virtual ErrorHandler::ReturnCode collectMCResults( const VarSpace & proxyVsp ///< A set of variable parameters which were used to build proxy
                                                       , const ObsSpace & obs      ///< Additional restrictions on variable parameters range
                                                       ) = 0;


      /// @brief Get Monte Carlo simulation results
      /// @return array of cases which contains parameters values and observables values calculated through response surface proxy evaluation
      virtual const MCResults & getSimulationResults() = 0;

   protected:
      MonteCarloSolver() {;}

   private:
   };
}

#endif // CASA_API_MONTE_CARLO_SOLVER_H
