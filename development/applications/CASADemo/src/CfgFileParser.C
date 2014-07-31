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

#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

#include <cassert>

void CfgFileParser::parseFile( const std::string & fileName )
{
   std::ifstream file( fileName.c_str() );

   std::string line;
   // process command
   while( std::getline( file, line ) )
   {
      if ( line[0] == '#' ) continue;

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
            if (      result == "app"          ) m_cmdList.push_back( app );
            else if ( result == "base_project" ) m_cmdList.push_back( base_project );
            else if ( result == "varprm"       ) m_cmdList.push_back( varprm ); 
            else if ( result == "target"       ) m_cmdList.push_back( target );
            else if ( result == "doe"          ) m_cmdList.push_back( doe );
            else if ( result == "run"          ) m_cmdList.push_back( run );
            else if ( result == "location"     ) m_cmdList.push_back( location );
            else throw std::runtime_error( std::string( "Unknown command: " ) + result );
            m_cmdPrms.push_back( std::vector<std::string>() );
         }
         else
         {
            assert( m_cmdList.size() == m_cmdPrms.size() );

            if ( opt.size() ) // continue parsing one option
            {
               if ( result[result.size()-1] == '"' )
               {                  
                  opt += " " + result.substr( 0, result.size()-1 ); 
                  m_cmdPrms.back().push_back( opt );
                  opt = "";
               }
               else { opt += " " + result; }
            
            }
            else // new option
            {
               if ( result[0] == '"' )
               {
                  if ( result[result.size()-1] == '"'  )
                  {
                     m_cmdPrms.back().push_back( result.substr( 1, result.size() - 2 ) );
                  }
                  else
                  {
                     opt = result.substr( 1 );
                  }
               }
               else
               {
                  m_cmdPrms.back().push_back( result );
               }
            }  
         }
         ++tokNum;
      }
   }
   assert( m_cmdList.size() == m_cmdPrms.size() ); 
}

std::ostream & operator << ( std::ostream & ost, const CfgFileParser & cfg )
{
   for ( size_t i = 0; i < cfg.m_cmdList.size(); ++i )
   {
      switch( cfg.m_cmdList[i] )
      {
         case CfgFileParser::app:            ost << "app         "; break;
         case CfgFileParser::base_project:   ost << "base_project"; break;
         case CfgFileParser::varprm:         ost << "varprm      "; break;
         case CfgFileParser::target:         ost << "target      "; break;
         case CfgFileParser::doe:            ost << "doe         "; break;
         case CfgFileParser::run:            ost << "run         "; break;
         case CfgFileParser::location:       ost << "location    "; break;
         default: break;
      }
      ost << ":";
      for ( size_t j = 0; j < cfg.m_cmdPrms[i].size(); ++j )
      {
         ost << " \"" << cfg.m_cmdPrms[i][j] << "\""; 
      }
      ost << std::endl;
   }
   return ost;
}

