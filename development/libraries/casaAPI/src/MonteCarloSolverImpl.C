//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file MonteCarloSolverImpl.C
/// @brief This file keeps implementation of API for performing Monte Carlo simulation


#include "MonteCarloSolverImpl.h"
#include "ObsSpace.h"
#include "RSProxyImpl.h"
#include "RunCaseImpl.h"
#include "SUMlibUtils.h"
#include "VarSpace.h"
#include "VarPrmContinuous.h"

#include <stdexcept>
#include <climits>

// SUMlib includes
#include "Exception.h"
#include "MC.h"
#include "MCMC.h"
#include "MCSolver.h"
#include "McmcStatistics.h"
#include "ReferenceProxy.h"

namespace casa
{

MonteCarloSolverImpl::MonteCarloSolverImpl( Algorithm algo, KrigingType interp, PriorDistribution priorDist, MeasurementDistribution measureDistr ) 
   : m_algo( algo )
   , m_kriging( interp )
   , m_priorDistr( priorDist )
   , m_measureDistr( measureDistr )
   , m_stdDevFactor( 1.0 )
{
   ;
}

MonteCarloSolverImpl::~MonteCarloSolverImpl()
{
   for ( size_t i = 0; i < m_results.size(); ++i ) delete m_results[i].second;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Creates a new SUMlib mcmc instance and returns it.
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SUMlib::McmcBase * MonteCarloSolverImpl::createMcmc( const SUMlib::CompoundProxyCollection & proxySet
                                                   , const ObsSpace                        & obs
                                                   , const VarSpace                        & varSpace
                                                   , const VarSpace                        & mcmcSpace
                                                   , unsigned int                            numSamples
                                                   )
{
   SUMlib::CompoundProxyCollection::CompoundProxyList const & proxies = proxySet.getProxyList();
   m_parSpace = proxySet.getParameterSpace();

   std::auto_ptr< SUMlib::McmcBase > mcmcBase;
   static const unsigned int seed( 1970-10-16 );
   //static const unsigned int seed( 1966-12-10 );
   static SUMlib::RandomGenerator rng( seed );

   // Convert observables definition CASA->SUMlib
   for ( size_t i = 0; i < m_input.size(); ++i ) delete m_input[i]; // clean any previous runs
   m_input.clear();

   size_t numObsVals = 0; // to handle compound observables
   for ( size_t i = 0; i < obs.size(); ++i )
   {
      const Observable * obv = obs.observable( i );
      if ( obv->hasReferenceValue() )
      {
         const ObsValue * obrv = obv->referenceValue();
         if ( obrv->isDouble() )
         {
            const std::vector<double> & vals = obrv->asDoubleArray();

            for ( size_t k = 0; k < vals.size(); ++k )
            {
               m_input.push_back( new SUMlib::ReferenceProxy( *proxies[numObsVals], vals[k], obv->stdDeviationForRefValue() ) );
               ++numObsVals;
            }
         }
      }
      else
      {
         for ( size_t k = 0; k < obv->dimension(); ++k )
         {
            m_input.push_back( new SUMlib::ReferenceProxy( *proxies[ numObsVals ] ) );
            ++numObsVals;
         }
      }
   }
   assert( numObsVals == proxies.size() );

   // Unscaled min/max/mean/cov but fixed parameters are removed in unscaledPdf
   sumext::convertVarSpace2ParameterPdf( varSpace, m_parSpace, m_unscaledPdf );

   // Scale PDF to [-1:1].
   SUMlib::ParameterPdf scaledPdf ( m_unscaledPdf );
   scaledPdf.scale();

   SUMlib::ParameterBounds boxConstraints;
   sumext::createBoxConstraints( varSpace, mcmcSpace, boxConstraints );

   try
   {
      int mcmcSeed = INT_MAX * rng.uniformRandom();

      switch( m_algo )
      {
         case MonteCarlo:
            mcmcBase.reset( new SUMlib::MC( mcmcSeed, m_input, numSamples, scaledPdf, boxConstraints, m_statistics ) );
            m_stepMethod   = MetropolisHasting;
            m_stdDevFactor = 1.0;
            break;

         case MCMC:
            mcmcBase.reset( new SUMlib::MCMC( mcmcSeed, m_input, numSamples, scaledPdf, boxConstraints, m_statistics ) );
            m_stepMethod = MetropolisHasting;
            break;

         case MCLocSolver:
            mcmcBase.reset( new SUMlib::MCSolver( mcmcSeed, m_input, numSamples, scaledPdf, boxConstraints, m_statistics ) );
            m_stepMethod = MetropolisHasting;
            break;

         default: assert(0); break;
      }
   }
   catch ( const SUMlib::Exception & e )
   {
      throw ErrorHandler::Exception( SUMLibException ) << "SUMlib exception caught: " << e.what();
   }

   return mcmcBase.release();
}


ErrorHandler::ReturnCode MonteCarloSolverImpl::configureSolver( const RSProxy & proxy, const VarSpace & proxyVsp, unsigned int maxNumSteps )
{
   assert( m_mcmc.get() );

   try
   {
      if ( proxy.kriging() == RSProxy::NoKriging && NoKriging != m_kriging )
      {
         throw ErrorHandler::Exception( MonteCarloSolverError ) << "Incompatible kriging type between RSProxy and MC/MCMC";
      }

      // set MC/MCMC configuration
      switch( proxy.kriging() )
      {
         case RSProxy::NoKriging:     m_mcmc->setProxyKrigingType( SUMlib::NoKriging     ); break;
         case RSProxy::LocalKriging:  m_mcmc->setProxyKrigingType( SUMlib::LocalKriging  ); break;
         case RSProxy::GlobalKriging: m_mcmc->setProxyKrigingType( SUMlib::GlobalKriging ); break;
         default: throw ErrorHandler::Exception( MonteCarloSolverError ) << "Unknown RSProxy kriging type";
      }

      switch( m_kriging )
      {
         case NoKriging:     m_mcmc->setKrigingUsage( SUMlib::McmcBase::NoMcmcKriging    ); break;
         case SmartKriging:  m_mcmc->setKrigingUsage( SUMlib::McmcBase::SmartMcmcKriging ); break;
         case GlobalKriging: m_mcmc->setKrigingUsage( SUMlib::McmcBase::FullMcmcKriging  ); break;
         default: throw ErrorHandler::Exception( MonteCarloSolverError ) << "Unknown MC kriging type";
      }

      switch( m_measureDistr )
      {
         case NoMeasurements: m_mcmc->setMeasurementDistributionType( SUMlib::McmcBase::NoMeasurements     ); break;
         case Normal:         m_mcmc->setMeasurementDistributionType( SUMlib::McmcBase::NormalDistribution ); break;
         case Mixed:          m_mcmc->setMeasurementDistributionType( SUMlib::McmcBase::MixedDistribution  ); break;
         case Robust:         m_mcmc->setMeasurementDistributionType( SUMlib::McmcBase::RobustDistribution ); break;
         default: throw ErrorHandler::Exception( MonteCarloSolverError ) << "Unknown measurement distribution type";
      }

      switch( m_priorDistr )
      {
         case NoPrior: m_mcmc->setParameterDistributionType( SUMlib::McmcBase::NoPrior ); break;

         case MarginalPrior:
            {
               m_mcmc->setParameterDistributionType( SUMlib::McmcBase::MarginalDistribution );

               // Only the continuous parameters are subject to a marginal PDF.        
               std::vector< SUMlib::MarginalProbDistr::Type > sumPdfTypes;
               for ( size_t i = 0; i < proxyVsp.numberOfContPrms(); ++i )
               {
                  const VarPrmContinuous * prm = proxyVsp.continuousParameter( i );
                  for ( size_t j = 0; j < prm->dimension(); ++j )
                  {
                     switch ( prm->pdfType() )
                     {
                        case VarPrmContinuous::Block:    sumPdfTypes.push_back( SUMlib::MarginalProbDistr::Uniform    ); break;
                        case VarPrmContinuous::Triangle: sumPdfTypes.push_back( SUMlib::MarginalProbDistr::Triangular ); break;
                        case VarPrmContinuous::Normal:   sumPdfTypes.push_back( SUMlib::MarginalProbDistr::Normal     ); break;
                        default: assert( 0 ); break;
                     }
                  }
               }
               m_mcmc->setMarginalDistributionType( sumPdfTypes );
            }
            break;

         case MultivariatePrior: m_mcmc->setParameterDistributionType( SUMlib::McmcBase::MultivariateGaussianDistribution ); break;

         default: throw ErrorHandler::Exception( MonteCarloSolverError ) << "Unknown prior distribution type";
      }
      
      switch( m_stepMethod )
      {
         case MetropolisHasting: m_mcmc->setStepMethodType( SUMlib::McmcBase::MetropolisHasting ); break;
         case SurvivalOfTheFittest: m_mcmc->setStepMethodType( SUMlib::McmcBase::SurvivalOfTheFittest ); break;
         default: throw ErrorHandler::Exception( MonteCarloSolverError ) << "Unknown step method";
      }

      m_mcmc->setStdDevFactor( m_stdDevFactor );
      m_mcmc->setMaxNbOfIterations( maxNumSteps );
   }
   catch ( const SUMlib::Exception & e )
   {
      return reportError( SUMLibException, e.what() );
   }
   catch ( const ErrorHandler::Exception & ex )
   {
      return reportError( ex.errorCode(), ex.what() );
   }

   return NoError;
}


// Get the goodness of fit (GOF) to be displayed (in %). Preferably, the GOF should be larger than about 50%.
double MonteCarloSolverImpl::GOF() const
{
   return m_statistics.getGoodnessOfFitReduced();
}

// For MCMC algorithm get proposed standard deviation factor to keep GOF > 50%
double MonteCarloSolverImpl::proposedStdDevFactor() const
{
   return MCMC == m_algo ? sqrt( m_statistics.getChi2() ) : 1.0;
}
 

ErrorHandler::ReturnCode MonteCarloSolverImpl::runSimulation( RSProxy        & proxy
                                                            , const VarSpace & proxyVsp
                                                            , const VarSpace & mcmcVsp
                                                            , const ObsSpace & obs
                                                            , unsigned int     numOfSamples
                                                            , unsigned int     maxNumSteps
                                                            , double           stdDevFactor 
                                                            )
{
   if ( NoError != prepareSimulation( proxy, proxyVsp, mcmcVsp, obs, numOfSamples, maxNumSteps, stdDevFactor ) ) return errorCode();
   try
   {
      m_mcmc->execute();
   }
   catch ( const SUMlib::Exception & e )
   {
      return reportError( SUMLibException, e.what() );
   }
   return collectMCResults( proxyVsp, obs );
}

// Perform all neccessary steps for Monte Carlo simulation but do not run calculation itself 
ErrorHandler::ReturnCode MonteCarloSolverImpl::prepareSimulation( RSProxy        & proxy
                                                                , const VarSpace & proxyVsp           
                                                                , const VarSpace & mcmcVsp            
                                                                , const ObsSpace & obs                
                                                                , unsigned int     numOfSamples       
                                                                , unsigned int     maxNumSteps        
                                                                , double           stdDevFactor
                                                                )
{
   RSProxyImpl & rsp = dynamic_cast<RSProxyImpl &>( proxy );
   m_stdDevFactor = stdDevFactor;

   if ( !rsp.getProxyCollection() ) return reportError( ErrorHandler::NonexistingID, "Unprepared proxy given for MC/MCMC" );

   m_mcmc.reset( createMcmc( *(rsp.getProxyCollection()), obs, proxyVsp, mcmcVsp, numOfSamples ) );

   if ( NoError != configureSolver( proxy, proxyVsp, maxNumSteps ) ) return errorCode();

   return NoError; 
}


// Make one Monte Carlo iteration
ErrorHandler::ReturnCode MonteCarloSolverImpl::iterateOnce( int & itNum )
{
   if ( !m_mcmc.get() ) return reportError( MonteCarloSolverError, "Solver is not initialized." );
   try
   {
      itNum = m_mcmc->iterateOnce();
   }
   catch ( const SUMlib::Exception & e )
   {
      return reportError( SUMLibException, e.what() );
   }
   return NoError;
}

// Collect all results of Monte Carlo solver run
ErrorHandler::ReturnCode MonteCarloSolverImpl::collectMCResults( const VarSpace & proxyVsp, const ObsSpace & obs )
{
   if ( !m_mcmc.get() ) return reportError( MonteCarloSolverError, "Solver is not initialized." );
   // Get MC output.
   if ( !m_results.empty() ) // clean previous run results
   {
      for ( size_t i = 0; i < m_results.size(); ++i ) delete m_results[i].second;
      m_results.clear();
   }

   // Process parameters value for each sampling point sorted by RMSE
   const SUMlib::McmcBase::ParameterRanking & prmVals = m_mcmc->getBestMatches();
   std::vector<SUMlib::Case>                  prmCases;
      
   for ( SUMlib::McmcBase::ParameterRanking::const_iterator it = prmVals.begin(); it != prmVals.end(); ++it )
   {
      SUMlib::Case cs;
      m_parSpace.unprepare( it->second, cs );

      m_results.push_back( MCSamplingPoint( it->first, new RunCaseImpl() ) );
      sumext::convertCase( cs, proxyVsp, *m_results.back().second );
   }

   // process observables value for each sampling point
   const std::vector<SUMlib::ProxyValueList> & obsVals = m_mcmc->getSortedYSample();
   assert( obsVals.size() == m_results.size() );

   for ( size_t i = 0; i < obsVals.size(); ++i )
   {
      sumext::convertObservablesValue( obsVals[i], obs, *(m_results[i].second) );
   }

   return NoError;
}

}