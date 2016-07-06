//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file StringHandler.C
/// @brief This file tests the String Handler.

#include "../src/StringHandler.h"

#include <gtest/gtest.h>

// utility library
#include "../src/FormattingException.h"

typedef formattingexception::GeneralException StringHandlerException;

//Test the parser
TEST( StringHandler, parser )
{
   std::string toBeParsed = "Hello;My;Dear, :)";
   std::string delemiter = ";";
   std::vector<std::string> tokens = {};

   // Nothing is happening the string is empty
   StringHandler::parseLine( "", delemiter, tokens );
   EXPECT_EQ( 0, tokens.size() );

   // This is an error
   try{
      StringHandler::parseLine( toBeParsed, "", tokens );
      FAIL() << "Expected 'Delimiter empty when parsing line' exception";
   }
   catch (StringHandlerException &ex){
      EXPECT_EQ( "Delimiter empty when parsing line", std::string( ex.what() ) );
      EXPECT_EQ( 0, tokens.size() );
   }
   catch (...) {
      FAIL() << "Expected 'Delimiter empty when parsing line' exception";
   }

   // Parse the line with ";"
   StringHandler::parseLine( toBeParsed, delemiter, tokens );
   EXPECT_EQ( 3, tokens.size() );
   if (tokens.size() == 3){
      EXPECT_EQ( "Hello", tokens[0] );
      EXPECT_EQ( "My", tokens[1] );
      EXPECT_EQ( "Dear, :)", tokens[2] );
   }
}