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

// CASA
#include "CasaDeserializer.h"
#include "MonteCarloSolverImpl.h"
#include "ObsSpace.h"
#include "RSProxyImpl.h"
#include "RunCaseImpl.h"
#include "SUMlibUtils.h"
#include "VarSpace.h"
#include "VarPrmContinuous.h"

// STL
#include <stdexcept>

// STD C library
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
   , m_GOF( 0.0 )
{
   ;
}

MonteCarloSolverImpl::~MonteCarloSolverImpl()
{
   for ( size_t i = 0; i < m_results.size(); ++i ) delete m_results[i].second;
   for ( size_t i = 0; i < m_input.size();   ++i ) delete m_input[i];
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
      int mcmcSeed = static_cast<int>( INT_MAX * rng.uniformRandom() );

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
   return m_GOF;
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

      std::auto_ptr<RunCaseImpl> mcCase( new RunCaseImpl() );

      sumext::convertCase( cs, proxyVsp, *mcCase.get() );
      mcCase->setRunStatus( RunCase::Completed ); // MC cases always completed

      m_results.push_back( MCSamplingPoint( it->first, mcCase.release() ) );
   }

   // process observables value for each sampling point
   const std::vector<SUMlib::ProxyValueList> & obsVals = m_mcmc->getSortedYSample();
   assert( obsVals.size() == m_results.size() );

   for ( size_t i = 0; i < obsVals.size(); ++i )
   {
      sumext::convertObservablesValue( obsVals[i], obs, *(m_results[i].second) );
   }
   
   m_GOF = m_statistics.getGoodnessOfFitReduced();

   // collect P10-P90 CDF
   SUMlib::McmcBase::P10ToP90Parameters cases;
   m_mcmc->getP10toP90( m_cdf, cases );

   return NoError;
}

// Serialize object to the given stream
bool MonteCarloSolverImpl::save( CasaSerializer & sz, unsigned int fileVersion ) const
{
   bool ok = true;

   if ( fileVersion >= 0 )
   {
      ok = ok ? sz.save( static_cast<int>(m_algo),         "Algo"        ) : ok;
      ok = ok ? sz.save( static_cast<int>(m_kriging),      "Kriging"     ) : ok;
      ok = ok ? sz.save( static_cast<int>(m_priorDistr),   "PriorDistr"  ) : ok;
      ok = ok ? sz.save( static_cast<int>(m_measureDistr), "MeasurDistr" ) : ok;
      ok = ok ? sz.save( static_cast<int>(m_stepMethod),   "StepMethod"  ) : ok;
      ok = ok ? sz.save( m_stdDevFactor,                   "StdDevFact"  ) : ok;

      // save MC results
      ok = ok ? sz.save( m_results.size(), "ResultsSetSize" ) : ok;
      for ( size_t i = 0; i < m_results.size() && ok; ++i )
      {
         ok = sz.save( m_results[i].first, "RMSEVal" );
         ok = ok ? sz.save( *(m_results[i].second), "MCRunCase" ) : ok;
      }
   }

   if ( fileVersion >= 1 )
   {
      ok = sz.save( m_GOF, "GOF" );

      // save P10-P90 CDF per observable
      ok = ok ? sz.save( m_cdf.size(), "CDFSetSize" ) : ok;
      for ( size_t i = 0; i < m_cdf.size() && ok; ++i )
      {
         ok = sz.save( m_cdf[i], "ObsCDF" );
      }
   }

   // save SUMlib needed data structures
   // TODO implement saving of SUMlib data structures
   /*ok = ok ? sz.save( m_input.size(), "McmcProxySetSize" ) : ok;
   for ( size_t i = 0; i < m_input.size() && ok; ++i )
   {
      ok = sz.save( *m_input[i], "McmcProxy" );
   }
   */
   ok = ok ? sz.save( m_parSpace, "ParameterSpace" ) : ok;

   // SUMlib::McmcStatistics           m_statistics; TODO implement save/load
   // SUMlib::ParameterPdf             m_unscaledPdf; TODO implemetn save/load
   // std::auto_ptr<SUMlib::McmcBase>  m_mcmc;          // TODO implement SUMlib MC/MC/MC Solver object itself 

   return ok;
}

// Create a new instance and deserialize it from the given stream
MonteCarloSolverImpl::MonteCarloSolverImpl( CasaDeserializer & dz, const char * objName )
{
   // read from file object name and version
   std::string  objNameInFile;
   std::string  objType;
   unsigned int objVer;

   bool ok = dz.loadObjectDescription( objType, objNameInFile, objVer );
   if ( objType.compare( this->typeName() ) || objNameInFile.compare( objName ) )
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
         << "Deserialization error. Can not load object: " << objName;
   }

   if ( version() < objVer )
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
         << "Version of object in file is newer. No forward compatibility!";
   }

   int val;
   ok = ok ? dz.load( val, "Algo" ) : ok;
   m_algo = static_cast<Algorithm>(val);
   
   ok = ok ? dz.load( val, "Kriging" ) : ok;
   m_kriging = static_cast<KrigingType>(val);

   ok = ok ? dz.load( val, "PriorDistr" ) : ok;
   m_priorDistr = static_cast<PriorDistribution>(val);

   ok = ok ? dz.load( val, "MeasurDistr" ) : ok;
   m_measureDistr = static_cast<MeasurementDistribution>(val);

   ok = ok ? dz.load( val,   "StepMethod"  ) : ok;
   m_stepMethod = static_cast<StepMethod>(val);
   
   ok = ok ? dz.load( m_stdDevFactor, "StdDevFact" ) : ok;

   // load MC results
   size_t setSize;
   ok = ok ? dz.load( setSize, "ResultsSetSize" ) : ok;
   for ( size_t i = 0; i < setSize && ok; ++i )
   {  
      double val;
      ok = dz.load( val, "RMSEVal" );
      RunCaseImpl * rco = ok ? new RunCaseImpl( dz, "MCRunCase" ) : 0;
      if ( ok ) m_results.push_back( std::pair<double, RunCase*>( val, rco ) );
   }

   if ( objVer > 0 )
   {
      ok = ok ? dz.load( m_GOF, "GOF" ) : ok;

      // load P10-P90 CDF per observable
      ok = ok ? dz.load( setSize, "CDFSetSize" ) : ok;
      for ( size_t i = 0; i < setSize && ok; ++i )
      {
         std::vector<double> cdf;
         ok = dz.load( cdf, "ObsCDF" );
         m_cdf.push_back( cdf );
      }
   }

   // TODO implement loading of SUMlib data structures
   /* ok = ok ? sz.save( setSize, "McmcProxySetSize" ) : ok;
   for ( size_t i = 0; i < setSize && ok; ++i )
   {
      SUMlib::McmcProxy * po = new SUMlib::McmcProxy();
      ok = ok ? dz.load( *po, "McmcProxy" );
   }
   */
   ok = ok ? dz.load( m_parSpace, "ParameterSpace" ) : ok;

   // SUMlib::McmcStatistics           m_statistics; TODO implement save/load
   // SUMlib::ParameterPdf             m_unscaledPdf; TODO implemetn save/load
   // std::auto_ptr<SUMlib::McmcBase>  m_mcmc;          // TODO implement SUMlib MC/MC/MC Solver object itself 

   if ( !ok ) throw Exception( DeserializationError ) << "MonteCarloSolverImpl deserialization error";
}

}
