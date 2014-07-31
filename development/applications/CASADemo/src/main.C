#include "cmbAPI.h"
#include "casaAPI.h"
#include "FilePath.h"
#include "FolderPath.h"
#include "ObsValueDoubleScalar.h"
#include "PrmSourceRockTOC.h"
#include "PrmTopCrustHeatProduction.h"
#include "RunCase.h"
#include "RunCaseSet.h"
#include "RunManagerImpl.h"
#include "VarPrmSourceRockTOC.h"
#include "VarPrmTopCrustHeatProduction.h"

#include <memory>
#include <string>
#include <iostream>

#include <cstdlib>

#include "CfgFileParser.h"


using namespace casa;

int Usage( const char * name )
{
   std::cout << "Usage: " << name << " cfgfile.casa" << std::endl;
   return -1;
}

int ReportError( ErrorHandler & errHnd )
{
   std::cerr << "CASA error: " << errHnd.errorCode() << ", " << errHnd.errorMessage() << std::endl;
   return -1;
}

ErrorHandler::ReturnCode AddVarParameter( casa::ScenarioAnalysis & sc, const std::vector< std::string > & prms )
{
   VarPrmContinuous::PDF ppdf = VarPrmContinuous::Block;

   if ( prms[0] == "TopCrustHeatProduction" )
   {
      assert( prms.size() == 4 );

      double minVal = atof( prms[1].c_str() );
      double maxVal = atof( prms[2].c_str() );
      
      VarPrmContinuous::PDF ppdf = VarPrmContinuous::Block;

      if (      prms[3] == "Block"    ) { ppdf = VarPrmContinuous::Block;    } // equal PDF
      else if ( prms[3] == "Triangle" ) { ppdf = VarPrmContinuous::Triangle; } // triangle PDF
      else if ( prms[3] == "Normal"   ) { ppdf = VarPrmContinuous::Normal;   } // gauss PDF
     
      return BusinessLogicRulesSet::VaryTopCrustHeatProduction( sc, minVal, maxVal, ppdf );
   }
   else if ( prms[0] == "SourceRockTOC" )
   {
      assert( prms.size() == 5 );

      double minVal = atof( prms[2].c_str() );
      double maxVal = atof( prms[3].c_str() );
 
      if (      prms[4] == "Block"    ) { ppdf = VarPrmContinuous::Block;    } // equal PDF
      else if ( prms[4] == "Triangle" ) { ppdf = VarPrmContinuous::Triangle; } // triangle PDF
      else if ( prms[4] == "Normal"   ) { ppdf = VarPrmContinuous::Normal;   } // gauss PDF
   
      return BusinessLogicRulesSet::VarySourceRockTOC( sc, prms[1].c_str(), minVal,  maxVal,  ppdf );
   }
   
   return sc.reportError( ErrorHandler::UndefinedValue, std::string( "Unknown variable parameter name: ") + prms[0] );
}


ErrorHandler::ReturnCode AddObservable( casa::ScenarioAnalysis & sc, const std::vector< std::string > & prms )
{
   if ( prms[0] == "XYZPoint" )
   {
      assert( prms.size() == 10 );

      double x      = atof( prms[2].c_str() );
      double y      = atof( prms[3].c_str() );
      double z      = atof( prms[4].c_str() );
      double age    = atof( prms[5].c_str() );
      double refVal = atof( prms[6].c_str() );
      double stdDev = atof( prms[7].c_str() );
      double wgtSA  = atof( prms[8].c_str() );
      double wgtUA  = atof( prms[9].c_str() );

      casa::Observable * xyzVal = DataDigger::newObsPropertyXYZ( x, y, z, prms[1].c_str(), age );
      xyzVal->setReferenceValue( new ObsValueDoubleScalar( xyzVal, refVal ), stdDev );
      xyzVal->setSAWeight( wgtSA );
      xyzVal->setUAWeight( wgtUA );
  
      if ( sc.obsSpace().addObservable( xyzVal ) != ErrorHandler::NoError ) return sc.moveError( sc.obsSpace() );

      return ErrorHandler::NoError;
   }
   
   return sc.reportError( ErrorHandler::UndefinedValue, std::string( "Unknown variable parameter name: ") + prms[0] );
}
               
void PrintObsValues( casa::ScenarioAnalysis & sc )
{
  
   ObsSpace   & obSpace = sc.obsSpace();
   RunCaseSet & rcSet   = sc.doeCaseSet();

   for ( size_t rc = 0; rc < rcSet.size(); ++rc )
   {
      // go through all run cases
      const RunCase * cs = rcSet[rc];
      
      if ( !cs ) continue;

      std::cout << "    " << cs->projectPath() << std::endl;
      std::cout << "    Observable values:" << std::endl;

      for ( size_t i = 0; i < cs->observablesNumber(); ++i )
      {
         casa::ObsValue * ov = cs->observableValue( i );
         if ( ov && ov->observable() && ov->isDouble() ) 
         {
            std::cout << "      " << ov->observable()->name() << " = " << ov->doubleValue() << std::endl;
         }
      }
   }
}


int main( int argc, char ** argv )
{
   if ( argc < 2 ) return Usage( argv[0] );
   
   CfgFileParser  cmdFile;
   
   cmdFile.parseFile( argv[1] );
   //std::cout << cmdFile;

   casa::ScenarioAnalysis sc;

   //process commands
   for ( size_t i = 0; i < cmdFile.cmdsNum(); ++i )
   {
      const std::vector< std::string > & prms = cmdFile.parameters( i );

      switch( cmdFile.cmd( i ) )
      {
         case CfgFileParser::app: // add new application
            {
               std::cout << "Add cauldron application to calculation pipeline " << prms[0] << "(";
               for ( size_t p = 1; p < prms.size(); ++p ) std::cout << prms[p] << ((p == prms.size()-1) ? "" : ",");
               std::cout << ")" << std::endl;

               CauldronApp * app = NULL;
               size_t p = 1;

               if (      prms[0] == "fastcauldron" ) app = RunManager::createApplication( RunManager::fastcauldron );
               else if ( prms[0] == "fastgenex6"   ) app = RunManager::createApplication( RunManager::fastgenex6 );
               else if ( prms[0] == "fastctc"      ) app = RunManager::createApplication( RunManager::fastctc );
               else if ( prms[0] == "fasttouch7"   ) app = RunManager::createApplication( RunManager::fasttouch7 );
               else if ( prms[0] == "fastmig"      ) app = RunManager::createApplication( RunManager::fastmig );
               else if ( prms[0] == "track1d"      ) app = RunManager::createApplication( RunManager::track1d );
               else if ( prms[0] == "generic"      ) app = RunManager::createApplication( RunManager::generic, atof( prms[p++].c_str() ), prms[p++] );

               for ( ; p < prms.size(); ++p ) app->addOption( prms[p] );
               
               casa::RunManager & rm = sc.runManager();
               if ( ErrorHandler::NoError != rm.addApplication( app ) ) return ReportError( rm );

            }
            break;

         case CfgFileParser::base_project: // add base project name
            {
               std::cout << "Set base case: " << prms[0] << std::endl;
               if( ErrorHandler::NoError != sc.defineBaseCase( prms[0].c_str() ) ) return ReportError( sc );
            }
            break;

         case CfgFileParser::doe:
            {  
               std::cout << "Generating " << prms[0] << " DoE";
               int numRuns = 0;
               if (      prms[0] == "Tornado"              ) { sc.setDoEAlgorithm( DoEGenerator::Tornado    );           }
               else if ( prms[0] == "BoxBehnken"           ) { sc.setDoEAlgorithm( DoEGenerator::BoxBehnken );           }
               else if ( prms[0] == "PlackettBurman"       ) { sc.setDoEAlgorithm( DoEGenerator::PlackettBurman );       }
               else if ( prms[0] == "PlackettBurmanMirror" ) { sc.setDoEAlgorithm( DoEGenerator::PlackettBurmanMirror ); }
               else if ( prms[0] == "FullFactorial"        ) { sc.setDoEAlgorithm( DoEGenerator::FullFactorial );        }
               else if ( prms[0] == "LatinHypercube"       )
               {
                  sc.setDoEAlgorithm( DoEGenerator::LatinHypercube );
                  if ( prms.size() > 1 )
                  {
                     numRuns = atol( prms[1].c_str() );
                  }
               }
               else if ( prms[0] == "SpaceFilling"         )
               {
                  if ( ErrorHandler::NoError != sc.setDoEAlgorithm( DoEGenerator::SpaceFilling ) ) return ReportError( sc );
                  if ( prms.size() > 1 ) { numRuns = atol( prms[1].c_str() ); }
               }

               if( ErrorHandler::NoError != sc.doeGenerator().generateDoE( sc.varSpace(), sc.doeCaseSet(), numRuns ) ) return ReportError( sc.doeGenerator() );

               std::cout << " for " << sc.doeCaseSet().size() << " cases" << std::endl;
            }
            break;
            
         case CfgFileParser::location:
            {
               std::cout << "Generating the set of case in folder: " << prms[0] << std::endl;
               if ( ErrorHandler::NoError != sc.setScenarioLocation( prms[0].c_str() ) ) return ReportError( sc );
               if ( ErrorHandler::NoError != sc.applyMutations(      sc.doeCaseSet() ) ) return ReportError( sc );
               if ( ErrorHandler::NoError != sc.dataDigger().requestObservables( sc.obsSpace(), sc.doeCaseSet() ) ) return ReportError( sc );
            }
            break;

         case CfgFileParser::varprm:
            std::cout << "Add variable parameter: " << prms[0] << "(";
            for ( size_t i = 1; i < prms.size(); ++i )
            { 
               std::cout << prms[i] << ((i == prms.size()-1) ? "" : "," );
            }
            std::cout << ")" << std::endl;

            if ( ErrorHandler::NoError != AddVarParameter( sc, prms ) ) { return ReportError( sc ); }
            break;

         case CfgFileParser::target:
            std::cout << "Add observable: " << prms[0] << "(";
            for ( size_t i = 1; i < prms.size(); ++i )
            { 
               std::cout << prms[i] << ((i == prms.size()-1) ? "" : "," );
            }
            std::cout << ")" << std::endl;

            if ( ErrorHandler::NoError != AddObservable( sc, prms ) ) { return ReportError( sc ); }
            break;


         case CfgFileParser::run:
            {
               assert( prms.size() == 2 );
               
               std::cout << "Adding jobs to the queue and generating scripts for:" << std::endl;
               
               casa::RunManager & rm = sc.runManager();
               rm.setCauldronVersion( prms[1].c_str() ); 
               rm.setClusterName( prms[0].c_str() );

               for ( size_t i = 0; i < sc.doeCaseSet().size(); ++i )
               {
                  std::cout << "    " << (sc.doeCaseSet()[i])->projectPath() << std::endl;
                  if ( ErrorHandler::NoError != rm.scheduleCase( *(sc.doeCaseSet()[i]) ) ) { return ReportError( rm ); }
               }
               std::cout << "Submitting jobs to the cluster " << prms[0] << " using Cauldron: " << prms[1] << std::endl;
               if ( ErrorHandler::NoError != rm.runScheduledCases( false ) ) { return ReportError( rm ); }
               
               if( sc.dataDigger().collectRunResults( sc.obsSpace(), sc.doeCaseSet() ) != ErrorHandler::NoError ) return ReportError( sc );
               PrintObsValues( sc );
            }
            break;
      }
   }
}

