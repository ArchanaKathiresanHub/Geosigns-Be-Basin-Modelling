//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// \file LogHandlerTime.cpp
/// \brief This file tests the LogHandler time logging functionality

#include "LogHandlerUnitTester.h"

namespace UnitTestLogTime
{
   ///1. INITIALISE TEST--------------------------------------------------------------------------------

   //Initialize constants for comparaison
   /// \brief Expected parsed lines from the log file and console
   const std::string expectedLog[] =
   {
      "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~",
      "Simulation time: 2 hours 10 minutes 36 seconds"
   };

   //Initialise variables
   std::vector<std::string> parsedLinesLog; ///< Parsed lines from the log file
}

///2. TEST-------------------------------------------------------------------------------------------
//Tests time logging utility
TEST( LogHandlerTime, time )
{
   try{
      LogHandler logUnitTestTime( "log_unit_test_time", LogHandler::DETAILED_LEVEL );
      LogHandler::displayTime( LogHandler::INFO_SEVERITY, 7836, "Simulation time" );
      analyzeLogFile( logUnitTestTime.getName(), UnitTestLogTime::parsedLinesLog );

      EXPECT_EQ( sizeof( UnitTestLogTime::expectedLog)/sizeof(std::string), UnitTestLogTime::parsedLinesLog.size() );
      for (unsigned int i = 0; i < UnitTestLogTime::parsedLinesLog.size(); i++)
      {
         EXPECT_EQ( UnitTestLogTime::expectedLog[i], UnitTestLogTime::parsedLinesLog[i] );
      }
   }
   catch (const LogHandlerException& ex) {
      FAIL() << "Unexpected exception: " << ex.what();
   }
}
