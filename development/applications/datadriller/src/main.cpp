//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "datadriller.h"

#include <algorithm>
#include <cstring>
#include <iostream>

static void showUsage ( const char* command, const char* message = 0 );

static bool parseCmdLineArgs( int argc, char** argv,
                              std::string& inputProjectFileName,
                              std::string& outputProjectFileName,
                              bool& calculateTrapAndMissingProperties );

int main( int argc, char** argv )
{
  std::string inputProjectFileName;
  std::string outputProjectFileName;
  bool calculateTrapAndMissingProperties = true;

  if ( !parseCmdLineArgs( argc, argv, inputProjectFileName, outputProjectFileName, calculateTrapAndMissingProperties ) )
  {
    return 1;
  }

  DataExtraction::DataDriller dataDriller( inputProjectFileName );
  dataDriller.run( calculateTrapAndMissingProperties );
  dataDriller.saveToFile( outputProjectFileName );

  return 0;
}

void showUsage( const char* command, const char* message )
{
  std::cerr << std::endl;

  if ( message != 0 )
  {
    std::cerr << command << ": "  << message << std::endl;
  }

  std::cerr << "Usage: " << command << std::endl
            << "\t-input <cauldron-project-file>          specify input file" << std::endl
            << "\t[-output <cauldron-project-file>]       specify output file" << std::endl
            << "\t[-help]                                 print this message." << std::endl
            << "\t[-lean]   lean mode, only read data available in HDF files [excluding Trap properties, Porosity and (Horizontal)Permeability]" << std::endl
            << std::endl;
}

bool parseCmdLineArgs( int argc, char** argv,
                       std::string& inputProjectFileName,
                       std::string& outputProjectFileName,
                       bool& calculateTrapAndMissingProperties )
{
  inputProjectFileName  = "";
  outputProjectFileName = "";

  for ( int arg = 1; arg < argc; arg++ )
  {
    if ( std::strncmp( argv[arg], "-input", std::max<std::size_t>( 2, std::strlen( argv[arg] ) ) ) == 0 )
    {
      if ( arg + 1 >= argc )
      {
        showUsage( argv[ 0 ], "Argument for '-input' is missing");
        return false;
      }
      inputProjectFileName = argv[++arg];
    }
    else if ( std::strncmp( argv[arg], "-output", std::max<std::size_t>( 2, std::strlen( argv[arg] ) ) ) == 0 )
    {
      if ( arg + 1 >= argc )
      {
        showUsage( argv[ 0 ], "Argument for '-output' is missing" );
        return false;
      }
      outputProjectFileName = argv[++arg];
    }
    else if ( std::strncmp( argv[arg], "-lean",    std::max<std::size_t>( 2, std::strlen( argv[arg] ) ) ) == 0 ) { calculateTrapAndMissingProperties = false; }
    else if ( std::strncmp( argv[arg], "-help",    std::max<std::size_t>( 2, std::strlen( argv[arg] ) ) ) == 0 ||
              std::strncmp( argv[arg], "-?",       std::max<std::size_t>( 2, std::strlen( argv[arg] ) ) ) == 0 ||
              std::strncmp( argv[arg], "-usage",   std::max<std::size_t>( 2, std::strlen( argv[arg] ) ) ) == 0 )  { showUsage( argv[ 0 ], "Standard usage."  ); return false; }
    else { showUsage( argv[ 0 ], "Unknown argument" ); return false; }
  }

  if ( inputProjectFileName.empty()  ) { showUsage ( argv[ 0 ], "No project file specified"); return false; }
  if ( outputProjectFileName.empty() ) { outputProjectFileName = inputProjectFileName;  }

  return true;
}

