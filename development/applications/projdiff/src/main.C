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
   std::cout << "Utility for compare two Cauldron project3d files" << std::endl;
   std::cout << "Usage: " << std::endl;
   std::cout << "   " << pName << " [-ignore <com sep. tables list>]  [-table <com.sep.tables list>] [-tol eps] [-project1] in1.project3d [-project2] in2.project3d" << std::endl;
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

std::vector<std::string> compareTablesNames( mbapi::Model & cldProject1
                                           , mbapi::Model & cldProject2
                                           , std::set<std::string> & ignoreList
                                           , std::set<std::string> & procesList
                                           )
{
   std::vector<std::string> ret;

   const std::vector<std::string> & lst1 = cldProject1.tablesList();
   const std::vector<std::string> & lst2 = cldProject2.tablesList();

   std::map<std::string, int> commonSet;

   // put first project tables list into map
   for ( size_t i = 0; i < lst1.size(); ++i )
   {
      // check for ignore list
      if ( ignoreList.size() > 0 && ignoreList.count( lst1[i] )  > 0 ||
           procesList.size() > 0 && procesList.count( lst1[i] ) == 0 
         ) { continue; }

      commonSet[ lst1[i] ] = cldProject1.tableSize( lst1[i] );  
   }

   // go over the second project table list and check for the same tables
   for ( size_t i = 0; i < lst2.size(); ++i )
   {
      // check for ignore list
      if ( ignoreList.size() > 0 && ignoreList.count( lst2[i] )  > 0 ||
           procesList.size() > 0 && procesList.count( lst2[i] ) == 0 
         ) { continue; }

      if ( commonSet.count( lst2[i] ) > 0 )
      {  
         int tbl2Size = cldProject2.tableSize( lst2[i] );
         int tbl1Size = commonSet[ lst2[i] ];
         
         // both exists
         if ( tbl2Size > 0 && tbl1Size > 0 ) { ret.push_back( lst2[i] ); }
         else if ( tbl1Size > 0 )
         {
            Message( std::string( "Only in " ) + cldProject1.projectFileName() + " table " + lst2[i], WARNING );
            ++g_totalDiffNumber;
         }
         else if ( tbl2Size > 0 )
         {
            Message( std::string( "Only in " ) + cldProject2.projectFileName() + " table " + lst2[i], WARNING );
            ++g_totalDiffNumber;
         }
         commonSet.erase( lst2[i] ); // exclude table from the set
      }
      else if ( cldProject2.tableSize( lst2[i] ) > 0 )
      {
         Message( std::string( "Only in " ) + cldProject2.projectFileName() + " table " + lst2[i], WARNING );
         ++g_totalDiffNumber;
      }
   }

   // go again over list of tables in commonSet - all of the not exist in the second project, report them 
   for ( std::map<std::string,int>::iterator it = commonSet.begin(); it != commonSet.end(); ++it )
   {
      if ( it->second > 0 ) // skip zero sized tables
      {
         Message( std::string( "Only in " ) + cldProject1.projectFileName() + " table " + it->first, WARNING );
         ++g_totalDiffNumber;
      }
   }
   return ret;
}

void compareTable( mbapi::Model & cldProject1, mbapi::Model & cldProject2, const std::string & tblName, double tol )
{
   int tblSize1 = cldProject1.tableSize( tblName );
   int tblSize2 = cldProject2.tableSize( tblName );

   // tables with different sizes couldn't be compared
   if ( tblSize1 != tblSize2 )
   {
      ostringstream oss;
      oss << "Table " << tblName << " has " << tblSize1 << " records in project " << cldProject1.projectFileName() <<
                                    " but " << tblSize2 << " records in project " << cldProject2.projectFileName();
      Message( oss.str(), WARNING );
      ++g_totalDiffNumber;
      return;
   }

   std::vector<mbapi::Model::ProjectTableColumnDataType> colTypes;
   const std::vector<std::string> & colNames = cldProject1.tableColumnsList( tblName, colTypes );

   for ( size_t i = 0; i < tblSize1; ++i )
   {
      for ( size_t j = 0; j < colNames.size(); ++j )
      {
         ostringstream oss;

         switch ( colTypes[j] )
         {
         case mbapi::Model::Bool:
         case mbapi::Model::Int:
         case mbapi::Model::Long:
            {
               long v1 = cldProject1.tableValueAsInteger( tblName, i, colNames[j] );
               long v2 = cldProject2.tableValueAsInteger( tblName, i, colNames[j] );
               if ( v1 != v2 ) { oss << tblName << "("<< i+1 << ")." << colNames[j] << ": " << v1 << " != " << v2; }
            }
            break;

         case mbapi::Model::Float:
         case mbapi::Model::Double:
            {
               double v1 = cldProject1.tableValueAsDouble( tblName, i, colNames[j] );
               double v2 = cldProject2.tableValueAsDouble( tblName, i, colNames[j] );
               if ( !NumericFunctions::isEqual( v1,  v2, tol ) )
               {
                  oss << tblName << "("<< i+1 << ")." << colNames[j] << ": " << v1 << " != " << v2;
               }
            }
            break;

         case mbapi::Model::String:
            {
               const std::string & v1 = cldProject1.tableValueAsString( tblName, i, colNames[j] );
               const std::string & v2 = cldProject2.tableValueAsString( tblName, i, colNames[j] );
               if ( v1 != v2 ) { oss << tblName << "("<< i+1 << ")." << colNames[j] << ": \""  << v1 << "\" != \"" << v2 << "\""; }              
            }
            break;
         }
         if ( !oss.str().empty() ) { Message( oss.str(), WARNING ); ++g_totalDiffNumber; }
      }
   }
}

// add known rules for sorting table by columns
static void addDefaultRulesForTableSorting( std::map<std::string, std::vector<std::string> >  & tableSorterList )
{
   std::vector<std::string> fldList;
   fldList.push_back( "Time" );
   fldList.push_back( "PropertyName" );
   fldList.push_back( "FormationName" );
   fldList.push_back( "NodeIndex" );

   tableSorterList["1DTimeIoTbl"] = fldList;

   fldList.clear();

   fldList.push_back( "Time" );
   fldList.push_back( "PropertyName" );
   fldList.push_back( "FormationName" );

   tableSorterList["3DTimeIoTbl"] = fldList;

   fldList.push_back( "SurfaceName" );
   tableSorterList["TimeIoTbl"] = fldList;
}


///////////////////////////////////////////////////////////////////////////////
//
// Main function
//
///////////////////////////////////////////////////////////////////////////////
int main( int argc, char ** argv )
{
   double eps = 1.e-5;

   std::map<std::string, std::vector<std::string> >  tableSorterList;

   addDefaultRulesForTableSorting( tableSorterList );

   std::set<std::string> ignoreList; // list of tables which will be skipped
   std::set<std::string> procesList; // list of tables which will be processed

   if ( argc < 3 ) return Usage( argv[0] );

   const char * in1File = NULL;
   const char * in2File = NULL;

   // parser input args
   for ( int i = 1; i < argc; ++i )
   {
      if ( argv[i][0] == '-') // parser utility flags
      {
         if (      !strcmp( argv[i], "-q"        ) ) { g_VerboseLevel = 0; }
         if (      !strcmp( argv[i], "-v"        ) && argc > i+1 && argv[i+1][0] != '-' ) { g_VerboseLevel = atol( argv[++i] ); }
         else if ( !strcmp( argv[i], "-tol"      ) && argc > i+1 && argv[i+1][0] != '-' ) { eps            = atof( argv[++i] ); }
         else if ( !strcmp( argv[i], "-project1" ) && argc > i+1 && argv[i+1][0] != '-' ) { in1File        =       argv[++i];   }
         else if ( !strcmp( argv[i], "-project2" ) && argc > i+1 && argv[i+1][0] != '-' ) { in2File        =       argv[++i];   }
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


   Message( std::string( "### Comparing list of tables..." ), PROGRESSMSG, false );
   const std::vector<std::string> & commonTblLst = compareTablesNames( cldProject1, cldProject2, ignoreList, procesList );

   Message( std::string( " done in " ) + to_string( (WallTime::clock() - timer).floatValue() ) + " sec.", PROGRESSMSG );
   timer = WallTime::clock();
   
   Message( std::string( "#### Comparing records in tables..." ), PROGRESSMSG );
   for ( size_t i = 0; i < commonTblLst.size(); ++i )
   {
      if ( tableSorterList.count( commonTblLst[i] ) )
      {
         cldProject1.tableSort( commonTblLst[i], tableSorterList[commonTblLst[i]] );
         cldProject2.tableSort( commonTblLst[i], tableSorterList[commonTblLst[i]] );
      }
      Message( std::string( "###### Table: " ) + commonTblLst[i], PROGRESSMSG, false );
      compareTable( cldProject1, cldProject2, commonTblLst[i], eps );
      Message( std::string( " done in " ) + to_string( (WallTime::clock() - timer).floatValue() ) + " sec.", PROGRESSMSG );
      timer = WallTime::clock();
   }

   Message( "###### All done!", PROGRESSMSG );
   
   std::ostringstream oss;
   oss << "Found " << g_totalDiffNumber << " differences";
   Message( oss.str(), 0 );

   return 0;
}


