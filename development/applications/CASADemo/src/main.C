#include "cmbAPI.h"
#include "casaAPI.h"
#include "FilePath.h"
#include "FolderPath.h"
#include "PrmSourceRockTOC.h"
#include "PrmTopCrustHeatProduction.h"
#include "RunManagerImpl.h"
#include "VarPrmSourceRockTOC.h"
#include "VarPrmTopCrustHeatProduction.h"

#include <memory>
#include <string>
#include <iostream>

#include <cstdlib>

using namespace casa;

int Usage( const char * name )
{
   std::cout << "Usage: " << name << " base-case.project3d" << std::endl;
   return -1;
}

int ReportError( ErrorHandler & errHnd )
{
   std::cerr << "CASA error: " << errHnd.errorCode() << ", " << errHnd.errorMessage() << std::endl;
   return -1;
}

int main( int argc, char ** argv )
{
   if ( argc < 2 ) return Usage( argv[0] );

   double minTOC = 5.0;
   double maxTOC = 15.0;
   double minTCHP = 0.1;
   double maxTCHP = 4.9;
   const char * layerName = "Lower Jurassic";
   const char * projectFileName = argv[1];

   ibs::FolderPath pathToCaseSet = ibs::FolderPath( getenv( "HOME" ) );
   pathToCaseSet << "CasaDemo_CaseSet";

   casa::ScenarioAnalysis sc;

   sc.defineBaseCase( projectFileName );
   
   // vary 2 parameters
   BusinessLogicRulesSet::VarySourceRockTOC(          sc, layerName, minTOC,  maxTOC,  VarPrmContinuous::Block );
   BusinessLogicRulesSet::VaryTopCrustHeatProduction( sc,            minTCHP, maxTCHP, VarPrmContinuous::Block );

   // set up and generate DoE
   sc.setDoEAlgorithm( DoEGenerator::Tornado );
   casa::DoEGenerator & doe = sc.doeGenerator( );

   doe.generateDoE( sc.varSpace(), sc.doeCaseSet() );
   
   if ( ErrorHandler::NoError != sc.setScenarioLocation( pathToCaseSet.path().c_str() ) ) return ReportError( sc );

   if ( ErrorHandler::NoError != sc.applyMutations( sc.doeCaseSet() ) ) return ReportError( sc );

   RunManagerImpl & rm = dynamic_cast< RunManagerImpl &> ( sc.runManager() );
 
   // set up simulation pipeline, the first is fastcauldron
   CauldronApp * app = RunManager::createApplication( RunManager::fastcauldron );
   app->addOption( "-itcoupled" );
   rm.addApplication( app );

   // then set up fastgenex6
   app = RunManager::createApplication( RunManager::fastgenex6 );
   rm.addApplication( app );

   // and at the end set up track1d
   app = RunManager::createApplication( RunManager::track1d );
   app->addOption( "-coordinates 460001,6750001" );
   app->addOption( "-age 0.0" );
   app->addOption( "-properties Temperature,Vr,OilExpelledCumulative,HcGasExpelledCumulative,TOC" );
   rm.addApplication( app );

   for ( size_t i = 0; i < sc.doeCaseSet().size(); ++i )
   {
      if ( ErrorHandler::NoError != rm.scheduleCase( *(sc.doeCaseSet()[i]) ) ) { return ReportError( rm ); }
   }

   if ( ErrorHandler::NoError != rm.runScheduledCases( false ) ) { return ReportError( rm ); }
}

