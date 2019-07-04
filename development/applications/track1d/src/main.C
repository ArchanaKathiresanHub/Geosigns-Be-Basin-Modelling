//
// Copyright ( C ) 2013-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "Formation.h"
#include "Snapshot.h"
#include "Surface.h"

#include "parseUtilities.h"
#include "track1d.h"

#include <algorithm>
#include <cstring>

static void showUsage( const char* command, const char* message = 0 );

int main( int argc, char** argv )
{
  DataExtraction::DoublePairVector realWorldCoordinatePairs;
  DataExtraction::DoublePairVector logicalCoordinatePairs;
  DataExtraction::StringVector propertyNames;
  DataExtraction::DoubleVector ages;

  DataExtraction::StringVector topSurfaceFormationNames;
  DataExtraction::StringVector bottomSurfaceFormationNames;
  DataExtraction::StringVector formationNames;
  DataExtraction::StringVector formationSurfaceNames;

  bool basement = false;
  bool all2Dproperties = false;
  bool all3Dproperties = false;
  bool listProperties = false;
  bool listSnapshots = false;
  bool listStratigraphy = false;
  bool history = false;
  bool lean = false;

  std::string inputProjectFileName = "";
  std::string outputFileName = "";

  for ( int arg = 1; arg < argc; arg++ )
  {
    if ( std::strncmp( argv[ arg ], "-coordinates", std::max( 2, ( int )strlen( argv[ arg ] ) ) ) == 0 )
    {
      if ( arg + 1 >= argc )
      {
        showUsage( argv[ 0 ], "Argument for '-coordinates' is missing" );
        return 1;
      }
      realWorldCoordinatePairs = DataExtraction::ParseUtilities::parseCoordinates( argv[ ++arg ] );
    }
    else if ( std::strncmp( argv[ arg ], "-logical-coordinates", std::max( 2, ( int )strlen( argv[ arg ] ) ) ) == 0 )
    {
      if ( arg + 1 >= argc )
      {
        showUsage( argv[ 0 ], "Argument for '-logical-coordinates' is missing" );
        return 1;
      }
      logicalCoordinatePairs = DataExtraction::ParseUtilities::parseCoordinates( argv[ ++arg ] );
    }
    else if ( std::strncmp( argv[ arg ], "-properties", std::max( 5, ( int )strlen( argv[ arg ] ) ) ) == 0 )
    {
      if ( arg + 1 >= argc || argv[ arg + 1 ][ 0 ] == '-' )
      {
        showUsage( argv[ 0 ], "Argument for '-properties' is missing" );
        return 1;
      }
      propertyNames = DataExtraction::ParseUtilities::parseStrings( argv[ ++arg ] );
    }
    else if ( std::strncmp( argv[ arg ], "-ages", std::max( 2, ( int )strlen( argv[ arg ] ) ) ) == 0 )
    {
      if ( arg + 1 >= argc || argv[ arg + 1 ][ 0 ] == '-' )
      {
        showUsage( argv[ 0 ], "Argument for '-ages' is missing" );
        return 1;
      }
      ages = DataExtraction::ParseUtilities::parseAges( argv[ ++arg ] );
    }
    else if ( std::strncmp( argv[ arg ], "-formations", std::max( 5, ( int )strlen( argv[ arg ] ) ) ) == 0 )
    {
      if ( arg + 1 >= argc || argv[ arg + 1 ][ 0 ] == '-' )
      {
        showUsage( argv[ 0 ], "Argument for '-formations' is missing" );
        return 1;
      }
      formationNames = DataExtraction::ParseUtilities::parseStrings( argv[ ++arg ] );
    }
    else if ( std::strncmp( argv[ arg ], "-surfaces", std::max( 5, ( int )strlen( argv[ arg ] ) ) ) == 0 )
    {
      if ( arg + 1 >= argc || argv[ arg + 1 ][ 0 ] == '-' )
      {
        showUsage( argv[ 0 ], "Argument for '-surfaces' is missing" );
        return 1;
      }
      formationSurfaceNames = DataExtraction::ParseUtilities::parseStrings( argv[ ++arg ] );
    }
    else if ( std::strncmp( argv[ arg ], "-topsurfaces", std::max( 2, ( int )strlen( argv[ arg ] ) ) ) == 0 )
    {
      if ( arg + 1 >= argc || argv[ arg + 1 ][ 0 ] == '-' )
      {
        showUsage( argv[ 0 ], "Argument for '-topsurfaces' is missing" );
        return 1;
      }
      topSurfaceFormationNames = DataExtraction::ParseUtilities::parseStrings( argv[ ++arg ] );
    }
    else if ( std::strncmp( argv[ arg ], "-bottomsurfaces", std::max( 3, ( int )strlen( argv[ arg ] ) ) ) == 0 )
    {
      if ( arg + 1 >= argc || argv[ arg + 1 ][ 0 ] == '-' )
      {
        showUsage( argv[ 0 ], "Argument for '-bottomsurfaces' is missing" );
        return 1;
      }
      bottomSurfaceFormationNames = DataExtraction::ParseUtilities::parseStrings( argv[ ++arg ] );
    }
    else if ( std::strncmp( argv[ arg ], "-history", std::max( 3, ( int )strlen( argv[ arg ] ) ) ) == 0 )
    {
      history = true;
    }
    else if ( std::strncmp( argv[ arg ], "-basement", std::max( 3, ( int )strlen( argv[ arg ] ) ) ) == 0 )
    {
      basement = true;
    }
    else if ( std::strncmp( argv[ arg ], "-all-2D-properties", std::max( 7, ( int )strlen( argv[ arg ] ) ) ) == 0 )
    {
      all2Dproperties = true;
    }
    else if ( std::strncmp( argv[ arg ], "-all-3D-properties", std::max( 7, ( int )strlen( argv[ arg ] ) ) ) == 0 )
    {
      all3Dproperties = true;
    }
    else if ( std::strncmp( argv[ arg ], "-list-properties", std::max( 7, ( int )strlen( argv[ arg ] ) ) ) == 0 )
    {
      listProperties = true;
    }
    else if ( std::strncmp( argv[ arg ], "-list-snapshots", std::max( 8, ( int )strlen( argv[ arg ] ) ) ) == 0 )
    {
      listSnapshots = true;
    }
    else if ( std::strncmp( argv[ arg ], "-list-stratigraphy", std::max( 8, ( int )strlen( argv[ arg ] ) ) ) == 0 )
    {
      listStratigraphy = true;
    }
    else if ( std::strncmp( argv[ arg ], "-lean", std::max( 4, ( int )strlen( argv[ arg ] ) ) ) == 0 )
    {
      lean = true;
    }
    else if ( std::strncmp( argv[ arg ], "-project", std::max( 5, ( int )strlen( argv[ arg ] ) ) ) == 0 )
    {
      if ( arg + 1 >= argc || argv[ arg + 1 ][ 0 ] == '-' )
      {
        showUsage( argv[ 0 ], "Argument for '-project' is missing" );
        return 1;
      }
      inputProjectFileName = argv[ ++arg ];
    }
    else if ( std::strncmp( argv[ arg ], "-help", std::max( 3, ( int )strlen( argv[ arg ] ) ) ) == 0 )
    {
      showUsage( argv[ 0 ], " Standard usage." );
      return 1;
    }
    else if ( std::strncmp( argv[ arg ], "-?", std::max( 2, ( int )strlen( argv[ arg ] ) ) ) == 0 )
    {
      showUsage( argv[ 0 ], " Standard usage." );
      return 1;
    }
    else if ( std::strncmp( argv[ arg ], "-save", std::max( 2, ( int )strlen( argv[ arg ] ) ) ) == 0 )
    {
      if ( arg + 1 >= argc || argv[ arg + 1 ][ 0 ] == '-' )
      {
        showUsage( argv[ 0 ], "Argument for '-save' is missing" );
        return 1;
      }
      outputFileName = argv[ ++arg ];
    }
    else if ( std::strncmp( argv[ arg ], "-usage", std::max( 2, ( int )strlen( argv[ arg ] ) ) ) == 0 )
    {
      showUsage( argv[ 0 ], " Standard usage." );
      return 1;
    }
    else if ( std::strncmp( argv[ arg ], "-", 1 ) != 0 )
    {
      inputProjectFileName = argv[ arg ];
    }
    else
    {
      std::cerr << std::endl << "Unknown or ambiguous option: " << argv[ arg ] << std::endl;
      showUsage( argv[ 0 ] );
      return 1;
    }
  }

  std::ofstream outputFile;
  std::ostream* p_outputStream =&std::cout;

  if ( outputFileName != "" )
  {
    outputFile.open( outputFileName.c_str() );
    if ( outputFile.fail() )
    {
      showUsage( argv[ 0 ], "Could not open specified output file" );
      return 1;
    }
    else
    {
      p_outputStream =&outputFile;
    }
  }

  if ( inputProjectFileName == "" )
  {
    showUsage( argv[ 0 ], "No project file specified" );
    return 1;
  }

  DataExtraction::Track1d track1d( inputProjectFileName );

  if ( !track1d.isCorrect() )
  {
    showUsage( argv[ 0 ], "Could not open specified project file" );
    return 1;
  }

  if ( listProperties || listSnapshots || listStratigraphy )
  {
    if ( listProperties ) track1d.doListProperties();
    if ( listSnapshots ) track1d.doListSnapshots();
    if ( listStratigraphy ) track1d.doListStratigraphy();
    return 0;
  }

  track1d.acquireCoordinatePairs( realWorldCoordinatePairs, logicalCoordinatePairs );
  track1d.acquireSnapshots( ages );
  track1d.acquireProperties( propertyNames, all2Dproperties, all3Dproperties );
  track1d.acquireFormationSurfacePairs( topSurfaceFormationNames, bottomSurfaceFormationNames, formationNames,
                                        formationSurfaceNames, basement );

  bool success = track1d.run( lean );
  if ( success )
  {
    track1d.writeOutputStream( *p_outputStream, history );
  }

  if ( outputFile.is_open() )
  {
    outputFile.close();
  }

  return success ? 0 : 1;
}

void showUsage( const char* command, const char* message )
{
  std::cout << std::endl;

  if ( message != 0 )
  {
    std::cout << command << ": " << message << std::endl;
  }

  std::cout << "Usage ( case sensitive!! ): " << command << std::endl << std::endl
            << "\t[-coordinates x1,y1,x2,y2....]                     real-world coordinates to produce output for" << std::endl
            << "\t[-logical-coordinates i1,j1,i2,j2....]             logical coordinates to produce output for" << std::endl
            << "\t[-properties name1,name2...]                       properties to produce output for" << std::endl
            << "\t[-ages age1[-age2],...]                            select snapshot ages using single values and/or ranges" << std::endl << std::endl
            << "\t[-history]                                         produce output in a time-centric instead of a depth-centric fashion" << std::endl << std::endl
            << "\t[-topsurfaces formation1,formation2...]            produce output for the surfaces at the top of the given formations" << std::endl
            << "\t[-bottomsurfaces formation1,formation2...]         produce output for the surfaces at the bottom of the given formations" << std::endl
            << "\t[-formations formation1,formation2...]             produce output for the given formations" << std::endl
            << "\t[-surfaces formation1:surface1,...]                produce output for the given formation surfaces" << std::endl << std::endl
            << "\t                                                   the four options above can include Crust or Mantle" << std::endl << std::endl
            << "\t[-basement]                                        produce output for the basement as well," << std::endl
            << "\t                                                   only needed if none of the three options above have been specified" << std::endl << std::endl
            << "\t[-project] projectname                             name of 3D Cauldron project file to produce output for" << std::endl
            << "\t[-save filename]                                   name of file to save output ( *.csv format ) table to, otherwise save to stdout" << std::endl
            << std::endl
            << "\t[-all-3D-properties]                               produce output for all 3D properties" << std::endl
            << "\t[-all-2D-properties]                               produce output for all 2D properties" << std::endl
            << "\t[-lean]                                            do not calculate derived properties if the data is not already available (prevent heavy operations)" << std::endl
            << "\t[-list-properties]                                 print a list of available properties and exit" << std::endl
            << "\t[-list-snapshots]                                  print a list of available snapshots and exit" << std::endl
            << "\t[-list-stratigraphy]                               print a list of available surfaces and formations and exit" << std::endl << std::endl
            << "\t[-help]                                            print this message and exit" << std::endl << std::endl;
  std::cout << "If names in an argument list contain spaces, put the list between double or single quotes, e.g:"
            << "\t-formations \"Dissolved Salt,Al Khalata\"" << std::endl;
  std::cout << "Bracketed options are optional and options may be abbreviated" << std::endl << std::endl;
}

