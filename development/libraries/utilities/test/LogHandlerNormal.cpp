//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/// @file LogHandlerNormal.cpp
/// @brief This file tests the LogHandler for the Normal verbosity

#include "LogHandlerUnitTester.h"

///1. INITIALISE TEST--------------------------------------------------------------------------------

//Initialize constants for comparaison
const std::string expectedLog[] =                ///< Expected parsed lines from the log file and console
{
   "Basin_Fatal:    This is a fatal error: 0",
   "Basin_Error:    This is an error: 1",
   "Basin_Warning:  This is a warning: 2"
};

//Initialise variables
std::vector<std::string> parsedLinesLog;         ///< Parsed lines from the log file

///2. TEST-------------------------------------------------------------------------------------------
//Tests log for normal verbosity
TEST( LogHandlerSerial, log_normal )
{
   try{
      LogHandler logUnitTestNormal( "log_unit_test_normal", LogHandler::NORMAL_LEVEL );
      writeLogUnitTest();

      analyzeLogFile( logUnitTestNormal.getName(), parsedLinesLog );

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
