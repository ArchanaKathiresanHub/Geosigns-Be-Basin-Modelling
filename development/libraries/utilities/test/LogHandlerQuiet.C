//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file LogHandlerQuiet.C
/// @brief This file tests the LogHandler for the Quiet verbosity

#include "LogHandlerUnitTester.C"

///1. INITIALISE TEST--------------------------------------------------------------------------------

//Initialize constants for comparaison
std::vector<std::string> expectedLog =                ///< Expected parsed lines from the log file and console
{                                                      
   "MeSsAgE FATAL    This is a fatal error."
};
                                                       
//Initialise variables
std::vector<std::string> parsedLinesLog = {};         ///< Parsed lines from the log file


///3. TEST-------------------------------------------------------------------------------------------
//Tests log for quiet verbosity
TEST( LogHandlerSerial, log_quiet )
{
   try{
      LogHandler logUnitTestQuiet( "log_unit_test_quiet", LogHandler::QUIET );
      writeLogUnitTest( logUnitTestQuiet );

      analyzeLogFile( logUnitTestQuiet.getName(), parsedLinesLog );

      EXPECT_EQ( expectedLog.size(), parsedLinesLog.size() );
      for (unsigned int i = 0; i < parsedLinesLog.size(); i++)
      {
         EXPECT_EQ( expectedLog[i], parsedLinesLog[i] );
      }
   }
   catch (const formattingexception::GeneralException& ex) {
      std::cerr << ex.what();
      EXPECT_EQ( "NoError", "Error" );
   }
}