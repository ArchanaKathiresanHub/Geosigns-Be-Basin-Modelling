//
// Copyright (C) 2012-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

//wellReader
#include "wellReader.h"

//cmbAPI
#include "cmbAPI.h"

//std
#include <cstring>
#include <string>

//utilities
#include "LogHandler.h"
#include "NumericFunctions.h"


/// @brief Prints usage of the mapMorph application
/// @details Use cout and not the LogHandler as it may not yet be initialzed
void usage();

/// @brief Initialize log handler
/// @param[in] verbosityLevel The log verbosity
/// @return True if initialization was successfull
bool initLogHandler( const char* verbosityLevel );

/// @brief Reads the command line arguments
bool readCommandLineArguments( const int argc, char** argv, const char*& inFile, const char*& outFile, const char*& wellFile,
                               std::vector<double>& shifts, std::vector<double>& scales, double& radiusOfInfluence, const char*& verbosityLevel );

const std::string s_basementTblName          = "BasementIoTbl";
const std::string s_topCrustHeatProdGrid     = "TopCrustHeatProdGrid";
const std::string s_topCrustHeatProd         = "TopCrustHeatProd";
const std::vector<std::string> s_mapAddition = {"_Min","_Max"};
const std::string s_mapFileSuffix            = "_Morphed.HDF";

int main( const int argc, char** argv )
{
  const char* inFile;
  const char* outFile;
  const char* wellFile;
  const char* verbosityLevel;
  std::vector<double> scales = {1.0, 1.0};
  std::vector<double> shifts = {0.0, 0.0};
  double radiusOfInfluence = 1.e3;

  if ( !readCommandLineArguments( argc, argv, inFile, outFile, wellFile, shifts, scales, radiusOfInfluence, verbosityLevel ) )
  {
    usage();
    return 1;
  }

  std::vector<double> xWells;
  std::vector<double> yWells;
  if ( wellFile && !WellReader::readWells(wellFile, xWells, yWells) )
  {
    LogHandler( LogHandler::ERROR_SEVERITY ) << "Error reading wells from file: " << wellFile;
    return 1;
  }

  if ( !initLogHandler( verbosityLevel ) )
  {
    return 1;
  }

  // Setup IO file names
  const std::string inFileName ( inFile  ? inFile : "Project.project3d" );
  const std::string outFileName( outFile ? outFile : "mapMorph_" + inFileName );
  LogHandler( LogHandler::INFO_SEVERITY, LogHandler::TITLE ) << "mapMorph";

  // Load project
  mbapi::Model cldModel;
  if ( ErrorHandler::NoError != cldModel.loadModelFromProjectFile( inFileName.c_str() ) )
  {
    LogHandler( LogHandler::ERROR_SEVERITY ) << "Failed to load project file: " << inFileName;
    return -1;
  }
  LogHandler( LogHandler::INFO_SEVERITY, LogHandler::SUBSECTION ) << "Successfully read project file: " << inFileName;

  // Get the top crust heat production map name
  const std::string & mapName = cldModel.tableValueAsString( s_basementTblName, 0, s_topCrustHeatProdGrid );
  if ( mapName.empty() )
  {
    LogHandler( LogHandler::ERROR_SEVERITY ) << "Project file " << inFileName << " does not contain a map for the top crust heat production";
    return -1;
  }
  LogHandler( LogHandler::INFO_SEVERITY, LogHandler::SUBSECTION ) << "Successfully found top crust heat production map: " << mapName;

  LogHandler( LogHandler::INFO_SEVERITY, LogHandler::SUBSECTION ) << "Morphing map";
  mbapi::MapsManager & mpMgr = cldModel.mapsManager();
  try
  {
    if ( ErrorHandler::NoError != cldModel.errorCode() )
    {
      throw ErrorHandler::Exception( cldModel.errorCode() ) << cldModel.errorMessage();
    }

    const mbapi::MapsManager::MapID mID = mpMgr.findID( mapName );
    if ( IsValueUndefined( mID ) )
    {
      throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "Can't find the map: " << mapName
                                                                   << " defined for top crust heat production rate in maps catalog";
    }

    for ( unsigned int i = 0; i < s_mapAddition.size(); ++i )
    {
      double minVal, maxVal;
      if ( ErrorHandler::NoError != mpMgr.mapValuesRange( mID, minVal, maxVal ) )
      {
        throw ErrorHandler::Exception( mpMgr.errorCode() ) << mpMgr.errorMessage();
      }

      // Copy map to avoid influence on other project parts
      const std::string newMapName = mapName + s_mapAddition[i];
      mbapi::MapsManager::MapID cmID = mpMgr.copyMap( mID, newMapName, mapName + s_mapFileSuffix );
      if ( IsValueUndefined( cmID ) )
      {
        throw ErrorHandler::Exception( ErrorHandler::IoError ) << "Copy map " << mapName << " failed";
      }

      // Scale and save map
      if ( ErrorHandler::NoError != mpMgr.scaleAndShiftMapCorrectedForWells( cmID, scales[i], shifts[i], radiusOfInfluence, xWells, yWells ) ||
           ErrorHandler::NoError != mpMgr.saveMapToHDF( cmID, mapName + s_mapFileSuffix ) )
      {
        throw ErrorHandler::Exception( mpMgr.errorCode() ) << mpMgr.errorMessage();
      }
    }

    // Update and save project file
    if ( ErrorHandler::NoError != cldModel.setTableValue( s_basementTblName, 0, s_topCrustHeatProd, Utilities::Numerical::IbsNoDataValue ) ||
         ErrorHandler::NoError != cldModel.saveModelToProjectFile( outFileName.c_str() ) )
    {
      throw ErrorHandler::Exception( cldModel.errorCode() ) << cldModel.errorMessage();
    }
    LogHandler( LogHandler::INFO_SEVERITY, LogHandler::SUBSECTION ) << "Successfully saved project file: " << outFileName;
  }
  catch( std::exception& ex )
  {
    LogHandler( LogHandler::ERROR_SEVERITY ) << "Could not morph map, see details below";
    LogHandler( LogHandler::ERROR_SEVERITY ) << ex.what();
    return -1;
  }
  catch (...)
  {
    LogHandler( LogHandler::FATAL_SEVERITY ) << "Unexpected error when morphing map";
    return -1;
  }

  return 0;
}

void usage()
{
  std::cout << "_________________________________________________________________________"         << std::endl
            << "Utility for morphing top crust heat production maps into a min map and a max map"  << std::endl
            << std::endl
            << "Usage:"                                                                            << std::endl
            << "mapMorph  [-project] <in.project3d> [options]"                                     << std::endl
            << "Morph the top crust heat production map in in.project3d and add maps to new file mapMorph_in.project3d, in.project3d is not modified."           << std::endl
            << "The morphed min and max maps are stored in a new HDF file."                                                                                      << std::endl
            << "   options:"                                                                                                                                     << std::endl
            << "      -h | --help                       Print this help"                                                                                         << std::endl
            << "      -save <out.project3d>             Saves the upgraded in.project3d to out.project3d instead of mapMorph_in.project3d"                       << std::endl
            << "      -wells <wells.in>                 File containing well locations, at well location maps is not morphed"                                    << std::endl
            << "      -radius <radius>                  Radius of influence [m] around a well location, where the morphing is slowly increased"                  << std::endl
            << "      -shiftmin <shift>                 Shift value for min map, absolute value by which the map is increased"                                   << std::endl
            << "      -shiftmax <shift>                 Shift value for max map, absolute value by which the map is increased"                                   << std::endl
            << "      -scalemin <scale>                 Scaling value, multiplication factor for min map, map x is scaled as x_new = x_old * scale + shift"      << std::endl
            << "      -scalemax <scale>                 Scaling value, multiplication factor for max map, map x is scaled as x_new = x_old * scale + shift"      << std::endl
            << "      -verbosity <verbosity>            Sets log handler verbosity, default is detailed (quiet||minimal||normal||detailed||diagnostic)"          << std::endl
            << std::endl
            << "   examples:"                                                                                                                                    << std::endl
            << "      1. Morph the top crust heat production map in in.project3d and save it to its default name"                                                << std::endl
            << "         mapMorph -project in.project3d -wells wells.in -radius 500 -shiftmin 0.8 -scalemin 0.5 -shiftmax 0.6 -scalemax 2.0"                     << std::endl
            << "      2. Scale the top crust heat producion map in in.project3d by a factor 0.7 and 2.3 and save the project file to p10.p3d, no wells selected" << std::endl
            << "         mapMorph -project in.project3d -save p10.p3d -scalemin 0.7 -scalemax 2.3"                                                               << std::endl
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
      LogHandler( "mapMorph", LogHandler::DETAILED_LEVEL );
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

bool readCommandLineArguments( const int argc, char** argv, const char*& inFile, const char*& outFile, const char*& wellFile,
                               std::vector<double>& shifts, std::vector<double>& scales, double& radiusOfInfluence, const char*& verbosityLevel )
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
    if ( argv[i][0] != '-' ||
         !std::strcmp( argv[i], "-h" ) || !std::strcmp( argv[i], "--help" ) ||
         argc <= i+1 )
    {
      return false;
    }
    if ( !std::strcmp( argv[i], "-verbosity") )
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
    else if ( !std::strcmp( argv[i], "-wells" ) )
    {
      wellFile = argv[++i];
    }
    else if ( !std::strcmp( argv[i], "-shiftmin" ) )
    {
      shifts[0] = std::atof(argv[++i]);
    }
    else if ( !std::strcmp( argv[i], "-shiftmax" ) )
    {
      shifts[1] = std::atof(argv[++i]);
    }
    else if ( !std::strcmp( argv[i], "-scalemin" ) )
    {
      scales[0] = std::atof(argv[++i]);
    }
    else if ( !std::strcmp( argv[i], "-scalemax" ) )
    {
      scales[1] = std::atof(argv[++i]);
    }
    else if ( !std::strcmp( argv[i], "-radius" ) )
    {
      radiusOfInfluence = std::atof(argv[++i]);
    }
    else
    {
      std::cerr << std::string( "Unknown parameter: " ) + argv[i] << std::endl;
      return false;
    }
  }

  return true;
}

