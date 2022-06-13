//                                                                      
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

#include "CfgFileParser.h"
#include "CasaCommander.h"

#include "ErrorHandler.h"
#include "casaCmdInterface.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <locale>

#include <cassert>
#include <cmath>

// this function delete spaces from the beginning and at the end of the givng string:
// " asdf   " -> "asdf"
static std::string trim( const std::string & s )
{
   std::string::const_iterator it = s.begin();
   while ( it != s.end() && isspace(*it) ) it++;

   std::string::const_reverse_iterator rit = s.rbegin();
   while ( rit.base() != it && isspace(*rit) ) rit++;

   // remove "" around the giving string if so
   if ( *it  == '"' ) { it++; }
   if ( *rit == '"' ) { rit++; }

   return std::string( it, rit.base() );
}

void CfgFileParser::parseFile( const std::string & cmdFile, CasaCommander & cmdQueue )
{
   std::ifstream file( cmdFile.c_str() );

   std::string line;
   std::string cmdLine;

   size_t lineNum = 0;
   // process command
   while( std::getline( file, line ) )
   {
      ++lineNum;

      if ( line[0] == '#' || line.empty() ) continue;

      // check if line is splitted by \ character. If yes - just read next one and concatenate
      std::string::size_type splitFound = line.rfind( '\\' );
      if ( splitFound != std::string::npos ) 
      {
         cmdLine += line.substr( 0, splitFound );
         continue;
      }
      else { cmdLine += line; }

      std::string              cmdID;
      std::vector<std::string> cmdPrms = casaCmdInterface::stringVecFromCasaCommand(cmdLine,cmdID,int(lineNum));

      cmdQueue.addCommand( cmdID, cmdPrms, lineNum );
      cmdLine = "";
   }
}

// split list of strings divided by sep in to array of strings
std::vector<std::string> CfgFileParser::list2array( const std::string & lst, char sep )
{
   std::vector<std::string> strList; // array where we will keep strings from list to return

   std::string listOfStr = lst;

   // cut []
   if ( listOfStr[0]                    == '[' ) listOfStr.erase( 0, 1 );
   if ( listOfStr[listOfStr.size() - 1] == ']' ) listOfStr.erase( listOfStr.size() - 1, 1 );

   std::istringstream iss( listOfStr ); // tokenizer

   std::string result; // one token

   while( std::getline( iss, result, sep ) ) 
   {
      if ( result.empty() || (result.size() == 1 && result[0] == sep ) ) continue; // skip spaces and separators

      strList.push_back( trim( result ) ); // add token to the list
   }

   return strList;
}

// parse matlab like vector: [1,3,4,5] to vector of doubles
std::vector<double> CfgFileParser::set2array( const std::string & lst, char sep )
{
   std::vector<double> valSet; // array where we will keep double values from the set to return
   std::string listOfVal = lst;

   // cut []
   if ( listOfVal[0]                    == '[' ) listOfVal.erase( 0, 1 );
   if ( listOfVal[listOfVal.size() - 1] == ']' ) listOfVal.erase( listOfVal.size() - 1, 1 );

   std::istringstream iss( listOfVal ); // tokenizer

   std::string result; // one token

   while (std::getline(iss, result, sep))
   {
      if (result.empty() || (result.size() == 1 && result[0] == sep)) continue; // skip spaces and separators

      valSet.push_back( atof(result.c_str() ) ); // add token to the list
   }

   return valSet;
}

// read well trajectory file with reference values
void CfgFileParser::readTrajectoryFile( const std::string & fileName, 
                                        std::vector<double> & x,
                                        std::vector<double> & y,
                                        std::vector<double> & z,
                                        std::vector<double> & ref,
                                        std::vector<double> & sdev )
{
   std::ifstream ifs( fileName.c_str() );

   if ( !ifs.is_open() ) { throw ErrorHandler::Exception( ErrorHandler::IoError ) << "Can not open trajectory file: " << fileName; }

   x.clear();
   y.clear();
   z.clear();
   ref.clear();
   sdev.clear();

   std::string line;
   
   // process one line 
   while( std::getline( ifs, line ) )
   {
      if ( line[0] == '#' ) continue;

      std::istringstream iss( line );

      std::string result;
      
      int tokNum = 0;
      std::string opt;

      double xc, yc, zc, rv, sd;

      while( std::getline( iss, result, ' ') ) 
      {
         if ( result.empty() || (result.size() == 1 && result[0] == ' ') ) continue;

         switch( tokNum )
         {
            case 0: xc = atof( result.c_str() ); x.push_back(    xc ); break;
            case 1: yc = atof( result.c_str() ); y.push_back(    yc ); break;
            case 2: zc = atof( result.c_str() ); z.push_back(    zc ); break;
            case 3: rv = atof( result.c_str() ); ref.push_back(  rv ); break;
            case 4: sd = atof( result.c_str() ); sdev.push_back( sd ); break;
            default: throw ErrorHandler::Exception( ErrorHandler::IoError ) << "Wrong format of Well trajectory file: " << fileName;
         }
         ++tokNum;
      }
   }

   ifs.close();
}

bool CfgFileParser::isNumericPrm( const std::string & prm )
{
   std::locale loc;
   return std::isdigit( prm[0], loc );
}
