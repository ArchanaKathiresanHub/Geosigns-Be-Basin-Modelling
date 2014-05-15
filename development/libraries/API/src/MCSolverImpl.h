// 
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file MCSolverImpl.h
/// @brief This file keeps declaration implementation of API for performing Monte Carlo simulation

#ifndef CASA_API_MONTE_CARLO_SOLVER_IMPL_H
#define CASA_API_MONTE_CARLO_SOLVER_IMPL_H

#include "MCSolver.h"

namespace casa
{   
   /// Monte Carlo solver implementation
   class MCSolverImpl : public MCSolver
   {
   public:

      // Constructor / Destructor
      MCSolverImpl();
      virtual ~MCSolverImpl();

      // Start Monte Carlo simulation
      // numerOfSampl number of Monte Carlo points to sample
      // proxy A response Surface proxy object
      // varPrmsSet A set of variable parameters with ranges
      // return ErrorHandler::NoError on success, error code otherwise
      virtual ErrorHandler::ReturnCode runSimulation( int numerOfSampl, const RSProxy & proxy, const VarSpace & varPrmsSet );

      // Get Monte Carlo simulation results
      // return array of cases which contains parameters values and observables values calculated through response surface proxy evaluation
      virtual std::vector<RunCase*> & getSimulationResults();

   protected:

   private:
      std::vector<RunCase*> m_results;
   };
}

#endif // CASA_API_MONTE_CARLO_SOLVER_IMPL_H
