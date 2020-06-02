//
// Copyright (C) 2012-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/// @file main.C This utility allow to load a legacy .project3d file and then save its updated version
///    if -clean command line option is given, utility will clean the following tables:
///       * TimeIoTbl
///       * 3DtimeIoTbl
///       * 1DTimeIoTbl

//CmbAPI
#include "cmbAPI.h"

//std
#include <iostream>
#include <sstream>
#include <string>
#include <cstring>

//utilities
#include "LogHandler.h"

//Prograde
#include "UpgradeManagers.h"

/// @brief Prints message as cerr, and adds end of line
/// @details Used only before the LogHandler is initialyzed
void MessageError( const std::string & msg )
{
   std::cerr << msg << std::endl;
}

/// @brief Prints usage of the propgrade application
/// @details Use cout and not the LogHandler as it may not yet be initialzed
int Usage()
{
   std::cout << "_________________________________________________________________________" << std::endl;
   std::cout << "Utility for upgrading and cleaning project3d files"                        << std::endl << std::endl;

   std::cout << "Usage:"                                                                                         << std::endl;
   std::cout << "prograde  [-project] <in.project3d> [options]"                                                  << std::endl;
   std::cout << "upgrade the in.project3d and save it to prograde_out_in.project3d in.project3d is not modified" << std::endl;
   std::cout << "   options:"                                                                                                                                    << std::endl <<
                "      -h | --help                       Print this help"                                                                                        << std::endl <<
                "      -save <out.project3d>             Saves the upgraded in.project3d to out.project3d instead of prograde_out_in.project3d"                  << std::endl <<
                "      -clean [<comma sep. tables list>] Clean the TimeIoTbl, 3DTimeIoTbl, 1DTimeIoTbl and SimulationDetailsIoTbl and extra tables if specified" << std::endl <<
                "      -verbosity <verbosity>            Sets log handler verbosity, default is detailed (quiet||minimal||normal||detailed||diagnostic)"         << std::endl << std::endl;

   std::cout << "   examples:"                                                                                                                                        << std::endl <<
                "      1. Upgrade a project3d and save it to its default name"                                                                                                  << std::endl <<
                "         prograde -project in.project3d"                                                                                                                       << std::endl <<
                "      2. Upgrade a project3d and save it to p10.p3d"                                                                                                           << std::endl <<
                "         prograde -project in.project3d -save p10.p3d"                                                                                                         << std::endl <<
                "      3. Upgrade a project3d, clean TimeIoTbl, 3DTimeIoTbl, 1DTimeIoTbl and SimulationDetailsIoTbl and save it to p10.p3d"                                     << std::endl <<
                "         prograde -project in.project3d -clean -save p10.p3d"                                                                                                  << std::endl <<
                "      4. Upgrade a project3d, clean TimeIoTbl, 3DTimeIoTbl, 1DTimeIoTbl and SimulationDetailsIoTbl, RandomIoTbl and AnotherRandomIoTbl and save it to p10.p3d" << std::endl <<
                "         prograde -project in.project3d -clean RandomIoTbl,AnotherRandomIoTbl -save p10.p3d"                                                                   << std::endl;
   std::cout << "_________________________________________________________________________" << std::endl;
   return 0;
}

/// @brief Parse the given string using the specified delimiter and store the result in a vector
/// @return The parsed string as a vector of sub-string
static std::vector<std::string> List2Array( const std::string & listOfStr, const char sep )
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

/// @brief Initialyze log hanlder
/// @param[in] verbosityLevel The log verbosity
/// @throw formattingexception::GeneralException if the verbosity level is unknwown
void InitLogHandler(const char* verbosityLevel)
{
    if (verbosityLevel) {
        if      (!strcmp( verbosityLevel, "quiet"      )) { LogHandler( "prograde", LogHandler::QUIET_LEVEL      ); }
        else if (!strcmp( verbosityLevel, "minimal"    )) { LogHandler( "prograde", LogHandler::MINIMAL_LEVEL    ); }
        else if (!strcmp( verbosityLevel, "normal"     )) { LogHandler( "prograde", LogHandler::NORMAL_LEVEL     ); }
        else if (!strcmp( verbosityLevel, "detailed"   )) { LogHandler( "prograde", LogHandler::DETAILED_LEVEL   ); }
        else if (!strcmp( verbosityLevel, "diagnostic" )) { LogHandler( "prograde", LogHandler::DIAGNOSTIC_LEVEL ); }
        else throw std::invalid_argument ("Unknown <" + std::string( verbosityLevel ) + "> option for -verbosity command line parameter.");
    }
    else {
        LogHandler( "prograde", LogHandler::DETAILED_LEVEL );
    }
}

/// @brief Reads the command line arguments
bool ReadCommandLineArguments(const int argc, char** argv,
    bool& cleanResult, const char*& inFile, const char*& outFile, const char*& verbosityLevel,
    std::vector<std::string>& tablesList, bool& printHelp)
{
    if ( argc == 1 )
    {
        return false;
    }

    cleanResult = false;
    inFile = nullptr;
    outFile = nullptr;
    verbosityLevel = nullptr;

    // parser input args
    for ( int i = 1; i < argc; ++i )
    {
        if ( argv[i][0] == '-') // parser utility flags
        {
            if      ( !strcmp( argv[i], "-h") or !strcmp( argv[i], "--help" ))
            {
               printHelp = true;
               return true;
            }
            if ( !strcmp( argv[i], "-clean" ))                                               { cleanResult = true;          }
            else if ( !strcmp( argv[i], "-verbosity") && argc > i+1 && argv[i+1][0] != '-' ) { verbosityLevel = argv[++i]; }
            else if ( !strcmp( argv[i], "-project"  ) && argc > i+1 && argv[i+1][0] != '-' ) { inFile = argv[++i];          }
            else if ( !strcmp( argv[i], "-save"     ) && argc > i+1 && argv[i+1][0] != '-' ) { outFile = argv[++i];         }
            else if ( !strcmp( argv[i], "-table"    ) && argc > i+1 && argv[i+1][0] != '-' )
            {
                tablesList = List2Array( argv[++i], ',' );
            }
            else
            {
                MessageError( std::string( "Unknown parameter: " ) + argv[i] );
                return false;
            }
        }
        else
        {
            if (      !inFile  ) { inFile  = argv[i]; }
            else if ( !outFile ) { outFile = argv[i]; }
            else
            {
                MessageError( std::string( "Unknown parameter: " ) + argv[i] );
                return false;
            }
        }
    }
    return true;
}

int main( const int argc, char ** argv )
{

   ////////////////////////////////////////////
   ///1. Read commmand line arguments

   bool cleanResult = false;
   bool printHelp   = false;
   const char* inFile;
   const char* outFile;
   const char* verbosityLevel;
   std::vector<std::string> tablesList;
   if (not ReadCommandLineArguments( argc, argv,
                                     cleanResult, inFile, outFile, verbosityLevel, tablesList, printHelp )) {
      Usage();
      return 1;
   }
   if (printHelp) return Usage();

   ////////////////////////////////////////////
   ///2. Intitialise prograde loger
   try{
       InitLogHandler(verbosityLevel);
   }
   catch (std::exception& ex){
      std::cerr << ex.what() << std::endl;
      Usage();
      return 1;
   }
   catch (...){
      std::cerr << "Fatal error when initialising log file(s)" << std::endl;
      return 1;
   }

   ////////////////////////////////////////////
   ///3. Run prograde
   // setup IO file names
   std::string inFileName (  inFile  ? inFile : "Project.project3d" );
   std::string outFileName( outFile ? outFile : ( std::string( "prograde_out_" ) + inFileName ) );
   LogHandler( LogHandler::INFO_SEVERITY, LogHandler::TITLE   ) << "PROGRADE";
   LogHandler( LogHandler::INFO_SEVERITY, LogHandler::SECTION ) << "Upgrading " << inFile << " to " << outFileName;

   // load project
   mbapi::Model cldProject;
   LogHandler( LogHandler::INFO_SEVERITY, LogHandler::SUBSECTION ) << "Reading project file: " << inFileName;
   if ( ErrorHandler::NoError != cldProject.loadModelFromProjectFile( inFileName.c_str() ) )
   {
      LogHandler( LogHandler::ERROR_SEVERITY ) << std::string( "Failing to load project file: " ) + inFileName;
      return -1;
   }

   // upgrade project
   try {
      LogHandler( LogHandler::INFO_SEVERITY, LogHandler::SUBSECTION ) << "Upgrading project file: " << inFileName;
      Prograde::UpgradeManagers upgradeManagers(cldProject);
      upgradeManagers.runAll();
   }
   catch( const std::exception& ex ){
       LogHandler( LogHandler::ERROR_SEVERITY ) << "Could not upgrade project file, see details below";
       LogHandler( LogHandler::ERROR_SEVERITY ) << ex.what()<<". Migration from BPA to BPA2 Basin Aborted...";
	   return -1;
   }
   catch (...) {
      LogHandler( LogHandler::FATAL_SEVERITY ) << "Unexpected error when upgrading project file. Migration from BPA to BPA2 Basin Aborted...";
	  return -1;
   }

   //clean project
   if ( cleanResult )
   {
      LogHandler( LogHandler::INFO_SEVERITY, LogHandler::SUBSECTION ) << "Cleaning project file: " << inFileName;
      size_t numRecs = cldProject.tableSize( "TimeIoTbl" );
      LogHandler( LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_STEP ) << "cleaning TimeIoTbl";
      cldProject.clearTable( "TimeIoTbl" );

      numRecs += cldProject.tableSize( "3DTimeIoTbl" );
      LogHandler( LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_STEP ) << "cleaning 3DTimeIoTbl";
      cldProject.clearTable( "3DTimeIoTbl" );

      numRecs += cldProject.tableSize( "1DTimeIoTbl" );
      LogHandler( LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_STEP ) << "cleaning 1DTimeIoTbl";
      cldProject.clearTable( "1DTimeIoTbl" );    

      numRecs += cldProject.tableSize( "SimulationDetailsIoTbl" );
      LogHandler( LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_STEP ) << "cleaning SimulationDetailsIoTbl";
      cldProject.clearTable( "SimulationDetailsIoTbl" );

      for (const auto & table : tablesList)
      {
         numRecs += cldProject.tableSize( table );
         cldProject.clearTable( table );
      }
      LogHandler( LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_STEP ) << "Deleted " << numRecs << " records in all tables";
   }

   ////////////////////////////////////////////
   ///4. Save results
   LogHandler( LogHandler::INFO_SEVERITY, LogHandler::SUBSECTION ) << "Saving project as: " << outFileName;
   if ( ErrorHandler::NoError != cldProject.saveModelToProjectFile( outFileName.c_str() ) )
   {
      LogHandler( LogHandler::ERROR_SEVERITY ) << std::string( "Failing to load project file: " ) + outFileName;
      return -1;
   }

   LogHandler( LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS ) << "Project file upgraded as: " << outFileName;
   return 0;
}
