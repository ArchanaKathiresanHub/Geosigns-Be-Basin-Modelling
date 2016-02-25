//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file LogHandlerCreate.C
/// @brief This file tests the that the Log Handler creates a log file and that it can be only created once.

#include "LogHandlerUnitTester.h"

//Test if the log is created
TEST( LogHandlerSerial, log_created )
{
   try{
      LogHandler logUnitTestDiagnostic( "log_unit_test_created", LogHandler::DIAGNOSTIC );
      writeLogUnitTest();

      bool fileExisits = false;
      // C++11 std::ifstream myfile( logUnitTestDiagnostic.getName() );
      std::ifstream myfile( logUnitTestDiagnostic.getName().c_str() );
      if (myfile.is_open()){
         fileExisits = true;
      }

      EXPECT_EQ( fileExisits, true );
   }
   catch (const LogHandlerException& ex) {
      FAIL() << "Unexpected exception: " << ex.what();
   }
}

//Test that the log can be created only once
TEST( LogHandlerSerial, log_created_again )
{
   // Test if the exception is thrown
   EXPECT_THROW( LogHandler( "log_unit_test_crash", LogHandler::DIAGNOSTIC ), LogHandlerException );

   // Test if the good exception is thrown
   try{
      LogHandler( "log_unit_test_crash", LogHandler::DIAGNOSTIC );
      FAIL() << "Expected 'Log file 'log_unit_test_created_0.log' already created, cannot create new log file 'log_unit_test_crash_0.log'.' exception";
   }
   catch (const LogHandlerException& ex) {
      EXPECT_EQ( "Log file 'log_unit_test_created_0.log' already created, cannot create new log file 'log_unit_test_crash_0.log'.", std::string(ex.what()) );
   }
   catch (...) {
      FAIL() << "Expected 'Log file log_unit_test_crash.log already created.' exception";
   }
}
