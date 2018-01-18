#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstdlib>
#include <cstring>
#include <string>

#include "WallTime.h"

#include "AllochthonousLithologySimulator.h"
#include "AllochthonousModellingFactory.h"


#ifdef FLEXLM
#undef FLEXLM
#endif

#ifdef DISABLE_FLEXLM
#undef FLEXLM
#else
#define FLEXLM 1
#endif

#ifdef FLEXLM
// FlexLM license handling
#include <EPTFlexLm.h>
#endif


using Numerics::FloatingPoint;

using namespace std;
using namespace AllochMod;


void stringToInt ( const char* str, int& value, bool& error );

void printUsage ( const char* commandName );

int main ( int argc, char* argv []) {

  WallTime::Time startTime;
  WallTime::Time endTime;
  WallTime::Duration executionTime;
  int    hours;
  int    minutes;
  double seconds;

  int i;
  int debugLevel = 0;
  bool error = false;
  bool mapOutputOnly = false;

  string projectFileName;
  string outputProjectFileName;

  #ifdef FLEXLM
  //------------------------------------------------------------//
  // FLEX licence manager
  //
  // Attempt to check out the license.
  
  int rc = EPTFLEXLM_OK;

  char feature[EPTFLEXLM_MAX_FEATURE_LEN];
  char version[EPTFLEXLM_MAX_VER_LEN];
  char errmessage[EPTFLEXLM_MAX_MESS_LEN];
   
  // FlexLM license handling only for node with rank = 0
  sprintf(feature, "ibs_cauldron_halo");

#ifdef IBSFLEXLMVERSION
  sprintf(version, IBSFLEXLMVERSION);
#else
  sprintf(version, "9999.99");
#endif
  fprintf (stderr, "version = %s\n", version);
  fprintf (stderr, "revision = %s\n", SVNREVISION);

  rc = EPTFlexLmInit(errmessage);

  if ( rc != EPTFLEXLM_OK ) {
    fprintf(stderr,
            "\n@@@@@@@@@@@@@@@\n Basin_Error: FlexLm license init problems: GeoMorph cannot start.\n Please contact your helpdesk\n@@@@@@@@@@@@@@@\n");
  }

  // FlexLM license handling: Checkout
  rc = EPTFlexLmCheckOut( feature, version, errmessage );

  if (rc == EPTFLEXLM_WARN) {
    fprintf(stderr,"\n@@@@@@@@@@@@@@@\n Basin_Warning: FlexLm license warning: GeoMorph will still start anyway.\n@@@@@@@@@@@@@@@\n");
  } else if ( rc != EPTFLEXLM_OK ) {
    fprintf(stderr,"\n@@@@@@@@@@@@@@@\n Basin_Error: FlexLm license error: GeoMorph cannot start.\n Please contact your helpdesk\n@@@@@@@@@@@@@@@\n");
  }

  if( rc != EPTFLEXLM_OK && rc != EPTFLEXLM_WARN) {
    // FlexLm license check in, close down and enable logging
    EPTFlexLmCheckIn( feature );
    EPTFlexLmTerminate();
    return 0;
  }
  //------------------------------------------------------------//
  #else
  std::cout << std::endl << std::endl 
            << " **************** GeoMorph warning FLEXLM is currently not active  ****************" 
            << std::endl << std::endl;
  #endif



  AllochthonousModellingFactory*   factory;

  startTime = WallTime::clock ();

  if ( argc == 1 ) {
    printUsage ( argv [ 0 ]);
    return 1;
  }

  for ( i = 1; i < argc;++i ) {

    if ( strcmp ( argv [ i ], "-project" ) == 0 ) {

      if ( i < argc - 1 ) {
        projectFileName = argv [ i + 1 ];
        i++;
      } else {
        std::cout << " Basin_Error No project file name given " << std::endl;
        printUsage ( argv [ 0 ]);
        return 1;
      }

    } else if ( strcmp ( argv [ i ], "-save" ) == 0 ) {

      if ( i < argc - 1 ) {
        outputProjectFileName = argv [ i + 1 ];
        i++;
      } else {
        std::cout << " Basin_Error No output project file name given " << std::endl;
        printUsage ( argv [ 0 ]);
        return 1;
      }

    } else if ( strcmp ( argv [ i ], "-help" ) == 0 || strcmp ( argv [ i ], "-?" ) == 0 ) {

      printUsage ( argv [ 0 ]);
      return 1;

    } else if ( strcmp ( argv [ i ], "-debug" ) == 0 ) {

      if ( i < argc - 1 ) {
        stringToInt ( argv [ i + 1 ], debugLevel, error );
      } else {
        std::cout << " Basin_Error No debug level given " << std::endl;
        printUsage ( argv [ 0 ]);
        return 1;
      }

      if ( error ) {
        std::cout << " Basin_Error Could not convert '" << argv [ i + 1 ] << "' to an integer " << std::endl;
        printUsage ( argv [ 0 ]);
        return 1;
      }

      i++;
    } else if ( strcmp ( argv [ i ], "-mapoutput" ) == 0 ) {
      mapOutputOnly = true;
    }

  }

  if ( projectFileName == "" ) {
    std::cout << " Basin_Error no project name given " << std::endl;
    printUsage ( argv [ 0 ]);
    return 1;
  }

  if ( outputProjectFileName == "" ) {
    outputProjectFileName = projectFileName;
  }

  factory = new AllochthonousModellingFactory;

  AllochthonousLithologySimulator* allochthonousSimulator;

  allochthonousSimulator = AllochthonousLithologySimulator::CreateFrom (projectFileName, factory);


  if ( mapOutputOnly ) {
    allochthonousSimulator->printMaps ( std::cout );
  } else {
    allochthonousSimulator->execute ( debugLevel );
    allochthonousSimulator->saveToFile ( outputProjectFileName );
  }

  endTime = WallTime::clock ();

  executionTime = endTime - startTime;

  executionTime.separate ( hours, minutes, seconds );
  cerr << " execution time : " << executionTime << "  " << hours << " hours " << minutes << " minutes " << seconds << " seconds " << endl;

  #ifdef FLEXLM
  // FlexLm license check in, close down and enable logging
  EPTFlexLmCheckIn( feature );
  EPTFlexLmTerminate();
  #endif


  return 0;
}


void stringToInt ( const char* str, int& value, bool& error ) {

  int   radix = 10;
  char* strEnd;

  value = int ( std::strtol ( str, &strEnd, radix ));
  error = ( strEnd == str );

}


void printUsage ( const char* commandName ) {

  cerr << endl;
  cerr << " usage: " << endl << commandName;
  cerr << "  -project <project-file-name>";
  cerr << "  [-save <new-project-file-name>]";
  cerr << "  [-help]";
  cerr << "  [-?]";
  cerr << "  [-debug <debug-level>]";
  cerr << "  [-smp <number-of-threads>]"  << endl;
  cerr << endl;

}
