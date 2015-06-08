//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 
#include "casaAPI.h"

#include "CasaCommander.h"
#include "CfgFileParser.h"

#include <stdexcept>

int main( int argc, char ** argv )
{
   if ( argc < 2 )
   {
      std::cout << "Usage: " << argv[0] << " [options] cfgfile.casa\n";
      std::cout << " options are:\n";
      std::cout << "    -quiet|minimal|detailed - set level of printed on screen information to off/minimal/full\n";
      std::cout << "    -help print help page about CASADemo commands and exit\n";
      return -1;
   }

   const char                  * cmdFileName = 0;
   CasaCommander::VerboseLevel   msgLvl = CasaCommander::Minimal;
   CasaCommander                 cmdQueue;
   bool                          cmdExecutionStarted = false; // to distinct execution error from input file parsing errors

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

      // parse command file
      CfgFileParser  cmdFile;
   
      cmdFile.parseFile( argv[1], cmdQueue );

      // New scenario object
      std::auto_ptr<casa::ScenarioAnalysis> sc( new casa::ScenarioAnalysis() );

      //process commands
      cmdExecutionStarted = true;
      cmdQueue.executeCommands( sc );
   }
   catch ( const std::runtime_error & ex )
   {
      std::cerr << "Command file " << cmdFileName << " execution error: " << std::endl;
      std::cerr << "   SUMlib error: " << ex.what() << std::endl;
      std::cerr << "   CASA command \"" << cmdQueue.curCmdName() << "\" at line: " << cmdQueue.curCmdInputFileLineNumber() << std::endl;
      return -1;
   }
   catch ( ErrorHandler::Exception & ex )
   {
      if ( cmdExecutionStarted )
      {
         std::cerr << "Exception on processing command: " << cmdQueue.curCmdName() << " located at line " << 
            cmdQueue.curCmdInputFileLineNumber() << " of input file " << cmdFileName << std::endl;
      }
      std::cerr << "CASA error ID:" << ex.errorCode() << ", message: " << ex.what() << std::endl;
      return -1;
   }
   catch ( ... )
   {
      if ( cmdExecutionStarted )
      {
         std::cerr << "Exception on processing command: " << cmdQueue.curCmdName() << " located at line " << 
            cmdQueue.curCmdInputFileLineNumber() << " of input file " << cmdFileName << std::endl;
      }
       std::cerr << "CASA unknown exception, aborting..." << std::endl;
      return -1;
   }
}

