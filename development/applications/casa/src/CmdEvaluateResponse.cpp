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
#include "CmdEvaluateResponse.h"
#include "CfgFileParser.h"
#include "MatlabExporter.h"

#include "casaAPI.h"
#include "VarPrmContinuous.h"
#include "VarPrmCategorical.h"
#include "RunCaseImpl.h"

#include "LogHandler.h"

#include <cstdlib>
#include <iostream>
#include <memory>

CmdEvaluateResponse::CmdEvaluateResponse( CasaCommander & parent, const std::vector< std::string > & cmdPrms ) : CasaCmd( parent, cmdPrms )
{
   if ( m_prms.size() != 3 )
   {
      throw ErrorHandler::Exception( ErrorHandler::RSProxyError ) << "Wrong parameters number: " 
         << m_prms.size() << " (expected 3) in response proxy " << (m_prms.size() > 0 ? (m_prms[0] + " ") : "" ) << "evaluation command";
   }

   m_proxyName = m_prms[0];    // get proxy name
   if ( m_proxyName.empty() ) throw ErrorHandler::Exception( ErrorHandler::UndefinedValue ) << "No proxy name was given";
   
   // convert list of DoEs or data files like: "Tornado,BoxBenken" into array of names
   m_expList = CfgFileParser::list2array( m_prms[1], ',' );
   if ( m_expList.empty() ) throw ErrorHandler::Exception( ErrorHandler::UndefinedValue ) << "No any DoE or data file name was given";

   m_dataFileName = m_prms[2]; // output file name
   if ( m_proxyName.empty() ) throw ErrorHandler::Exception( ErrorHandler::UndefinedValue ) << "No output file name was specified";
}

void CmdEvaluateResponse::createRunCasesSet( std::auto_ptr<casa::ScenarioAnalysis> & sa
                                           , std::vector<casa::RunCase *>          & rcs
                                           , const std::vector<std::string>        & expList
                                           , std::vector<size_t>                   & sizePerExp
                                           )
{
   sizePerExp.clear();
   for ( size_t e = 0; e < expList.size(); ++e )
   {
      sizePerExp.push_back(0);

      sa->doeCaseSet().filterByExperimentName( expList[e] );
      if ( sa->doeCaseSet().size() ) // DoE name was given, add cases from DoE
      {
         for ( size_t j = 0; j < sa->doeCaseSet().size(); ++j )
         {
            const casa::RunCase * rc = sa->doeCaseSet()[j];
            // create new RunCase and make a shallow copy of parameters using shared pointers
            std::auto_ptr<casa::RunCase> nrc( new casa::RunCaseImpl() );
            for ( size_t k = 0; k < rc->parametersNumber(); ++k ) nrc->addParameter( rc->parameter( k ) );

            // add new case to the list
            rcs.push_back( nrc.release() );
            sizePerExp[e] += 1;
         }
      }
      else // file name is given, parse file and create a set of run cases
      {
         std::vector< std::vector<double> > prmVals;
         CfgFileParser::readParametersValueFile( expList[e], prmVals );
         casa::VarSpace & vs = sa->varSpace();

         for ( size_t i = 0; i < prmVals.size(); ++i )
         {
            std::auto_ptr<casa::RunCase> nrc( new casa::RunCaseImpl() );

            std::vector<double>::const_iterator vit = prmVals[i].begin();
   
            for ( size_t j = 0; j < vs.size(); ++j )
            {
               assert( vit != prmVals[i].end() );

               const casa::VarParameter * vprm = vs.parameter( j );
               switch( vprm->variationType() )
               {
                  case casa::VarParameter::Continuous:
                     {
                        const casa::VarPrmContinuous * cntVprm = dynamic_cast<const casa::VarPrmContinuous *>( vprm );
                        SharedParameterPtr prm = cntVprm->newParameterFromDoubles( vit );
                        nrc->addParameter( prm );
                        break;
                     }

                  case casa::VarParameter::Categorical:
                     {
                        const casa::VarPrmCategorical * catVprm = dynamic_cast<const casa::VarPrmCategorical *>( vprm );
                        unsigned int val = static_cast<unsigned int>( *vit );
                        ++vit;
                        SharedParameterPtr prm = catVprm->createNewParameterFromUnsignedInt( val );
                        nrc->addParameter( prm );
                        break;
                     }

                  default:
                     throw ErrorHandler::Exception( ErrorHandler::NotImplementedAPI ) << "Discrete parameter is not implemented yet";
                     break;
               }
            }
            rcs.push_back( nrc.release() );
         }
         sizePerExp[e] += 1;
      }
   }
   sa->doeCaseSet().filterByExperimentName( "" );
}

void CmdEvaluateResponse::execute( std::auto_ptr<casa::ScenarioAnalysis> & sa )
{
   std::vector<casa::RunCase *> rcs; // set of run cases which were created from set of parameters defined in external dat file
   std::vector<size_t> casePerExp;

   createRunCasesSet( sa, rcs, m_expList, casePerExp ); // create new set of run cases with parameters value

   // Search for given proxy name in the set of calculated proxies
   casa::RSProxy * proxy = sa->rsProxySet().rsProxy( m_proxyName.c_str() );
   // call response evaluation
   if ( !proxy ) { throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "Unknown proxy name:" << m_proxyName; }

   LogHandler( LogHandler::INFO ) << "Evaluating proxy " << m_proxyName << " for " << rcs.size() << " cases...";

   size_t i = 0;
   for ( size_t e = 0; e < m_expList.size(); ++e )
   {
      LogHandler( LogHandler::DEBUG ) << "Evaluate proxy for " << m_expList[e]  << "DoE/data file for " << casePerExp[e]  << " cases...";

      for ( size_t c = 0; c < casePerExp[e]; ++c )
      {
         assert( i < rcs.size() );

         if ( ErrorHandler::NoError != proxy->evaluateRSProxy( *rcs[i] ) )
         {
            throw ErrorHandler::Exception( proxy->errorCode() ) << proxy->errorMessage();
         }
         ++i;
      }
   }

   LogHandler( LogHandler::INFO ) << "Exporting proxy evaluation results to " << m_dataFileName << "file...";

   MatlabExporter::exportObsValues( m_dataFileName, rcs );

   for ( size_t i = 0; i < rcs.size(); ++i ) delete rcs[i]; // clean cases created here

   LogHandler( LogHandler::INFO ) << "Proxy evalutaion was succeeded";
}

