//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/// @file LogHandlerUnitTester.C
/// @brief This file is a testing utility for all LogHandler unit tests

#ifndef UTILITIES_LOGHANDLERUNITTESTER_H
#define UTILITIES_LOGHANDLERUNITTESTER_H

#include "../src/LogHandler.h"

// std library
#include <fstream>
#include <iostream>
#include <string>

// google tests library
#include <gtest/gtest.h>

// utility library
#include "../src/FormattingException.h"

/// @brief Write test messages in the boost log object
void writeLogUnitTest () {
   LogHandler( LogHandler::FATAL)   << "This is a fatal error: " << 0 ;
   LogHandler( LogHandler::ERROR)   << "This is an error: "      << 1 ;
   LogHandler( LogHandler::WARNING) << "This is a warning: "     << 2 ;
   LogHandler( LogHandler::INFO)    << "This is an info: "       << 3 ;
   LogHandler( LogHandler::DEBUG)   << "This is a debug info: "  << 4 ;
}

/// @ brief Count the number of lines in one log file, extract the lines after parsing
/// @details During parsing the first part of the line containing the timestamp, MPI rank, etc. is supressed
///   <Year-Month-Day Hour:Minute:Second.Fraction | MPIRank>:" like in "<2005-10-15 13:12:11.000000 | 4>: is supressed from each line
/// @param logName The full name of the log file
/// @param numberOfLines The number of lines in the log file
/// @param parsedLines The parsed lines from the log file
void analyzeLogFile( const std::string& logName, std::vector<std::string>& parsedLines  ) {
   parsedLines.clear();
   // C++11 std::ifstream myfile( logName );
   std::ifstream myfile( logName.c_str() );
   std::string line;
   const std::string delimiter = ">:";
   std::string parsedLine;
  
   if (myfile.is_open()){
      // For each line
      while (!myfile.eof()){
         getline( myfile, line );
         size_t pos = 0;
         pos = line.find( delimiter );
         // If we find the delimiter
         if (pos != std::string::npos){
            // Sparse the line
            parsedLine = line.substr( pos + delimiter.length() + 1, std::string::npos );
            parsedLines.push_back( parsedLine );
         }
      }
      myfile.close();
   }
}
#endif
