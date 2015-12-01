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

// Boost Log library
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>

void initLog( CasaCommander::VerboseLevel lev )
{
   boost::log::add_file_log
   ( "scenario.log"
   , boost::log::keywords::format = "[%TimeStamp%]: %Message%"
   );

   boost::log::add_console_log
   ( std::cout
   , boost::log::keywords::format = "%Message%"
   );

   switch ( lev )
   {
      case CasaCommander::Quiet:    boost::log::core::get()->set_filter( boost::log::trivial::severity >= boost::log::trivial::error ); break;
      case CasaCommander::Detailed: boost::log::core::get()->set_filter( boost::log::trivial::severity >= boost::log::trivial::debug ); break;
      case CasaCommander::Minimal:  
      default: boost::log::core::get()->set_filter( boost::log::trivial::severity >= boost::log::trivial::info ); break;
   }

   boost::log::add_common_attributes();
}

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
   std::auto_ptr<casa::ScenarioAnalysis> sc( new casa::ScenarioAnalysis() );

   try
   {
      // parse args
      for ( size_t i = 1; i < argc; ++i )
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
      initLog( msgLvl );

      // parse command file
      CfgFileParser  cmdFile;
   
      cmdFile.parseFile( cmdFileName, cmdQueue );

      //process commands
      cmdExecutionStarted = true;
      cmdQueue.executeCommands( sc );
   }
   catch ( const std::runtime_error & ex )
   {
      BOOST_LOG_TRIVIAL( fatal ) << "Command file " << cmdFileName << " execution error: " << "   SUMlib error: " << ex.what() <<
                                    ", CASA command \"" << cmdQueue.curCmdName() << "\" at line: " << cmdQueue.curCmdInputFileLineNumber();
      return -1;
   }
   catch ( const ErrorHandler::Exception & ex )
   {
      if ( cmdExecutionStarted )
      {
         BOOST_LOG_TRIVIAL( error ) << "Exception on processing command: " << cmdQueue.curCmdName() << " located at line " << 
            cmdQueue.curCmdInputFileLineNumber() << " of input file " << cmdFileName << ". ";
         sc->runManager().stopAllSubmittedJobs();
      }
      BOOST_LOG_TRIVIAL( fatal ) << "CASA error ID:" << ex.errorCode() << ", message: " << ex.what();
      return -1;
   }
   catch ( ... )
   {
      if ( cmdExecutionStarted )
      {
         BOOST_LOG_TRIVIAL( error ) << "Exception on processing command: " << cmdQueue.curCmdName() << " located at line " << 
            cmdQueue.curCmdInputFileLineNumber() << " of input file " << cmdFileName << ". ";
         sc->runManager().stopAllSubmittedJobs();
      }
      BOOST_LOG_TRIVIAL( fatal ) << "CASA unknown exception, aborting...";
      return -1;
   }
}

