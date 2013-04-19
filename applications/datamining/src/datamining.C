#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <cmath>

#include <vector>
#include <map>
#include <string>
#include <cstdlib>

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>

using namespace std;

// DataAccess API includes
#include "Interface/GridMap.h"
#include "Interface/Grid.h"
#include "Interface/Snapshot.h"
#include "Interface/Surface.h"
#include "Interface/Formation.h"
#include "Interface/Reservoir.h"
#include "Interface/LithoType.h"
#include "Interface/Property.h"
#include "Interface/PropertyValue.h"
#include "Interface/ProjectHandle.h"

#include "NumericFunctions.h"

#include "ProjectHandle.h"
#include "ElementPosition.h"
#include "Point.h"
#include "PropertyInterpolator2D.h"
#include "PropertyInterpolator3D.h"
#include "CauldronDomain.h"
#include "InterpolatedPropertyValues.h"
#include "DomainProperty.h"
#include "DomainPropertyFactory.h"
#include "DomainPropertyCollection.h"

#include "DomainSurfaceProperty.h"
#include "DomainFormationProperty.h"
#include "DomainReservoirProperty.h"

#include "PieceWiseInterpolator1D.h"

#include "DeviatedWell.h"
#include "DataMiner.h"

#include "array.h"

#include <string>
#include <vector>
using namespace std;

using namespace DataAccess;
using namespace Interface;
using namespace Numerics;
using namespace Mining;



static void showUsage ( const char* command,
                        const char* message = 0);


int main (int argc, char ** argv) {

   string projectFileName;
   double snapshotTime = 0;
   bool verbose = false;
   bool debug = false;
   int i;

   double x = 0.0;
   double y = 0.0;
   double z = 0.0;

   int arg;
   for (arg = 1; arg < argc; arg++)
   {
      if (strncmp (argv[arg], "-project", NumericFunctions::Maximum<size_t> (2, strlen (argv[arg]))) == 0)
      {
         if (arg + 1 >= argc)
         {
            showUsage ( argv[ 0 ], "Argument for '-project' is missing");
            return -1;
         }
         projectFileName = argv[++arg];
      }
      else if (strncmp (argv[arg], "-snapshot", NumericFunctions::Maximum<size_t> (2, strlen (argv[arg]))) == 0)
      {
         if (arg + 1 >= argc)
         {
            showUsage ( argv[ 0 ], "Argument for '-snapshot' is missing");
            return -1;
         }
         snapshotTime = atof (argv[++arg]);
      }
      else if (strncmp (argv[arg], "-point", NumericFunctions::Maximum<size_t> (2, strlen (argv[arg]))) == 0)
      {
         if (arg + 1 >= argc)
         {
            showUsage ( argv[ 0 ], "Argument for '-point' is missing");
            return -1;
         }
         x = atof (argv[++arg]);
         y = atof (argv[++arg]);
         z = atof (argv[++arg]);
      }
      else if (strncmp (argv[arg], "-debug", NumericFunctions::Maximum<size_t> (2, strlen (argv[arg]))) == 0)
      {
         debug = true;
      }
      else if (strncmp (argv[arg], "-help", NumericFunctions::Maximum<size_t> (2, strlen (argv[arg]))) == 0)
      {
         showUsage ( argv[ 0 ], " Standard usage.");
         return -1;
      }
      else if (strncmp (argv[arg], "-?", NumericFunctions::Maximum<size_t> (2, strlen (argv[arg]))) == 0)
      {
         showUsage ( argv[ 0 ], " Standard usage.");
         return -1;
      }
      else if (strncmp (argv[arg], "-usage", NumericFunctions::Maximum<size_t> (2, strlen (argv[arg]))) == 0)
      {
         showUsage ( argv[ 0 ], " Standard usage.");
         return -1;
      }
      else if (strncmp (argv[arg], "-verbose", NumericFunctions::Maximum<size_t> (2, strlen (argv[arg]))) == 0)
      {
         verbose = true;
      }
      else
      {
         showUsage ( argv[ 0 ]);
         return -1;
      }
   }

   if (projectFileName == "")
   {
      showUsage ( argv[ 0 ], "No project file specified");
      return -1;
   }

   Mining::DomainPropertyFactory* factory = new DataAccess::Mining::DomainPropertyFactory;

   Interface::ProjectHandle::UseFactory ( factory );

   Mining::ProjectHandle* projectHandle = (Mining::ProjectHandle*)(OpenCauldronProject (projectFileName, "r"));

   CauldronDomain domain ( projectHandle );
   const Interface::Snapshot* snapshot = projectHandle->findSnapshot ( snapshotTime );

   DomainPropertyCollection* domainProperties = projectHandle->getDomainPropertyCollection ();

   domain.setSnapshot ( snapshot );
   domainProperties->setSnapshot ( snapshot );

   DataMiner miner ( projectHandle );

   ElementPositionSequence positions;
   DataMiner::ResultValues results;
   DataMiner::PropertySet properties;

   PieceWiseInterpolator1D interp;
   ElementPosition element;

   const Interface::Property* property = projectHandle->findProperty ( "Porosity" );
   const Interface::Property* property2 = projectHandle->findProperty ( "BulkDensity" );
   const Interface::Property* property3 = projectHandle->findProperty ( "Depth" );
   const Interface::Property* property4 = projectHandle->findProperty ( "LithoStaticPressure" );
   const Interface::Property* property5 = projectHandle->findProperty ( "Permeability" );


   DeviatedWell well ("deviatedWell");
   ElementPositionSequence elements;

   std::vector<Numerics::Point> wellPoints;

   wellPoints.push_back ( Numerics::Point ( 1300.0, 5000.0, 0.0 ));
   wellPoints.push_back ( Numerics::Point ( 2500.0, 2500.0, 500.0 ));
   wellPoints.push_back ( Numerics::Point ( 5000.0, 1300.0, 1000.0 ));
   wellPoints.push_back ( Numerics::Point ( 8000.0, 2500.0, 1200.0 ));
   wellPoints.push_back ( Numerics::Point ( 9000.0, 5000.0, 1300.0 ));
   wellPoints.push_back ( Numerics::Point ( 7000.0, 8500.0, 1800.0 ));
   wellPoints.push_back ( Numerics::Point ( 3500.0, 5000.0, 2000.0 ));
   wellPoints.push_back ( Numerics::Point ( 1300.0, 5000.0, 2200.0 ));
   wellPoints.push_back ( Numerics::Point ( 2500.0, 2500.0, 2500.0 ));
   wellPoints.push_back ( Numerics::Point ( 5000.0, 1300.0, 3000.0 ));
   wellPoints.push_back ( Numerics::Point ( 8000.0, 2500.0, 3200.0 ));
   wellPoints.push_back ( Numerics::Point ( 9000.0, 5000.0, 3300.0 ));
   wellPoints.push_back ( Numerics::Point ( 7000.0, 8500.0, 3800.0 ));
   wellPoints.push_back ( Numerics::Point ( 3500.0, 5000.0, 3990.0 ));

   double depthAlongHole = 0.0;
   well.addLocation ( wellPoints [ 0 ], depthAlongHole );

   for ( i = 1; i < wellPoints.size (); ++i ) {
      depthAlongHole += separationDistance ( wellPoints [ i - 1 ], wellPoints [ i ]) + 10.0;
      well.addLocation ( wellPoints [ i ], depthAlongHole );

      cout << " point " << i << "  " << wellPoints [ i ].image () << endl;

   }

   well.freeze ();
   cout << "elements.size " << elements.size () << endl;
   domain.findWellPath ( well, elements, false, false, false );

   for ( i = 1; i < elements.size (); ++i ) {
      cout << elements [ i ].image () << endl;
   }


   properties.push_back ( property );
   properties.push_back ( property2 );
   properties.push_back ( property3 );
   properties.push_back ( property4 );
   properties.push_back ( property5 );

   miner.compute ( elements, properties, results );

   for ( i = 0; i < results.size (); ++i ) {
      std::cout << setw ( 5 ) << i << "  " << elements [ i ].getActualPoint ().image () << "  " << results [ i ].image () << endl;
   }

   return 0;

//    cout.precision ( 15 );
//    cout.flags ( ios::scientific );

   cout << " first point " << endl;
   domain.findLocation ( 1345.0, 4321.0, 2304.0, element );
   positions.push_back ( element );
   cout << element.image () << std::endl;
   cout << domainProperties->getDomainProperty ( property )->compute ( element ) << "  " << endl;
   cout << domainProperties->getDomainProperty ( property2 )->compute ( element ) << "  " << endl;
   cout << domainProperties->getDomainProperty ( property3 )->compute ( element ) << "  " << endl;
   cout << domainProperties->getDomainProperty ( property4 )->compute ( element ) << "  " << endl;
   cout << domainProperties->getDomainProperty ( property5 )->compute ( element ) << endl;
   cout << domainProperties->getDomainProperty ( property )->compute ( element ) << "  "
        << domainProperties->getDomainProperty ( property2 )->compute ( element ) << "  "
        << domainProperties->getDomainProperty ( property3 )->compute ( element ) << "  "
        << domainProperties->getDomainProperty ( property4 )->compute ( element ) << "  "
        << domainProperties->getDomainProperty ( property5 )->compute ( element ) << endl;

   cout << endl << endl << " second point " << endl;
   domain.findLocation ( 1500.0, 4000.0, 2304.0, element );
   positions.push_back ( element );
   cout << element.image () << std::endl;
   cout << domainProperties->getDomainProperty ( property )->compute ( element ) << "  "
        << domainProperties->getDomainProperty ( property2 )->compute ( element ) << "  "
        << domainProperties->getDomainProperty ( property3 )->compute ( element ) << "  "
        << domainProperties->getDomainProperty ( property4 )->compute ( element ) << "  "
        << domainProperties->getDomainProperty ( property5 )->compute ( element ) << endl;

   cout << endl << endl << " third point " << endl;
   domain.findLocation ( 1500.1, 4000.1, 2304.0, element );
   positions.push_back ( element );
   cout << element.image () << std::endl;
   cout << domainProperties->getDomainProperty ( property )->compute ( element ) << "  "
        << domainProperties->getDomainProperty ( property2 )->compute ( element ) << "  "
        << domainProperties->getDomainProperty ( property3 )->compute ( element ) << "  "
        << domainProperties->getDomainProperty ( property4 )->compute ( element ) << "  "
        << domainProperties->getDomainProperty ( property5 )->compute ( element ) << endl;

   cout << endl << endl << " fourth point " << endl;
   domain.findLocation ( 1499.0, 3999.0, 3900.0, element );
   positions.push_back ( element );
   cout << element.image () << std::endl;
   cout << domainProperties->getDomainProperty ( property )->compute ( element ) << "  "
        << domainProperties->getDomainProperty ( property2 )->compute ( element ) << "  "
        << domainProperties->getDomainProperty ( property3 )->compute ( element ) << "  "
        << domainProperties->getDomainProperty ( property4 )->compute ( element ) << "  "
        << domainProperties->getDomainProperty ( property5 )->compute ( element ) << endl;

   cout << endl << endl << " fifth point " << endl;
   domain.findLocation ( 1499.0, 3999.0, 5900.0, element );
   positions.push_back ( element );
   cout << element.image () << std::endl;
   cout << domainProperties->getDomainProperty ( property )->compute ( element ) << "  "
        << domainProperties->getDomainProperty ( property2 )->compute ( element ) << "  "
        << domainProperties->getDomainProperty ( property3 )->compute ( element ) << "  "
        << domainProperties->getDomainProperty ( property4 )->compute ( element ) << "  "
        << domainProperties->getDomainProperty ( property5 )->compute ( element ) << endl;


   cout << endl << endl << " fifth point " << endl;
   domain.findLocation ( 1499.0, 3999.0, projectHandle->findSurface ( "Surface2" ), element );
   positions.push_back ( element );
   cout << element.image () << std::endl;
   cout << domainProperties->getDomainProperty ( property )->compute ( element ) << "  "
        << domainProperties->getDomainProperty ( property2 )->compute ( element ) << "  "
        << domainProperties->getDomainProperty ( property3 )->compute ( element ) << "  "
        << domainProperties->getDomainProperty ( property4 )->compute ( element ) << "  "
        << domainProperties->getDomainProperty ( property5 )->compute ( element ) << endl;


   cout << endl << endl << " fifth point " << endl;
   domain.findLocation ( 1200.0, 3999.0, projectHandle->findSurface ( "Surface5" ), element );


   cout << endl << endl << " top surface " << endl;
   domain.getTopSurface ( 1200.0, 3999.0, element );
   positions.push_back ( element );
   cout << element.image () << std::endl;

   InterpolatedPropertyValues evals;

   cout << domainProperties->getDomainProperty ( property )->compute ( element ) << "  "
        << domainProperties->getDomainProperty ( property2 )->compute ( element ) << "  "
        << domainProperties->getDomainProperty ( property3 )->compute ( element ) << "  "
        << domainProperties->getDomainProperty ( property4 )->compute ( element ) << "  "
        << domainProperties->getDomainProperty ( property5 )->compute ( element ) << endl;

   cout << endl << endl << " bottom surface " << endl;
   domain.getBottomSurface ( 1200.0, 3999.0, element );
   positions.push_back ( element );
   cout << element.image () << std::endl;


   cout << domainProperties->getDomainProperty ( property )->compute ( element ) << "  "
        << domainProperties->getDomainProperty ( property2 )->compute ( element ) << "  "
        << domainProperties->getDomainProperty ( property3 )->compute ( element ) << "  "
        << domainProperties->getDomainProperty ( property4 )->compute ( element ) << "  "
        << domainProperties->getDomainProperty ( property5 )->compute ( element ) << endl;

//    domainProperties->getDomainProperty ( property )->compute ( element, evals );


   for ( i = 1; i <= 1; ++i ) { // 1000000
      domain.findLocation ( x, y, z, element );
//       domain.findLocation ( x, y, projectHandle->findSurface ( "Surface5" ), element );
//       domain.findLocation ( 1499.0, 3999.0, 3900.0, element );
//       domain.findLocation ( 1499.0, 3999.0, projectHandle->findSurface ( "Surface5" ), element );
   }

   cout << " after 1000000 searches: " << endl;
   cout << element.image () << std::endl;
   cout << domainProperties->getDomainProperty ( property )->compute ( element ) << "  "
        << domainProperties->getDomainProperty ( property2 )->compute ( element ) << "  "
        << domainProperties->getDomainProperty ( property3 )->compute ( element ) << "  "
        << domainProperties->getDomainProperty ( property4 )->compute ( element ) << "  "
        << domainProperties->getDomainProperty ( property5 )->compute ( element ) << endl;

   miner.compute ( positions, properties, results );

   for ( i = 0; i < results.size (); ++i ) {
      std::cout << positions [ i ].getActualPoint ().image () << "  " << results [ i ].image () << endl;
   }

   delete factory;
//    delete projectHandle;

   return 0;
}



void showUsage ( const char* command,
                 const char* message ) {

   std::cerr << std::endl;

   if ( message != 0 ) {
      std::cerr << command << ": "  << message << std::endl;
   }

   std::cerr << "Usage: " << command
             << " -project <cauldron-project-file>" << std::endl
             << " -help     Print this message." << std::endl
             << std::endl;

}

