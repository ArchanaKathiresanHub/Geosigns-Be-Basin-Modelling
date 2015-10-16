///
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
// This utility allow to load and then compare table by table 2 .project3d files

#include "cmbAPI.h"
#include "Path.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include <cstring>

#include <set>
#include <map>

// Utilities lib
#include <NumericFunctions.h>
#include <WallTime.h>


#define ERROR      -1
#define WARNING     1
#define INFOMSG     2
#define PROGRESSMSG 3

static int g_VerboseLevel = 3;
static int g_totalDiffNumber = 0;

template <typename T> std::string to_string( const T & prm ) { std::ostringstream oss; oss << prm; return oss.str(); }

void Message( const std::string & msg, int level, bool newLine = true )
{
   if ( level <= g_VerboseLevel ) 
   {
      if ( level == ERROR ) { std::cerr << msg << (newLine ? "\n" : ""); }
      else                  { std::cout << msg << (newLine ? "\n" : ""); std::cout.flush(); }
   }
}

int Usage( const char * pName )
{
   std::cout << "Utility for compare or merge two Cauldron project3d files" << std::endl;
   std::cout << "Usage: " << std::endl;
   std::cout << "To compare:\n   " << pName << " [-ignore <com sep. tables list>]  [-table <com.sep.tables list>] [-tol eps] [-project1]";
   std::cout <<                                " in1.project3d [-project2] in2.project3d" << std::endl;
   std::cout << "To merge:\n     " << pName << " -merge -table <com. sep. tables list> [-filter \"Table1:ColName1:Value1,...\"] -[project1]";
   std::cout <<                                " source.projcet3d [-project2] dest.project3d" << std::endl;
   return 0;
}


// split list of strings divided by sep in to array of strings
static std::vector<std::string> list2array( const std::string & listOfStr, char sep )
{
   std::vector<std::string> strList;          // array where we will keep strings from list to return
   std::istringstream       iss( listOfStr ); // tokenizer
   std::string              result;           // one token

   while( std::getline( iss, result, sep ) ) 
   {
      if ( result.empty() || (result.size() == 1 && result[0] == sep ) ) continue; // skip spaces and separators
      strList.push_back( result ); // add token to the list
   }
   return strList;
}


///////////////////////////////////////////////////////////////////////////////
//
// Main function
//
///////////////////////////////////////////////////////////////////////////////
int main( int argc, char ** argv )
{
   double eps = 1.e-5;

   bool compareFiles = true;
   bool mergeFiles = false;

   const std::string mergeFilter; // filter like "Table1:Column1:Value1,Table2:Column2:Value2"

   std::set<std::string> ignoreList; // list of tables which will be skipped
   std::set<std::string> procesList; // list of tables which will be processed
   std::vector< std::vector<std::string> > filterList; // list of filters which should be applied during tables merge

   if ( argc < 3 ) return Usage( argv[0] );

   const char * in1File = NULL;
   const char * in2File = NULL;

   // parser input args
   for ( int i = 1; i < argc; ++i )
   {
      if ( argv[i][0] == '-') // parser utility flags
      {
         if (      !strcmp( argv[i], "-q"        ) ) { g_VerboseLevel = 0; }
         else if ( !strcmp( argv[i], "-v"        ) && argc > i+1 && argv[i+1][0] != '-' ) { g_VerboseLevel = atol( argv[++i] ); }
         else if ( !strcmp( argv[i], "-tol"      ) && argc > i+1 && argv[i+1][0] != '-' ) { eps            = atof( argv[++i] ); }
         else if ( !strcmp( argv[i], "-project1" ) && argc > i+1 && argv[i+1][0] != '-' ) { in1File        =       argv[++i];   }
         else if ( !strcmp( argv[i], "-project2" ) && argc > i+1 && argv[i+1][0] != '-' ) { in2File        =       argv[++i];   }
         else if ( !strcmp( argv[i], "-merge"    )                                      ) { compareFiles = false; mergeFiles = true; }
         else if ( !strcmp( argv[i], "-filter"   ) && argc > i+1 && argv[i+1][0] != '-' ) 
         {
            const std::vector<std::string> & flist = list2array( argv[++i], ',' );
            if ( !flist.empty() ) 
            {
               for ( size_t j = 0; j < flist.size(); ++j )
               {
                  const std::vector<std::string> & flt = list2array( flist[j].c_str(), ':' );
                  if ( !flt.empty() )
                  {
                     filterList.push_back( std::vector<std::string>( flt.begin(), flt.end() ) );
                     if ( filterList.back().size() != 3 )
                     {
                        Message( std::string( "Wrong filter is given, must be in format Table:ColName:Value, but given: " ) + flist[j], ERROR );
                     }
                  }
               }
            }
         }
         else if ( !strcmp( argv[i], "-ignore"   ) && argc > i+1 && argv[i+1][0] != '-' )
         {
            const std::vector<std::string> & tlist = list2array( argv[++i], ',' );
            if ( !tlist.empty() ) ignoreList.insert( tlist.begin(), tlist.end() );
         }
         else if ( !strcmp( argv[i], "-table"    ) && argc > i+1 && argv[i+1][0] != '-' )
         {
            const std::vector<std::string> & tlist = list2array( argv[++i], ',' );
            if ( !tlist.empty() ) procesList.insert( tlist.begin(), tlist.end() );
         }
         else
         {
            Message( std::string( "Unknown parameter: " ) + argv[i], ERROR );
            return Usage( argv[0] );
         }
      }
      else
      {
         if (      !in1File ) { in1File = argv[i]; }
         else if ( !in2File ) { in2File = argv[i]; }
         else
         {
            Message( std::string( "Unknown parameter: " ) + argv[i], ERROR );
            return Usage( argv[0] );
         }
      }
   }
   if ( !in1File || !in2File ) 
   {
      Message( std::string( "Wrong parameters number, project file name is missing" ), ERROR );
      return Usage( argv[0] );
   }

   WallTime::Time timer = WallTime::clock();

   // load project
   mbapi::Model cldProject1;
   mbapi::Model cldProject2;

   Message( std::string( "# Reading project file: " ) + in1File + "...", PROGRESSMSG, false );
   if ( ErrorHandler::NoError != cldProject1.loadModelFromProjectFile( in1File ) )
   {
      Message( std::string( "Failing to load project file: " ) + in1File, ERROR );
      return -1;
   }
   Message( std::string( " done in " ) + to_string( (WallTime::clock() - timer).floatValue() ) + " sec.", PROGRESSMSG );
   timer = WallTime::clock();

   Message( std::string( "## Reading project file: " ) + in2File + "...", PROGRESSMSG, false );
   if ( ErrorHandler::NoError != cldProject2.loadModelFromProjectFile( in2File ) )
   {
      Message( std::string( "Failing to load project file: " ) + in2File, ERROR );
      return -1;
   }
   Message( std::string( " done in " ) + to_string( (WallTime::clock() - timer).floatValue() ) + " sec.", PROGRESSMSG );
   timer = WallTime::clock();

   if ( compareFiles )
   {
      Message( std::string( "### Comparing tables..." ), PROGRESSMSG, false );
      const std::string & diffs = cldProject1.compareProject( cldProject2, procesList, ignoreList, eps );

      g_totalDiffNumber += std::count( diffs.begin(), diffs.end(), '\n' );
   
      Message( std::string( " done in " ) + to_string( (WallTime::clock() - timer).floatValue() ) + " sec.", PROGRESSMSG );
      if ( !diffs.empty() ) { Message( std::string( "Found differences: \n" ) + diffs, WARNING ); }

      timer = WallTime::clock();
   
      Message( "###### All done!", PROGRESSMSG );
   
      std::ostringstream oss;
      oss << "Found " <<  g_totalDiffNumber << " differences";
      Message( oss.str(), g_totalDiffNumber > 0 ? 0 : 1 );

      return g_totalDiffNumber > 0 ? -1 : 0;
   }

   if ( mergeFiles )
   {
      Message( std::string( "### Merging tables..." ), PROGRESSMSG, false );
      
      size_t dlRecNum;
      size_t cpRecNum;

      const std::string ret = cldProject2.mergeProject( cldProject1, procesList, filterList, dlRecNum, cpRecNum );
      
      if ( !ret.empty() ) Message( ret, ERROR );

      Message( std::string( "#### Deleted records number: \n" ) + ibs::to_string( dlRecNum ), WARNING ); 
      Message( std::string( "#### Copied records number: \n"  ) + ibs::to_string( cpRecNum ), WARNING );

      Message( std::string( " done in " ) + to_string( (WallTime::clock() - timer).floatValue() ) + " sec.", PROGRESSMSG );
      Message( "###### All done!", PROGRESSMSG );

      return ret.empty() ? 0 : -1;
   }

   return 0;
}


