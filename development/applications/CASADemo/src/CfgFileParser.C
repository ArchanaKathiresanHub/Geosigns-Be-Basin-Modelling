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

#include <cassert>
#include <cmath>


void CfgFileParser::parseFile( const std::string & cmdFile, CasaCommander & cmdQueue )
{
   std::ifstream file( cmdFile.c_str() );

   std::string line;
   // process command
   while( std::getline( file, line ) )
   {
      if ( line[0] == '#' || line.empty() ) continue;
      
      std::string              cmdID;
      std::vector<std::string> cmdPrms;

      std::istringstream iss( line );

      std::string result;
      // get command itself first
      int tokNum = 0;
      std::string opt;

      while( std::getline( iss, result, ' ') ) 
      {
         if ( result.empty() || (result.size() == 1 && result[0] == ' ') ) continue;

         if ( 0 == tokNum )// get app name
         {
            cmdID = result;
         }
         else
         {
            if ( opt.size() ) // continue parsing one option
            {
               if ( *result.rbegin() == '"' )
               {                  
                  opt += " " + result.substr( 0, result.size()-1 ); 
                  cmdPrms.push_back( opt );
                  opt = "";
               }
               else { opt += " " + result; }
            
            }
            else // new option
            {
               if ( result[0] == '"' )
               {
                  if ( *result.rbegin() == '"'  )
                  {
                     cmdPrms.push_back( result.substr( 1, result.size() - 2 ) );
                  }
                  else
                  {
                     opt = result.substr( 1 );
                  }
               }
               else
               {
                  cmdPrms.push_back( result );
               }
            }  
         }
         ++tokNum;
      }
      cmdQueue.addCommand( cmdID, cmdPrms );
   }
}

// split list of strings divided by sep in to array of strings
std::vector<std::string> CfgFileParser::list2array( const std::string & listOfStr, char sep )
{
   std::vector<std::string> strList; // array where we will keep strings from list to return

   std::istringstream iss( listOfStr ); // tokenizer

   std::string result; // one token

   while( std::getline( iss, result, sep ) ) 
   {
      if ( result.empty() || (result.size() == 1 && result[0] == sep ) ) continue; // skip spaces and separators

      strList.push_back( result ); // add token to the list
   }

   return strList;
}

// read well trajectory file with reference values
void CfgFileParser::readTrajectoryFile( const std::string & fileName, 
                                        std::vector<double> & x,
                                        std::vector<double> & y,
                                        std::vector<double> & z,
                                        std::vector<double> & ref )
{
   std::ifstream file( fileName.c_str() );

   x.clear();
   y.clear();
   z.clear();
   ref.clear();

   std::string line;
   
   // process one line 
   while( std::getline( file, line ) )
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

