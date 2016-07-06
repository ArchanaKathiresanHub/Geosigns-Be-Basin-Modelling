//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file StringHandler.h
/// @brief This file has string utilities for all IBS applications and libraries

#ifndef UTILITIES_STRINGHANDLER_H
#define UTILITIES_STRINGHANDLER_H

#include "FormattingException.h"

/// @class StringHandler A utility for string opperations
class StringHandler
{

public:

   typedef formattingexception::GeneralException StringHandlerException;

   /// @brief Parse the given string according to the given token
   /// @param theString The command to be parsed
   /// @param theDelimiter The command option delimiter
   /// @return theTokens The command's options parsed from the command line
   static void parseLine( const std::string& theString, const std::string& theDelimiter, std::vector<std::string>& theTokens );

private:

};
#endif
