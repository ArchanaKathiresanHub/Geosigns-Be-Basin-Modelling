//                                                                      
// Copyright (C) 2015-2017 Shell International Exploration & Production.
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
   const std::string toBeParsed = "Hello;My;Dear, :)";
   const char delemiter = ';';
   std::vector<std::string> tokens = {};

   // Nothing is happening the string is empty
   StringHandler::parseLine( "", delemiter, tokens );
   EXPECT_EQ( 0, tokens.size() );

   // Parse the line with ";"
   StringHandler::parseLine( toBeParsed, delemiter, tokens );
   EXPECT_EQ( 3, tokens.size() );
   if (tokens.size() == 3){
      EXPECT_EQ( "Hello", tokens[0] );
      EXPECT_EQ( "My", tokens[1] );
      EXPECT_EQ( "Dear, :)", tokens[2] );
   }
}