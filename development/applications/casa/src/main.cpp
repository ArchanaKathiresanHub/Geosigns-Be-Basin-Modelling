//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

// CASA API
#include "casaAPI.h"

// CASA APP
#include "CasaCommander.h"
#include "CfgFileParser.h"

// STL
#include <stdexcept>

// Log library
#include "LogHandler.h"

int main( int argc, char ** argv )
{
   if ( argc < 2 )
   {
      std::cout << "Usage: " << argv[0] << " [options] cfgfile.casa\n";
      std::cout << " options are:\n";
      std::cout << "    -quiet|minimal|detailed - set level of printed on screen information to off/minimal/full\n";
      std::cout << "    -help print help page about CASADemo commands and exit\n";
      std::cout << "\n    To stop execution of casa scenario - create in the current folder (where casa app was started)\n";
      std::cout << "    an empty  file with the name: \"" << casa::RunManager::s_scenarioExecStopFileName << "\".\n";
      return -1;
   }

   const char                  * cmdFileName = 0;
   CasaCommander::VerboseLevel   msgLvl = CasaCommander::Minimal;
   CasaCommander                 cmdQueue;
   bool                          cmdExecutionStarted = false; // to distinct execution error from input file parsing errors

   // New scenario object
   std::unique_ptr<casa::ScenarioAnalysis> sc( new casa::ScenarioAnalysis() );

   try
   {
      // parse args
      for ( int i = 1; i < argc; ++i )
      {
         if ( '-' == argv[i][0] )
         {
            if (      !strcmp( argv[i] + 1, "quiet"    ) ) { msgLvl = CasaCommander::Quiet; }
            else if ( !strcmp( argv[i] + 1, "minimal"  ) ) { msgLvl = CasaCommander::Minimal; }
            else if ( !strcmp( argv[i] + 1, "detailed" ) ) { msgLvl = CasaCommander::Detailed; }
            else if ( !strcmp( argv[i] + 1, "help"     ) ) { CasaCommander::printHelpPage( (i+1 < argc ? argv[i+1] : "") ); exit(0); }
            else throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "Unknown option: " << argv[i];
         }
         else
         {
            cmdFileName = argv[i];
         }
      }
      
      // Set up loggin for casa app
      switch ( msgLvl )
      {
         case CasaCommander::Quiet:    LogHandler( "scenario.log", LogHandler::QUIET_LEVEL );      break;
         case CasaCommander::Detailed: LogHandler( "scenario.log", LogHandler::DIAGNOSTIC_LEVEL ); break;
         case CasaCommander::Minimal:  
         default:                      LogHandler( "scenario.log", LogHandler::DETAILED_LEVEL );   break;
      }
      
      // parse command file
      CfgFileParser  cmdFile;
   
      cmdFile.parseFile( cmdFileName, cmdQueue );

      //process commands
      cmdExecutionStarted = true;
      cmdQueue.executeCommands( sc );
   }
   catch ( const std::runtime_error & ex )
   {
      LogHandler( LogHandler::FATAL_SEVERITY ) << "Command file " << cmdFileName << " execution error: " << "   SUMlib error: " << ex.what() <<
                                    ", CASA command \"" << cmdQueue.curCmdName() << "\" at line: " << cmdQueue.curCmdInputFileLineNumber();
      return -1;
   }
   catch ( const ErrorHandler::Exception & ex )
   {
      if ( cmdExecutionStarted )
      {
         LogHandler( LogHandler::ERROR_SEVERITY ) << "Exception on processing command: " << cmdQueue.curCmdName() << " located at line " << 
            cmdQueue.curCmdInputFileLineNumber() << " of input file " << cmdFileName << ". ";
         sc->runManager().stopAllSubmittedJobs();
      }
      LogHandler( LogHandler::FATAL_SEVERITY ) << "CASA error ID:" << ex.errorCode() << ", message: " << ex.what();
      return -1;
   }
   catch ( ... )
   {
      if ( cmdExecutionStarted )
      {
         LogHandler( LogHandler::ERROR_SEVERITY ) << "Exception on processing command: " << cmdQueue.curCmdName() << " located at line " << 
            cmdQueue.curCmdInputFileLineNumber() << " of input file " << cmdFileName << ". ";
         sc->runManager().stopAllSubmittedJobs();
      }
      LogHandler( LogHandler::FATAL_SEVERITY ) << "CASA unknown exception, aborting...";
      return -1;
   }
}

