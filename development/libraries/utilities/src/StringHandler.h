//                                                                      
// Copyright (C) 2015-2017 Shell International Exploration & Production.
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

// utilities library
#include "FormattingException.h"

// std
#include <string>

/// @class StringHandler A utility for string opperations
class StringHandler
{

public:

   typedef formattingexception::GeneralException StringHandlerException;

   /// @brief Parse the given string according to the given token
   /// @param[in]  theString    The command to be parsed
   /// @param[in]  theDelimiter The command option delimiter
   /// @param[out] theTokens    The command's options parsed from the command line
   static void parseLine( const std::string& theString, const char& theDelimiter, std::vector<std::string>& theTokens );

private:

};
#endif
