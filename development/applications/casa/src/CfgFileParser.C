//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

#include "CfgFileParser.h"
#include "CasaCommander.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <locale>

#include <cassert>
#include <cmath>

static std::string trim( const std::string & s )
{
   std::string::const_iterator it = s.begin();
   while ( it != s.end() && isspace(*it) ) it++;

   std::string::const_reverse_iterator rit = s.rbegin();
   while ( rit.base() != it && isspace(*rit) ) rit++;

   // remove "" around string if so
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
      std::vector<std::string> cmdPrms;

      std::istringstream iss( cmdLine );

      std::string result;
      // get command itself first
      int tokNum = 0;

      while( std::getline( iss, result, ' ') ) 
      {
         if ( result.empty() || (result.size() == 1 && result[0] == ' ') ) continue; // ignore spaces
         if ( result[0] == '#' ) break; // ignore all after comment

         if ( 0 == tokNum ) // get cmd name
         {
            cmdID = result;
         }
         else
         {
            if ( result[0] == '"' ) // need to get the full string without any change inside
            {
               std::string opt = result.substr( 1, result.size() - 1 );
               if ( *opt.rbegin() == '"' )
               {
                  opt.erase( opt.size() - 1, 1 );
               }
               else if ( std::getline( iss, result, '"' ) )
               {
                  opt += " " + result;
               }
               else 
               {
                  std::ostringstream oss;
                  oss << "Can not find closing \" for the string: " << opt << ", at line: " << lineNum << ", command: " << cmdID;
                  throw::std::runtime_error( oss.str().c_str() ); 
               }

               cmdPrms.push_back( opt );
               continue;
            }
            else if ( result[0] == '[' ) // get whole array in one go
            {
               std::string opt = result;
               if ( *opt.rbegin() != ']' )
               {
                  if ( std::getline( iss, result, ']' ) )
                  {
                     opt += " " + result + "]";
                  }
                  else 
                  {
                     std::ostringstream oss;
                     oss << "Can not find closing ] for the set: " << opt << ", at line: " << lineNum << ", command: " << cmdID;
                     throw::std::runtime_error( oss.str().c_str() ); 
                  }
               }
               cmdPrms.push_back( opt );
               continue;
            }
            else 
            {
               cmdPrms.
                  push_back( result );
            }
         }
         ++tokNum;
      }
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
                                        std::vector<double> & ref )
{
   std::ifstream ifs( fileName.c_str() );

   if ( !ifs.is_open() ) { throw::std::runtime_error( std::string("Can not open trajectory file: " ) + fileName ); }

   x.clear();
   y.clear();
   z.clear();
   ref.clear();

   std::string line;
   
   // process one line 
   while( std::getline( ifs, line ) )
   {
      if ( line[0] == '#' ) continue;

      std::istringstream iss( line );

      std::string result;
      
      int tokNum = 0;
      std::string opt;

      double xc, yc, zc, rv;

      while( std::getline( iss, result, ' ') ) 
      {
         if ( result.empty() || (result.size() == 1 && result[0] == ' ') ) continue;

         switch( tokNum )
         {
            case 0: xc = atof( result.c_str() ); break;
            case 1: yc = atof( result.c_str() ); break;
            case 2: zc = atof( result.c_str() ); break;
            case 3: rv = atof( result.c_str() ); break;
            default: throw std::runtime_error( std::string( "Wrong format of Well trajectory file: " ) + fileName );
         }
         ++tokNum;
      }
      if ( 4 == tokNum )
      {
         x.push_back( xc );
         y.push_back( yc );
         z.push_back( zc );
         ref.push_back( rv );
      }
   }

   ifs.close();
}

// read parameters value from plain data file
void CfgFileParser::readParametersValueFile( const std::string & fileName, std::vector< std::vector<double> > & dataVals )
{
   std::ifstream file( fileName.c_str() );

   dataVals.clear();

   std::string line;
   
   // process one line 
   while( std::getline( file, line ) )
   {
      if ( line[0] == '#' ) continue;

      std::istringstream iss( line );

      std::string result;
      
      int tokNum = 0;
      std::string opt;

      std::vector<double> oneLineVals;

      while( std::getline( iss, result, ' ') ) 
      {
         if ( result.empty() || (result.size() == 1 && result[0] == ' ') ) continue;

         double v = atof( result.c_str() );
         oneLineVals.push_back( v );
      }
      if ( !oneLineVals.empty() ) dataVals.push_back( oneLineVals );
   }
}

bool CfgFileParser::isNumericPrm( const std::string prm )
{
   std::locale loc;
   return std::isdigit( prm[0], loc );
}

