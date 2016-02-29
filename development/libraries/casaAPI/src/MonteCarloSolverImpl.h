//
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/// @file MonteCarloSolverImpl.h
/// @brief This file keeps declaration implementation of API for performing Monte Carlo simulation

#ifndef CASA_API_MONTE_CARLO_SOLVER_IMPL_H
#define CASA_API_MONTE_CARLO_SOLVER_IMPL_H

// CASA
#include "MonteCarloSolver.h"

// SUMlib
#include "McmcBase.h"
#include "McmcStatistics.h"

// STL
#include <memory>

namespace SUMlib
{
   class McmcBase;
   class MCAlgorithmBase;
   class CompoundProxyCollection;
   class McmcProxy;
   class ParameterPdf;
}

namespace casa
{
   /// Monte Carlo solver implementation
   class MonteCarloSolverImpl : public MonteCarloSolver
   {
   public:

      // Constructor / Destructor
      MonteCarloSolverImpl( Algorithm               algo        = MonteCarloSolver::MonteCarlo // Monte Carlo algorithm type
                          , KrigingType             interp      = MonteCarloSolver::NoKriging  // Which Kriging interpolation to use
                          , PriorDistribution       priorDist   = MonteCarloSolver::NoPrior    // How to use variable parameter PDF. If NoPrior - uniform block PDF
                          , MeasurementDistribution measureDist = MonteCarloSolver::Normal     // How measurements are distributed
                          );

      virtual ~MonteCarloSolverImpl();

      // Get Monte Carlo Solver algorithm
      virtual Algorithm algorithm() const { return m_algo; };

      // Get type of kriging interpolation
      virtual KrigingType kriging() const { return m_kriging; }

      // Get how to measurements are distributed
      virtual MeasurementDistribution measurementDistrib() const { return m_measureDistr; }

      // Get how to variable paramters PDF are treated in MC
      virtual PriorDistribution priorDistribution() const { return m_priorDistr; }

      // Get the goodness of fit (GOF) to be displayed (in %). Preferably, the GOF should be larger than about 50%.
      virtual double GOF() const;

      // For MCMC algorithm get standard deviation factor value
      virtual double stdDevFactor() const { return m_stdDevFactor; }

      // For MCMC algorithm get proposed standard deviation factor to keep GOF > 50%
      virtual double proposedStdDevFactor() const;

      // Start Monte Carlo simulation
      virtual ErrorHandler::ReturnCode runSimulation( RSProxy        & proxy        // A Response Surface proxy object
                                                    , const VarSpace & proxyVsp     // A set of variable parameters which were used to build proxy
                                                    , const VarSpace & mcmcVsp      // Additional restrictions on variable parameters range
                                                    , const ObsSpace & obs          // Observables set
                                                    , unsigned int     numOfSamples // Number of Monte Carlo points to sample
                                                    , unsigned int     maxNumSteps  // Maximal number of steps
                                                    , double           stdDevFactor // used in MCMC to scale standard deviations of observables for better GOF value
                                                    );

      // Perform all neccessary steps for Monte Carlo simulation but do not run calculation itself
      virtual ErrorHandler::ReturnCode prepareSimulation( RSProxy        & proxy              // A response Surface proxy object
                                                        , const VarSpace & proxyVsp           // A set of variable parameters which were used to build proxy
                                                        , const VarSpace & mcmcVsp            // Additional restrictions on variable parameters range
                                                        , const ObsSpace & obs                // Observables set
                                                        , unsigned int     numOfSamples       // number of Monte Carlo points to sample
                                                        , unsigned int     maxNumSteps        // maximal number of steps
                                                        , double           stdDevFactor = 1.0 // used in MCMC to scale standard deviations of observables for better GOF value
                                                        );

      // Make one Monte Carlo iteration
      virtual ErrorHandler::ReturnCode iterateOnce( int & itNum );

      // Collect all results of Monte Carlo solver run
      virtual ErrorHandler::ReturnCode collectMCResults( const VarSpace & proxyVsp // A set of variable parameters which were used to build proxy
                                                       , const ObsSpace & obs      // Additional restrictions on variable parameters range
                                                       );

      // Get Monte Carlo simulation results
      // return array of cases which contains parameters values and observables values calculated through response surface proxy evaluation
      virtual const MCResults & getSimulationResults() { return m_results; }

      // Get CDF (cumulative density function) for P10-P90 quantiles
      // return vector size 9 which keeps CDF values for 10-20-30-40-50..-90% probability for each observable
      virtual const std::vector<std::vector<double> > & p10p90CDF() const { return m_cdf; }

      // Get number of samplings generated by MC simulation
      virtual size_t samplingsNumber() const { return m_results.size(); }

      // Get RMSE value for the i-th sampling
      virtual double RMSE( size_t i ) const { return i > m_results.size() ? 0.0 : m_results[i].first; }

      // Get RunCase object for i-th sampling point. RunCase keeps
      // parameters value generated by MonteCarlo solver and evaluated by RSProxy observables values
      virtual const RunCase * samplingPoint( size_t i ) const { return i > m_results.size() ? 0 : m_results[i].second; }

      // Serialization / Deserialization

      // version of serialized object representation
      virtual unsigned int version() const { return 0; }

      // Serialize object to the given stream
      virtual bool save( CasaSerializer & sz, unsigned int version ) const;

      // Get type name of the serialaizable object, used in deserialization to create object with correct type
      virtual const char * typeName() const { return "MonteCarloSolverImpl"; }

      // Create a new instance and deserialize it from the given stream
      MonteCarloSolverImpl( CasaDeserializer & inStream, const char * objName );

   private:
      // the parameter sample
      typedef std::vector< std::vector< double > > SampleParameterVector;

      enum StepMethod // depend on MC algorithm. For MC/MCMC - MetropolisHasting is used. For MCLocSolver - SurvivalOfTheFittest.
      {
         MetropolisHasting,
         SurvivalOfTheFittest
      };

      SUMlib::McmcBase * createMcmc( const SUMlib::CompoundProxyCollection & proxySet, const ObsSpace & obs,
                                     const VarSpace & proxyVs, const VarSpace & mcmcVs, unsigned int numSamples );

      ErrorHandler::ReturnCode configureSolver( const RSProxy & proxy, const VarSpace & proxyVsp, unsigned int maxNumSteps );

      MCResults                          m_results;       // Set of pairs - total RMSE + case generated by MC/MCMC simulation
      std::vector< std::vector<double> > m_cdf;           // P10-P90 CDF function for each observable

      double                             m_GOF;           // Goddnes of fit for MCMC

      Algorithm                          m_algo;          // type of MC solver - MC/MCMC/Based MC local solver
      KrigingType                        m_kriging;       // use kriging interpolation
      PriorDistribution                  m_priorDistr;    // use of ignore variable parameters PDF
      MeasurementDistribution            m_measureDistr;  // how measurements are distributed
      StepMethod                         m_stepMethod;    // which step method solver will use
      double                             m_stdDevFactor;  // used to scale standard deviation values to getting good Godness of Fit in MCMC

      // SUMlib needed data structures
      std::vector<SUMlib::McmcProxy*>  m_input;
      SUMlib::McmcStatistics           m_statistics;
      SUMlib::ParameterPdf             m_unscaledPdf;
      SUMlib::ParameterSpace           m_parSpace;
      std::unique_ptr<SUMlib::McmcBase>  m_mcmc;          // SUMlib MC/MC/MC Solver object itself
   };
}

#endif // CASA_API_MONTE_CARLO_SOLVER_IMPL_H
