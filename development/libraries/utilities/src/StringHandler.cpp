//                                                                      
// Copyright (C) 2015-2017 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

#include "StringHandler.h"
#include <regex>

void StringHandler::parseLine( const std::string& theString, const char& theDelimiter, std::vector<std::string>& theTokens )
{

   std::string::size_type startPos  = 0;
   std::string::size_type endPos    = 0;
   std::string::size_type increment = 0;
   std::string token;

   while (endPos != std::string::npos)
   {
      endPos = theString.find_first_of( theDelimiter, startPos );
      increment = endPos - startPos;

      token = theString.substr( startPos, increment );
      if (token.size() != 0)
      {
         theTokens.push_back( token );
      }
      startPos += increment + 1;
   }
}

std::string StringHandler::implode( const std::vector<std::string> & vos, const char * delim, size_t st )
{
   std::ostringstream ret;
   for ( size_t i = st; i < vos.size(); ++i )
   {
      ret << (i == st ? "" : delim ) << vos[i];
   }
   return ret.str();
}

