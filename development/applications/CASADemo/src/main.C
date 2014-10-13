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

int main( int argc, char ** argv )
{
   if ( argc < 2 )
   {
      std::cout << "Usage: " << argv[0] << " cfgfile.casa" << std::endl;
      return -1;
   }

   try
   {
      // parse args
      const char                  * cmdFileName = 0;
      CasaCommander::VerboseLevel   msgLvl = CasaCommander::Minimal;

      for ( size_t i = 1; i < argc; ++i )
      {
         if ( '-' == argv[i][0] )
         {
            if (      !strcmp( argv[i] + 1, "quite"    ) ) { msgLvl = CasaCommander::Quiet; }
            else if ( !strcmp( argv[i] + 1, "minimal"  ) ) { msgLvl = CasaCommander::Minimal; }
            else if ( !strcmp( argv[i] + 1, "detailed" ) ) { msgLvl = CasaCommander::Detailed; }
            else throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "Unknown option: " << argv[i];
         }
         else
         {
            cmdFileName = argv[i];
         }
      }

      // parse command file
      CfgFileParser  cmdFile;
      CasaCommander  cmdQueue;
   
      cmdFile.parseFile( argv[1], cmdQueue );

      // New scenario object
      casa::ScenarioAnalysis sc;

      //process commands
      cmdQueue.executeCommands( sc );
   }
   catch ( ErrorHandler::Exception & ex )
   {
      std::cerr << "CASA error ID:" << ex.errorCode() << ", message: " << ex.what() << std::endl;
      return -1;
   }
}

