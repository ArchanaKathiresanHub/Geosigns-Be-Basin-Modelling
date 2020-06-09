//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// \file LogHandlerStyles.cpp
/// \brief This file tests the LogHandler styles

#include "LogHandlerUnitTester.h"

namespace UnitTestLogStyle
{
   ///0. DEFINE TEST FUNCTIONS--------------------------------------------------------------------------
   /// \brief Writes some text into the log file with different styles
   void write() {
      LogHandler( LogHandler::INFO_SEVERITY, LogHandler::TITLE               ) << "TITLE I";
      LogHandler( LogHandler::INFO_SEVERITY, LogHandler::SECTION             ) << "SECTION 1";
      LogHandler( LogHandler::INFO_SEVERITY, LogHandler::SUBSECTION          ) << "SUBSECTION A";
      LogHandler( LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_STEP    ) << "COMPUTATION_STEP a";
      LogHandler( LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP ) << "COMPUTATION_SUBSTEP i";
      LogHandler( LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS ) << "COMPUTATION_DETAILS";
      LogHandler( LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_STEP    ) << "COMPUTATION_STEP b";
      LogHandler( LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP ) << "COMPUTATION_SUBSTEP i";
      LogHandler( LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS ) << "COMPUTATION_DETAILS";
      LogHandler( LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP ) << "COMPUTATION_SUBSTEP ii";
      LogHandler( LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS ) << "COMPUTATION_DETAILS";
      LogHandler( LogHandler::INFO_SEVERITY, LogHandler::SUBSECTION          ) << "SUBSECTION B";
      LogHandler( LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_STEP    ) << "COMPUTATION_STEP a";
      LogHandler( LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP ) << "COMPUTATION_SUBSTEP i";
      LogHandler( LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS ) << "COMPUTATION_DETAILS";
      LogHandler( LogHandler::INFO_SEVERITY, LogHandler::SECTION             ) << "SECTION 2";
      LogHandler( LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_STEP    ) << "COMPUTATION_STEP a";
      LogHandler( LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP ) << "COMPUTATION_SUBSTEP i";
      LogHandler( LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS ) << "COMPUTATION_DETAILS";
      LogHandler( LogHandler::INFO_SEVERITY, LogHandler::TITLE               ) << "TITLE II";
      LogHandler( LogHandler::INFO_SEVERITY, LogHandler::SECTION             ) << "SECTION 1";
      LogHandler( LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_STEP    ) << "COMPUTATION_STEP a";
      LogHandler( LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP ) << "COMPUTATION_SUBSTEP i";
      LogHandler( LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS ) << "COMPUTATION_DETAILS";
      LogHandler( LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_STEP    ) << "COMPUTATION_STEP b";
      LogHandler( LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP ) << "COMPUTATION_SUBSTEP i";
      LogHandler( LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS ) << "COMPUTATION_DETAILS";
   }

   ///1. INITIALISE TEST--------------------------------------------------------------------------------

//Initialize constants for comparaison
/// \brief Expected parsed lines from the log file and console
   const std::string expectedLog[] =
   {
      "________________________________________________________________________________",
      "____________________________________TITLE I_____________________________________",
      "//////////////////////////////"                                                  ,
      "/// SECTION 1"                                                                   ,
      "/// SUBSECTION A"                                                                ,
      "   -> COMPUTATION_STEP a"                                                        ,
      "      # COMPUTATION_SUBSTEP i"                                                   ,
      "        COMPUTATION_DETAILS"                                                     ,
      "   -> COMPUTATION_STEP b"                                                        ,
      "      # COMPUTATION_SUBSTEP i"                                                   ,
      "        COMPUTATION_DETAILS"                                                     ,
      "      # COMPUTATION_SUBSTEP ii"                                                  ,
      "        COMPUTATION_DETAILS"                                                     ,
      "/// SUBSECTION B"                                                                ,
      "   -> COMPUTATION_STEP a"                                                        ,
      "      # COMPUTATION_SUBSTEP i"                                                   ,
      "        COMPUTATION_DETAILS"                                                     ,
      "//////////////////////////////"                                                  ,
      "/// SECTION 2"                                                                   ,
      "   -> COMPUTATION_STEP a"                                                        ,
      "      # COMPUTATION_SUBSTEP i"                                                   ,
      "        COMPUTATION_DETAILS"                                                     ,
      "________________________________________________________________________________",
      "____________________________________TITLE II____________________________________",
      "//////////////////////////////"                                                  ,
      "/// SECTION 1"                                                                   ,
      "   -> COMPUTATION_STEP a"                                                        ,
      "      # COMPUTATION_SUBSTEP i"                                                   ,
      "        COMPUTATION_DETAILS"                                                     ,
      "   -> COMPUTATION_STEP b"                                                        ,
      "      # COMPUTATION_SUBSTEP i"                                                   ,
      "        COMPUTATION_DETAILS"
   };

   //Initialise variables
   std::vector<std::string> parsedLinesLog; ///< Parsed lines from the log file

}

///2. TEST-------------------------------------------------------------------------------------------
//Tests log styles for normal verbosity
TEST( LogHandlerStyles, all_styles )
{
   try{
      LogHandler logUnitTestStyle( "log_unit_test_styles", LogHandler::DETAILED_LEVEL );
      UnitTestLogStyle::write();

      analyzeLogFile( logUnitTestStyle.getName(), UnitTestLogStyle::parsedLinesLog );

      EXPECT_EQ( sizeof( UnitTestLogStyle::expectedLog)/sizeof(std::string), UnitTestLogStyle::parsedLinesLog.size() );
      for (unsigned int i = 0; i < UnitTestLogStyle::parsedLinesLog.size(); i++)
      {
         EXPECT_EQ( UnitTestLogStyle::expectedLog[i], UnitTestLogStyle::parsedLinesLog[i] );
      }
   }
   catch (const LogHandlerException& ex) {
      FAIL() << "Unexpected exception: " << ex.what();
   }
}
