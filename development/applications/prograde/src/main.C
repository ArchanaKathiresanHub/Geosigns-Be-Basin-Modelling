//
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
// This utility allow to load and then save the .project3d file
// if -clean command line option is given, utility will clean the following tables:
//   TimeIoTbl
//   3DtimeIoTbl
//   1DTimeIoTbl
//   DepthIoTbl
 
#include "cmbAPI.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include <cstring>


void Message( const std::string & msg )
{
   std::cout << msg << std::endl;
}

int Usage( const char * pName )
{
   std::cout << "Utility for cleaning and upgrading Cauldron project3d files" << std::endl;
   std::cout << "Usage: " << std::endl;
   std::cout << "   " << pName << " [-clean [-table <comma sep. tables list>]] [-project] in.project3d [[-save] out.project3d]" << std::endl;
   return 0;
}

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


int main( int argc, char ** argv )
{
   bool cleanResult = false;
   
   if ( argc == 1 ) return Usage( argv[0] );

   const char * inFile = NULL;
   const char * outFile = NULL;
   
   std::vector<std::string> tablesList;

   // parser input args
   for ( int i = 1; i < argc; ++i )
   {
      if ( argv[i][0] == '-') // parser utility flags
      {
         if (      !strcmp( argv[i], "-clean"   ) ) { cleanResult = true; }
         else if ( !strcmp( argv[i], "-project" ) && argc > i+1 && argv[i+1][0] != '-' ) { inFile = argv[++i]; }
         else if ( !strcmp( argv[i], "-save"    ) && argc > i+1 && argv[i+1][0] != '-' ) { outFile = argv[++i]; }
         else if ( !strcmp( argv[i], "-table"   ) && argc > i+1 && argv[i+1][0] != '-' )
         {
            tablesList = list2array( argv[++i], ',' );
         }
         else
         {
            Message( std::string( "Unknown parameter: " ) + argv[i] );
            return Usage( argv[0] );
         }
      }
      else
      {
         if (      !inFile  ) { inFile  = argv[i]; }
         else if ( !outFile ) { outFile = argv[i]; }
         else
         {
            Message( std::string( "Unknown parameter: " ) + argv[i] );
            return Usage( argv[0] );
         }
      }
   }

   std::string inFileName(  inFile  ? inFile : "Project.project3d" );
   std::string outFileName( outFile ? outFile : ( std::string( "prograde_out_" ) + inFileName ) );

   // load project
   mbapi::Model cldProject;
   
   std::cout << "Reading project file: " << inFileName << "..." << std::endl;
   if ( ErrorHandler::NoError != cldProject.loadModelFromProjectFile( inFileName.c_str() ) )
   {
      Message( std::string( "Failing to load project file: " ) + inFileName );
      return -1;
   }

   if ( cleanResult )
   {
      size_t numRecs = cldProject.tableSize( "TimeIoTbl" );
      std::cout << "Cleaning in project TimeIoTbl, ";
      cldProject.clearTable( "TimeIoTbl" );

      std::cout << "3DTimeIoTbl, ";
      numRecs += cldProject.tableSize( "3DTimeIoTbl" );
      cldProject.clearTable( "3DTimeIoTbl" );

      std::cout << "1DTimeIoTbl, ";
      numRecs += cldProject.tableSize( "1DTimeIoTbl" );
      cldProject.clearTable( "1DTimeIoTbl" );
      
      std::cout << "DepthIoTbl";
      numRecs += cldProject.tableSize( "DepthIoTbl" );
      cldProject.clearTable( "DepthIoTbl" );

      std::cout << "SimulationDetailsIoTbl";
      numRecs += cldProject.tableSize( "SimulationDetailsIoTbl" );
      cldProject.clearTable( "SimulationDetailsIoTbl" );
      
      for ( size_t i = 0; i < tablesList.size(); ++i )
      {
         numRecs += cldProject.tableSize( tablesList[i] );
         cldProject.clearTable( tablesList[i] );
      }
      std::cout << std::endl << "Deleted " << numRecs << " records in all tables" << std::endl;
   }

   std::cout << "Saving project as: " << outFileName << "..." << std::endl;
   if ( ErrorHandler::NoError != cldProject.saveModelToProjectFile( outFileName.c_str() ) )
   {
      Message( std::string( "Failing to load project file: " ) + inFile );
      return -1;
   }

   std::cout << "All done!" << std::endl;
   return 0;
}
