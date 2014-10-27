//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

#include "CasaCommander.h"
#include "CmdRunMC.h"

#include "casaAPI.h"

#include <cstdlib>
#include <cmath>
#include <iostream>

CmdRunMC::CmdRunMC( CasaCommander & parent, const std::vector< std::string > & cmdPrms ) : CasaCmd( parent, cmdPrms )
{
   // set default values
   m_mcAlg        = casa::MonteCarloSolver::MonteCarlo;
   m_samplesNum   = 500;
   m_maxStepsNum  = 100;
   m_proxyUsage   = casa::MonteCarloSolver::NoKriging;
   m_varPrmPDFEval= casa::MonteCarloSolver::NoPrior;
   
   assert( m_prms.size() > 0 ); // at least proxy name must be here

   // process command parameters
   m_proxyName = m_prms[0];

   if ( m_proxyName.empty() ) throw ErrorHandler::Exception( ErrorHandler::UndefinedValue ) << "Response surface proxy name must be specified";

   if ( m_prms.size() > 1 )
   {
      if (      m_prms[1] == "MC"          ) { m_mcAlg = casa::MonteCarloSolver::MonteCarlo;  }
      else if ( m_prms[1] == "MCMC"        ) { m_mcAlg = casa::MonteCarloSolver::MCMC;        }
      else if ( m_prms[1] == "MCLocSolver" ) { m_mcAlg = casa::MonteCarloSolver::MCLocSolver; }
      else
      {
         throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "Unknown name of Monte Carlo simulation algorithm: " << m_prms[1] << 
                                                                         ", must be MC | MCMC | MCLocSolver";
      
      }
   }

   if ( m_prms.size() > 2 ) m_samplesNum  = atol( m_prms[2].c_str() );
   if ( m_prms.size() > 3 ) m_maxStepsNum = atol( m_prms[3].c_str() );
   
   if ( m_prms.size() > 4 )
   {
      ///< how to evaluate proxy - POLYNOMIAL/SMART_KRIGING/FULL_KRIGING
      if (      m_prms[4] == "Polynomial"    ) { m_proxyUsage = casa::MonteCarloSolver::NoKriging;     }
      else if ( m_prms[4] == "SmartKriging"  ) { m_proxyUsage = casa::MonteCarloSolver::SmartKriging;  }
      else if ( m_prms[4] == "GlobalKriging" ) { m_proxyUsage = casa::MonteCarloSolver::GlobalKriging; }
      else
      {
         throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "Unknown way of proxy evaluation: " << m_prms[4] << 
                                                                   ", must be Polynomial | SmartKriging | GlobalKriging";
      }
   }

   if ( m_prms.size() > 5 )
   {
      if (      m_prms[5] == "NoPrior"           ) m_varPrmPDFEval = casa::MonteCarloSolver::NoPrior;
      else if ( m_prms[5] == "MarginalPrior"     ) m_varPrmPDFEval = casa::MonteCarloSolver::MarginalPrior;
      else if ( m_prms[5] == "MultivariatePrior" ) m_varPrmPDFEval = casa::MonteCarloSolver::MultivariatePrior;
      else
      {
         throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "Unknown distribution type: " << m_prms[5] << 
                             ", must be NoPriorDistribution | MarginalPriorDistribution | MultivariatePriorDistribution";
      }
   }

   // read standard deviation factor which will be used for RMSE calculation
   if ( m_prms.size() > 6 ) m_stdDevFactor = atof( m_prms[6].c_str() );
}

void CmdRunMC::execute( casa::ScenarioAnalysis & sa )
{
   if ( m_commander.verboseLevel() > CasaCommander::Quiet )
   {
      std::cout << "Running: ";
      switch( m_mcAlg )
      {
         case casa::MonteCarloSolver::MonteCarlo:  std::cout << "Monte Carlo";                    break;
         case casa::MonteCarloSolver::MCMC:        std::cout << "Markov Chain Monte Carlo";       break;
         case casa::MonteCarloSolver::MCLocSolver: std::cout << "Monte Carlo based local solver"; break;
      }
      std::cout << " ..." << std::endl;
   }
   
   // create corresponded MC algorithm
   if ( ErrorHandler::NoError != sa.setMCAlgorithm( static_cast<casa::MonteCarloSolver::Algorithm>( m_mcAlg ),
                                                    static_cast<casa::MonteCarloSolver::KrigingType>( m_proxyUsage ),
                                                    static_cast<casa::MonteCarloSolver::PriorDistribution>( m_varPrmPDFEval ),
                                                    casa::MonteCarloSolver::Normal )
      )
   {
      throw ErrorHandler::Exception( sa.errorCode() ) << sa.errorMessage();
   }

   // look for the proxy:
   casa::RSProxy * proxy = sa.rsProxySet().rsProxy( m_proxyName );
   if ( !proxy ) throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "Can't find response proxy with name: " << m_proxyName;

   // run simulation itself
   if ( ErrorHandler::NoError != sa.mcSolver().prepareSimulation( *proxy, sa.varSpace(), sa.varSpace(), sa.obsSpace(), m_samplesNum, m_maxStepsNum, m_stdDevFactor ) )
   {
      throw ErrorHandler::Exception( sa.mcSolver().errorCode() ) << sa.mcSolver().errorMessage();
   }
   
   std::cout << "Performed iterations: ";

   while( true )
   {
      int itNum;

      if ( ErrorHandler::NoError != sa.mcSolver().iterateOnce( itNum ) )
      {
         throw ErrorHandler::Exception( sa.mcSolver().errorCode() ) << sa.mcSolver().errorMessage();
      }
      if ( !itNum ) break; // converged
      
      if ( m_commander.verboseLevel() > CasaCommander::Quiet )
      {
         if ( itNum > 1 )
         {
            if (      itNum < 10   ) std::cout << "\b";
            else if ( itNum < 100  ) std::cout << ( itNum > 10   ? "\b\b"     : "\b"   );
            else if ( itNum < 1000 ) std::cout << ( itNum > 100  ? "\b\b\b"   : "\b\b" );
            else                     std::cout << ( itNum > 1000 ? "\b\b\b\b" : "\b\b\b" );
         }
         std::cout << itNum;
         std::cout.flush();
      }
   }

   if ( ErrorHandler::NoError != sa.mcSolver().collectMCResults( sa.varSpace(), sa.obsSpace() ) )
   {
      throw ErrorHandler::Exception( sa.mcSolver().errorCode() ) << sa.mcSolver().errorMessage();
   }

   if ( m_commander.verboseLevel() > CasaCommander::Quiet )
   {
      std::cout << "\nSucceeded." << std::endl;
   }
}

