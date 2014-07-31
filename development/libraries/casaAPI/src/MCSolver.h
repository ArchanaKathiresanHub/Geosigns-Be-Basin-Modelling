// 
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file MCSolver.h
/// @brief This file keeps declaration API for performing Monte Carlo simulation

#ifndef CASA_API_MONTE_CARLO_SOLVER_H
#define CASA_API_MONTE_CARLO_SOLVER_H

#include "ErrorHandler.h"
#include "RSProxy.h"
#include "VarSpace.h"

#include <vector>

/// @page CASA_MCSolverPage Monte Carlo solver
///
/// @link casa::MCSolver Monte Carlo @endlink simulation generates a set of random parameters vectors:
/// @f$ \left\{ \vec{P_{j}}\right\} =\left\{ \left(p_{1},p_{2},...,p_{n}\right)_j\right\} @f$
/// inside the given parameter's range. For each such randomly generated parameters vector it evaluates the
/// response surface proxy and calculates values for each observable.
/// 
/// Markov Chain Monte Carlo is a more sophisticated algorithm that iteratively converges to the  posterior 
/// probability distribution function. Again, the result is the (narrower) Probability Density Function (PDF) of the observables in terms of 
/// the quantiles. In other words MCMC algorithm generates parameter vectors during sampling only in the areas 
/// of the response surface, which match the reference value of the observables.

namespace casa
{   
   /// @brief Monte Carlo solver
   class MCSolver : public ErrorHandler
   {
   public:
      /// @brief Type of Monte Carlo algorithms implemented in casa::MCSolver
      enum MCAlgorithm
      {
         MonteCarlo,           ///< Monte Carlo
         MarkovChainMonteCarlo ///< Markov Chain Monte Carlo
      };

      /// @brief Type of Kriging interpolation which could be used in Monte Carlo
      enum MCKrigingType
      {
         NoKriging,
         LocalKriging,
         GlobalKriging
      };

      /// @brief Destructor
      virtual ~MCSolver() {;}

      /// @brief Start Monte Carlo simulation
      /// @param numerOfSampl number of Monte Carlo points to sample
      /// @param proxy A response Surface proxy object
      /// @param varPrmsSet A set of variable parameters with ranges
      /// @return ErrorHandler::NoError on success, error code otherwise
      virtual ErrorHandler::ReturnCode runSimulation( int numerOfSampl, const RSProxy & proxy, const VarSpace & varPrmsSet ) = 0;

      /// @brief Get Monte Carlo simulation results
      /// @return array of cases which contains parameters values and observables values calculated through response surface proxy evaluation
      virtual std::vector<RunCase*> & getSimulationResults() = 0;

   protected:
      MCSolver() {;}

   private:
   };
}

#endif // CASA_API_MONTE_CARLO_SOLVER_H
