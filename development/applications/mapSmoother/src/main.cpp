//
// Copyright (C) 2012-2019 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

//cmbAPI
#include "cmbAPI.h"

//std
#include <cstring>
#include <string>

//utilities
#include "LogHandler.h"
#include "NumericFunctions.h"




void usage();

bool initLogHandler( const char* verbosityLevel );

bool readCommandLineArguments( const int argc, char** argv, const char*& inFile, const char*& outFile, unsigned int& method,
                               double& smoothingRadius, unsigned int& nrOfThreads, const char*& verbosityLevel );

const std::string s_smoothMapsFileName       = "SmoothInput.HDF";
const std::string s_addition                 = "_smooth";

const std::string s_tableGridMap   = "GridMapIoTbl";
const std::string s_colReferredBy  = "ReferredBy";
const std::string s_colMapType     = "MapType";
const std::string s_colMapName     = "MapName";

const std::string s_tableBPANameMapping   = "BPANameMapping";
const std::string s_colTblIoMappingEncode = "TblIoMappingEncode";

int main( const int argc, char** argv )
{
  const char* inFile;
  const char* outFile;
  const char* verbosityLevel;
  double smoothingRadius = 1.e3;
  unsigned int method = 0;
  unsigned int nrOfThreads = 1;

  if ( !readCommandLineArguments( argc, argv, inFile, outFile, method, smoothingRadius, nrOfThreads, verbosityLevel ) )
  {
    usage();
    return 1;
  }

  if ( !initLogHandler( verbosityLevel ) )
  {
    return 1;
  }

  // Setup IO file names
  const std::string inFileName ( inFile  ? inFile : "Project.project3d" );
  const std::string outFileName( outFile ? outFile : "smooth_" + inFileName );
  LogHandler( LogHandler::INFO_SEVERITY, LogHandler::TITLE ) << "mapSmoother";

  // Load project
  mbapi::Model cldModel;
  if ( ErrorHandler::NoError != cldModel.loadModelFromProjectFile( inFileName.c_str() ) )
  {
    LogHandler( LogHandler::ERROR_SEVERITY ) << "Failed to load project file: " << inFileName;
    return -1;
  }
  LogHandler( LogHandler::INFO_SEVERITY, LogHandler::SUBSECTION ) << "Successfully read project file: " << inFileName;

  LogHandler( LogHandler::INFO_SEVERITY, LogHandler::SUBSECTION ) << "Smoothing maps";
  try
  {
    mbapi::MapsManager& mpMgr = cldModel.mapsManager();
    const int tableSize = cldModel.tableSize( s_tableGridMap );
    for (int i = 0; i < tableSize; ++i)
    {
      if ( cldModel.tableValueAsString( s_tableGridMap , i, s_colMapType ) != "HDF5" )
      {
        continue;
      }

      // Skip smoothened maps
      const std::string& mapName = cldModel.tableValueAsString( s_tableGridMap , i, s_colMapName );
      if ( mapName.find(s_addition) != std::string::npos )
      {
        continue;
      }

      // Skip OceaCrustalThicknessIoTbl table as the maps could already be subsampled
      if ( cldModel.tableValueAsString( s_tableGridMap , i, s_colReferredBy ) == "OceaCrustalThicknessIoTbl" )
      {
        continue;
      }

      const std::string newMapName = mapName + s_addition;
      mbapi::MapsManager::MapID cmID = mpMgr.copyMap( mpMgr.findID( mapName ), newMapName, s_smoothMapsFileName );
      if ( IsValueUndefined( cmID ) )
      {
        throw ErrorHandler::Exception( ErrorHandler::IoError ) << "Copy map " << mapName << " failed";
      }

      if ( ErrorHandler::NoError != mpMgr.smoothenGridMap( cmID, method, smoothingRadius, nrOfThreads ) ||
           ErrorHandler::NoError != mpMgr.saveMapToHDF( cmID, s_smoothMapsFileName ) )
      {
        throw ErrorHandler::Exception( mpMgr.errorCode() ) << mpMgr.errorMessage();
      }

      // Modify all referred tables
      const std::string tableReferredBy = cldModel.tableValueAsString( s_tableGridMap , i, s_colReferredBy );
      std::vector<datatype::DataType> colDataTypes;
      int col = 0;
      for ( const std::string columnName : cldModel.tableColumnsList(tableReferredBy, colDataTypes) )
      {
        if ( colDataTypes[col] == datatype::String )
        {
          for ( int j = 0; j < cldModel.tableSize( tableReferredBy ); ++j )
          {
            if ( cldModel.tableValueAsString( tableReferredBy, j, columnName ) == mapName &&
                 ErrorHandler::NoError != cldModel.setTableValue( tableReferredBy, j, columnName, newMapName ) )
            {
              throw ErrorHandler::Exception( cldModel.errorCode() ) << cldModel.errorMessage();
            }
          }
        }
        ++col;
      }

      // Modify BPA map name table
      const std::string bpaMapName = s_tableGridMap + ":" +  mapName;
      const std::string bpaNewMapName = s_tableGridMap + ":" + newMapName;
      for (int j = 0; j < cldModel.tableSize( s_tableBPANameMapping ); ++j )
      {
        if ( cldModel.tableValueAsString( s_tableBPANameMapping, j, s_colTblIoMappingEncode ) == bpaMapName )
        {
          if ( ErrorHandler::NoError != cldModel.setTableValue( s_tableBPANameMapping, j, s_colTblIoMappingEncode, bpaNewMapName ) )
          {
            throw ErrorHandler::Exception( cldModel.errorCode() ) << cldModel.errorMessage();
          }
        }
      }
    }

    // Save project file
    if ( ErrorHandler::NoError != cldModel.saveModelToProjectFile( outFileName.c_str() ) )
    {
      throw ErrorHandler::Exception( cldModel.errorCode() ) << cldModel.errorMessage();
    }
    LogHandler( LogHandler::INFO_SEVERITY, LogHandler::SUBSECTION ) << "Successfully saved project file: " << outFileName;
  }
  catch( std::exception& ex )
  {
    LogHandler( LogHandler::ERROR_SEVERITY ) << "Could not smoothen maps, see details below";
    LogHandler( LogHandler::ERROR_SEVERITY ) << ex.what();
    return -1;
  }
  catch (...)
  {
    LogHandler( LogHandler::FATAL_SEVERITY ) << "Unexpected error when smoothing map";
    return -1;
  }

  return 0;
}

void usage()
{
  std::cout << "_________________________________________________________________________"         << std::endl
            << "Utility for smoothing maps"                                                        << std::endl
            << std::endl
            << "Usage:"                                                                            << std::endl
            << "mapSmoother  [-project] <in.project3d> [options]"                                  << std::endl
            << "Smoothen input maps in in.project3d and add maps to new file smooth_in.project3d, in.project3d is not modified."                                   << std::endl
            << "The smoothened maps are stored in a new HDF file."                                                                                                 << std::endl
            << "   options:"                                                                                                                                       << std::endl
            << "      -h | --help                         Print this help"                                                                                         << std::endl
            << "      -save <out.project3d>               Saves the upgraded in.project3d to out.project3d instead of smooth_in.project3d"                         << std::endl
            << "      -gaussian(default) / movingaverage  Filter to be used"                                                                                       << std::endl
            << "      -radius <radius>                    Smoothing radius [m] "
            << "                                            Standard deviation in gaussian filter and mask size 4x bigger"                                         << std::endl
            << "                                            Half the mask size in moving average filter"                                                           << std::endl
            << "      -threads <nrOfThreads>              Number of threads, 1 by default "                                                                        << std::endl
            << "      -verbosity <verbosity>              Sets log handler verbosity, default is detailed (quiet||minimal||normal||detailed||diagnostic)"          << std::endl
            << std::endl
            << "   examples:"                                                                                               << std::endl
            << " mapSmoother -project Project.project3d -save Project_out.project3d -gaussian -radius 10000"                << std::endl
            << " mapSmoother -project Project.project3d -save Project_out.project3d -movingaverage -radius 5000 -threads 4" << std::endl
            << "_________________________________________________________________________" << std::endl;
}

bool initLogHandler(const char* verbosityLevel)
{
  try
  {
    if (verbosityLevel)
    {
      if      ( !strcmp( verbosityLevel, "quiet"      ) ) { LogHandler( "mapMorph", LogHandler::QUIET_LEVEL      ); }
      else if ( !strcmp( verbosityLevel, "minimal"    ) ) { LogHandler( "mapMorph", LogHandler::MINIMAL_LEVEL    ); }
      else if ( !strcmp( verbosityLevel, "normal"     ) ) { LogHandler( "mapMorph", LogHandler::NORMAL_LEVEL     ); }
      else if ( !strcmp( verbosityLevel, "detailed"   ) ) { LogHandler( "mapMorph", LogHandler::DETAILED_LEVEL   ); }
      else if ( !strcmp( verbosityLevel, "diagnostic" ) ) { LogHandler( "mapMorph", LogHandler::DIAGNOSTIC_LEVEL ); }
      else throw std::invalid_argument( "Unknown <" + std::string( verbosityLevel ) + "> option for -verbosity command line parameter.");
    }
    else
    {
      LogHandler( "mapSmoother", LogHandler::DETAILED_LEVEL );
    }
  }
  catch ( std::exception& ex )
  {
    std::cerr << ex.what() << std::endl;
    usage();
    return false;
  }
  catch ( ... )
  {
    std::cerr << "Fatal error when initialising log file(s)" << std::endl;
    return false;
  }
  return true;
}

bool readCommandLineArguments( const int argc, char** argv, const char*& inFile, const char*& outFile, unsigned int& method,
                               double& smoothingRadius, unsigned int& nrOfThreads, const char*& verbosityLevel )
{
  if ( argc == 1 )
  {
    return false;
  }

  inFile = nullptr;
  outFile = nullptr;
  verbosityLevel = nullptr;

  // Parser input args
  for ( int i = 1; i < argc; ++i )
  {
    if ( argv[i][0] != '-' || !std::strcmp( argv[i], "-h" ) || !std::strcmp( argv[i], "--help" ) )
    {
      return false;
    }

    if ( !std::strcmp( argv[i], "-gaussian") )
    {
      method = 0;
    }
    else if ( !std::strcmp( argv[i], "-movingaverage") )
    {
      method = 1;
    }
    else if ( argc <= i+1 )
    {
      return false;
    }
    else if ( !std::strcmp( argv[i], "-verbosity") )
    {
      verbosityLevel = argv[++i];
    }
    else if ( !std::strcmp( argv[i], "-project" ) )
    {
      inFile = argv[++i];
    }
    else if ( !std::strcmp( argv[i], "-save" ) )
    {
      outFile = argv[++i];
    }
    else if ( !std::strcmp( argv[i], "-radius" ) )
    {
      smoothingRadius = std::atof(argv[++i]);
    }
    else if ( !std::strcmp( argv[i], "-threads" ) )
    {
      nrOfThreads = static_cast<unsigned int>( std::atoi(argv[++i]) );
    }
    else
    {
      std::cerr << std::string( "Unknown parameter: " ) + argv[i] << std::endl;
      return false;
    }
  }

  return true;
}

