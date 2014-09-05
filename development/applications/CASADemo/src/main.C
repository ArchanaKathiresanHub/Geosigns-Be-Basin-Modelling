#include "cmbAPI.h"
#include "casaAPI.h"
#include "FilePath.h"
#include "FolderPath.h"
#include "ObsValueDoubleScalar.h"
#include "ObsValueDoubleArray.h"
#include "RunCase.h"
#include "RunCaseSet.h"
#include "RunManagerImpl.h"
#include "VarPrmSourceRockTOC.h"
#include "VarPrmTopCrustHeatProduction.h"

#include <memory>
#include <string>
#include <iostream>
#include <fstream>

#include <cstdlib>

#include "CfgFileParser.h"

std::string g_baseCaseName;
std::string g_location;
std::string g_dataFileName = "casa_data.m";

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

VarPrmContinuous::PDF Str2pdf( const std::string & pdf )
{
   if (      pdf == "Block"    ) return VarPrmContinuous::Block;    // equal PDF
   else if ( pdf == "Triangle" ) return VarPrmContinuous::Triangle; // triangle PDF
   else if ( pdf == "Normal"   ) return VarPrmContinuous::Normal;   // gauss PDF
   
   return VarPrmContinuous::Block;
}

ErrorHandler::ReturnCode AddVarParameter( casa::ScenarioAnalysis & sc, const std::vector< std::string > & prms )
{
   VarPrmContinuous::PDF ppdf = VarPrmContinuous::Block;

   if ( prms[0] == "TopCrustHeatProduction" )
   {
      assert( prms.size() == 4 );

      double minVal = atof( prms[1].c_str() );
      double maxVal = atof( prms[2].c_str() );
      
      ppdf = Str2pdf( prms[3] );
     
      return BusinessLogicRulesSet::VaryTopCrustHeatProduction( sc, minVal, maxVal, ppdf );
   }
   else if ( prms[0] == "SourceRockTOC" )
   {
      assert( prms.size() == 5 );

      double minVal = atof( prms[2].c_str() );
      double maxVal = atof( prms[3].c_str() );
 
      ppdf = Str2pdf( prms[3] );
   
      return BusinessLogicRulesSet::VarySourceRockTOC( sc, prms[1].c_str(), minVal,  maxVal,  ppdf );
   }
   else if ( prms[0] == "CrustThinningOneEvent" )
   {
      assert( prms.size() == 10 );

      // Initial crustal thickness
      double minCrustThickn = atof(  prms[1].c_str() );
      double maxCrustThickn = atof(  prms[2].c_str() );

      // Start thinning time
      double minTStart = atof( prms[3].c_str() );
      double maxTStart = atof( prms[4].c_str() );

      // Thinning duraration
      double minDeltaT = atof( prms[5].c_str() );
      double maxDeltaT = atof( prms[6].c_str() );

      double minFactor = atof( prms[7].c_str() );
      double maxFactor = atof( prms[8].c_str() );

      VarPrmContinuous::PDF pdfType = Str2pdf( prms[9] );
      
      return BusinessLogicRulesSet::VaryOneCrustThinningEvent( sc, minCrustThickn, maxCrustThickn, 
                                                                   minTStart,      maxTStart, 
                                                                   minDeltaT,      maxDeltaT,
                                                                   minFactor,      maxFactor, pdfType );
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
      double age    = atof( prms[5].c_str() ); // age for the observable
      double refVal = atof( prms[6].c_str() ); // observable reference value
      double stdDev = atof( prms[7].c_str() ); // std deviation value
      double wgtSA  = atof( prms[8].c_str() ); // observable weight for Sensitivity Analysis
      double wgtUA  = atof( prms[9].c_str() ); // observable weight for Uncertainty Analysis

      casa::Observable * xyzVal = ObsSpace::newObsPropertyXYZ( x, y, z, prms[1].c_str(), age );
      xyzVal->setReferenceValue( new ObsValueDoubleScalar( xyzVal, refVal ), stdDev );
      xyzVal->setSAWeight( wgtSA );
      xyzVal->setUAWeight( wgtUA );
  
      if ( sc.obsSpace().addObservable( xyzVal ) != ErrorHandler::NoError ) return sc.moveError( sc.obsSpace() );

      return ErrorHandler::NoError;
   }
   else if ( prms[0] == "WellTraj" ) // file  format X Y Z RefVal
   {
      assert( prms.size() == 7 );
      const std::string & trajFileName = prms[1];                 // well trajectory file with reference values
      const std::string & propName     = prms[2];                 // property name
      double              age          = atof( prms[3].c_str() ); // age for the observable
      double              stdDev       = atof( prms[4].c_str() ); // std deviation value
      double              wgtSA        = atof( prms[5].c_str() ); // observable weight for Sensitivity Analysis
      double              wgtUA        = atof( prms[6].c_str() ); // observable weight for Uncertainty Analysis

      // read trajectory file
      std::vector<double> x;
      std::vector<double> y;
      std::vector<double> z;
      std::vector<double> r;
      CfgFileParser::readTrajectoryFile( trajFileName, x, y, z, r );
      casa::Observable * wellVal = ObsSpace::newObsPropertyWell( x, y, z, propName.c_str(), age ); 
      wellVal->setReferenceValue( new ObsValueDoubleArray( wellVal, r ), stdDev );
      wellVal->setSAWeight( wgtSA );
      wellVal->setUAWeight( wgtUA );
 
      if ( sc.obsSpace().addObservable( wellVal ) != ErrorHandler::NoError ) return sc.moveError( sc.obsSpace() );

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
            const std::vector<double> & vals = ov->doubleValue();
            std::cout << "      " << ov->observable()->name() << " = (" ;
            for ( size_t i = 0; i < vals.size(); ++i )
            {
               std::cout << " " << vals[i];
            }
            std::cout << " )" << std::endl;
         }
      }
   }
}

void SaveResults( casa::ScenarioAnalysis & sc )
{
   std::ofstream ofs( g_dataFileName.c_str(), std::ios_base::out | std::ios_base::trunc );
   
   if ( ofs.bad() ) return;

   ofs << "BaseCaseName  = '" << g_baseCaseName << "';\n";
   ofs << "DoEName       = '" << DoEGenerator::DoEName( sc.doeGenerator().algorithm() ) << "';\n";
   ofs << "PathToCaseSet = '" << g_location << "';\n";

   RunCaseSet & rcs = sc. doeCaseSet();

   if ( !rcs.size() ) return;

   // save parameters value for each case
   // First save name of the variable parameters
   ofs << "ParametersName = [\n";
   for ( size_t j = 0; j < rcs[0]->parametersNumber(); ++j )
   {
      casa::Parameter * prm = rcs[0]->parameter( j );
      if ( !prm  ) continue;
      ofs << "    \"" << prm->name() << "\"\n";      
   }
   ofs << "];\n\n";

   // Second - values for parameters for each case
   ofs << "ParametersVal = [\n";
 
   for ( size_t i = 0; i < rcs.size(); ++i )
   {
      ofs << i;  // Case number

      for ( size_t j = 0; j < rcs[i]->parametersNumber(); ++j )
      {
         casa::Parameter * prm = rcs[i]->parameter( j );
         
         if ( !prm  ) continue;

         const std::vector<double> & prmVals = prm->asDoubleArray();

         for ( size_t k = 0; k < prmVals.size(); ++k )
         {
            ofs << "\t" << prmVals[k];
         }
      }
      ofs << "\n";
   }
   ofs << "];\n\n";

   // save observables value for each case
   // First save name of the observables
   ofs << "ObservablesName = [\n";
   for ( size_t j = 0; j < rcs[0]->observablesNumber(); ++j )
   {
      casa::ObsValue * obv = rcs[0]->observableValue( j );
      
      if ( !obv || !obv->isDouble()  ) continue;  // skip observables which is not double

      ofs << "    \"" << obv->observable()->name() << "\"\n";      
   }
   ofs << "];\n\n";

   // Second - value for observables for each case
   ofs << "ObservablesDim = [ ";
   for ( size_t j = 0; j < rcs[0]->observablesNumber(); ++j )
   {
      casa::ObsValue * obv = rcs[0]->observableValue( j );
      
      if ( !obv || !obv->isDouble()  ) continue;  // skip observables which is not double

      ofs << " " << obv->doubleValue().size();      
    }
    ofs << " ];\n\n";

   ofs << "ObservablesVal = [\n";
   for ( size_t i = 0; i < rcs.size(); ++i )
   {
      ofs << i;

      for ( size_t j = 0; j < rcs[i]->observablesNumber(); ++j )
      {
         casa::ObsValue * obv = rcs[i]->observableValue( j );

         if ( obv && obv->isDouble() )
         {
            const std::vector<double> & vals = obv->doubleValue();
            for ( size_t k = 0; k < vals.size(); ++k ) ofs << "\t" << vals[k];
         }
      }

      ofs << std::endl;
   }
   ofs << "];\n\n";

   // Third - observables MSE for each case
   ofs << "ObservablesMSE = [\n";
   for ( size_t i = 0; i < rcs.size(); ++i )
   {
      ofs << i;

      for ( size_t j = 0; j < rcs[i]->observablesNumber(); ++j )
      {
         casa::ObsValue * obv = rcs[i]->observableValue( j );

         if ( obv && obv->isDouble() )
         {
            ofs << "\t" << obv->MSE();
         }
      }

      ofs << std::endl;
   }
   ofs << "];\n";
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
               g_baseCaseName = prms[0];
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
               g_location = prms[0];
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
   std::cout << "Export CASA results to : " << g_dataFileName << std::endl;
   SaveResults( sc );
}

