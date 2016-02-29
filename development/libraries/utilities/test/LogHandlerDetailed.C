//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file LogHandlerDetailed.C
/// @brief This file tests the LogHandler for the Detailed verbosity

#include "LogHandlerUnitTester.h"

///1. INITIALISE TEST--------------------------------------------------------------------------------

//Initialize constants for comparaison
const std::string expectedLog[] =           ///< Expected parsed lines from the log file and console
{
   "MeSsAgE FATAL    This is a fatal error: 0",
   "MeSsAgE ERROR    This is an error: 1",
   "MeSsAgE WARNING  This is a warning: 2",
   "This is an info: 3"
};

//Initialise variables                                 
std::vector<std::string> parsedLinesLog;   ///< Parsed lines from the log file


///2. TEST-------------------------------------------------------------------------------------------
//Test log for detailed verbosity
TEST( LogHandlerDetailed, log_detailed )
{
   try{
      LogHandler logUnitTestDetailed( "log_unit_test_detailed", LogHandler::DETAILED_LEVEL );
      writeLogUnitTest();

      analyzeLogFile( logUnitTestDetailed.getName(), parsedLinesLog );

      EXPECT_EQ( sizeof(expectedLog)/sizeof(std::string), parsedLinesLog.size() );
      for (unsigned int i = 0; i < parsedLinesLog.size(); i++)
      {
         EXPECT_EQ( expectedLog[i], parsedLinesLog[i] );
      }
   }
   catch (const LogHandlerException& ex) {
      FAIL() << "Unexpected exception: " << ex.what();
   }
}
