#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <algorithm>

#include <cmath>

#include <vector>
#include <map>
#include <string>
#include <cstring>
#include <cstdlib>

#include <stdlib.h>

#ifndef _WIN32
#include <unistd.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>

using namespace std;

#include "database.h"
#include "cauldronschema.h"
#include "cauldronschemafuncs.h"

// DataAccess API includes
#include "Interface/GridMap.h"
#include "Interface/Grid.h"
#include "Interface/Snapshot.h"
#include "Interface/Surface.h"
#include "Interface/Formation.h"
#include "Interface/Reservoir.h"
#include "Interface/Trap.h"
#include "Interface/LithoType.h"
#include "Interface/Property.h"
#include "Interface/PropertyValue.h"
#include "Interface/ProjectHandle.h"
#include "Interface/ObjectFactory.h"

#include "PropertyAttribute.h"
#include "AbstractPropertyManager.h"
#include "DerivedPropertyManager.h"

#include "GeoPhysicsObjectFactory.h"
#include "GeoPhysicsProjectHandle.h"

// EosPack
#include "EosPack.h"


#include "array.h"

#include "OutputPropertyValue.h"
#include "FormationOutputPropertyValue.h"
#include "FormationMapOutputPropertyValue.h"
#include "FormationSurfaceOutputPropertyValue.h"
#include "SurfaceOutputPropertyValue.h"


#include <string>
#include <sstream>
#include <vector>
#include <utility>
#include <stdexcept>
using namespace std;

using namespace DataAccess;
using namespace Interface;
using namespace DerivedProperties;

#include "errorhandling.h"

bool debug = false;
bool basement = false;
bool all2Dproperties = false;
bool all3Dproperties = false;
bool listProperties = false;
bool listSnapshots = false;
bool listStratigraphy = false;




typedef pair <double, double> DoublePair;
typedef vector < DoublePair > DoublePairVector;
typedef vector < string > StringVector;
typedef vector < double > DoubleVector;

typedef map < const Property *, OutputPropertyValuePtr > OutputPropertyValueMap;
typedef map < const Formation *, OutputPropertyValueMap > FormationOutputPropertyValueMap;
typedef map < const Snapshot *, FormationOutputPropertyValueMap> SnapshotFormationOutputPropertyValueMap;


typedef map < const Formation *, unsigned int > FormationMaxKMap;

typedef map < const Property *, const GridMap * > GridMapsMap;

typedef pair < const Surface *, int > SubSurface;
typedef pair < const Formation *, SubSurface > FormationSurface;
typedef vector < FormationSurface > FormationSurfaceVector;


static bool splitString( char * string, char separator, char * & firstPart, char * & secondPart );
static bool parseCoordinates( DoublePairVector & coordinatePairs, char * coordinatesString );
static bool parseStrings( StringVector & strings, char * stringsString );
static bool parseAges( DoubleVector & ages, char * agesString );

static bool acquireSnapshots( ProjectHandle * projectHandle, SnapshotList & snapshots, DoubleVector & ages );

static bool acquireProperties ( ProjectHandle * projectHandle, 
                                const AbstractPropertyManager& propertyManager,
                                PropertyList & properties,
                                StringVector & propertyNames );

static bool acquireFormationSurfaces( ProjectHandle * projectHandle, FormationSurfaceVector & formationSurfacePairs, StringVector & formationNames, bool useTop );
static bool acquireFormations( ProjectHandle * projectHandle, FormationSurfaceVector & formationSurfacePairs, StringVector & formationNames );
static bool acquireSurfaces( ProjectHandle * projectHandle, FormationSurfaceVector & formationSurfacePairs, StringVector & formationSurfaceNames );

#if 0
static bool containsFormation( FormationSurfaceVector & formationSurfacePairs, const Formation * formation );
static bool containsSurface( FormationSurfaceVector & formationSurfacePairs, const Surface * surface );
static bool containsFormationSurface( FormationSurfaceVector & formationSurfacePairs, const Formation * formation, const Surface * surface, int index );
#endif

static bool specifiesFormationSurface( FormationSurface & formationSurface, const Formation * formation, const Surface * surface, int subSurfaceIndex );

static bool snapshotSorter( const Snapshot * snapshot1, const Snapshot * snapshot2 );
static bool snapshotIsEqual( const Snapshot * snapshot1, const Snapshot * snapshot2 );

void listOutputableProperties ( const GeoPhysics::ProjectHandle* projectHandle,
                                const DerivedPropertyManager&    propertyManager );


OutputPropertyValuePtr allocateOutputProperty ( DerivedProperties::AbstractPropertyManager& propertyManager, 
                                                const DataModel::AbstractProperty* property, 
                                                const DataModel::AbstractSnapshot* snapshot,
                                                const FormationSurface& formationItem );

static void outputSnapshotFormationData( ostream & outputStream
                                       , DoublePair & coordinatePair
                                       , const Snapshot * snapshot
                                       , FormationSurface & formationSurface
                                       , PropertyList & properties
                                       , SnapshotFormationOutputPropertyValueMap & allOutputPropertyValues
                                       , FormationSurfaceVector & formationSurfacePairs
                                       , double i
                                       , double j
                                       , unsigned int k
                                       , unsigned int maxK );

static void showUsage( const char* command, const char* message = 0 );

template <class T>
T Min( T x, T y )
{
   return ( x < y ? x : y );
}

template <class T>
T Max( T x, T y )
{
   return ( x > y ? x : y );
}

template <class T>
T Abs( T x )
{
   return ( x >= 0 ? x : -x );
}

template <class T>
void Swap( T & x, T & y )
{
   T v;
   v = x;
   x = y;
   y = v;
}

int main( int argc, char ** argv )
{
   DoublePairVector realWorldCoordinatePairs;
   DoublePairVector logicalCoordinatePairs;
   StringVector propertyNames;
   DoubleVector ages;

   StringVector topSurfaceFormationNames;
   StringVector bottomSurfaceFormationNames;
   StringVector formationNames;
   StringVector formationSurfaceNames;

   SnapshotList snapshots;
   PropertyList properties;

   FormationSurfaceVector formationSurfacePairs;

   bool versusDepth = true;
   bool reverseOutputOrder = false;

   string inputProjectFileName = "";
   string outputFileName = "";
   bool verbose = false;

   ofstream outputFile;

   ostream * p_outputStream = &cout;

   int arg;
   for ( arg = 1; arg < argc; arg++ )
   {
      if ( strncmp( argv[ arg ], "-coordinates", Max( 2, (int)strlen( argv[ arg ] ) ) ) == 0 )
      {
         if ( arg + 1 >= argc )
         {
            showUsage( argv[ 0 ], "Argument for '-coordinates' is missing" );
            return -1;
         }
         if ( !parseCoordinates( realWorldCoordinatePairs, argv[ ++arg ] ) )
         {
            showUsage( argv[ 0 ], "Illegal argument for '-coordinates'" );
            return -1;
         }
      }
      else if ( strncmp( argv[ arg ], "-logical-coordinates", Max( 2, (int)strlen( argv[ arg ] ) ) ) == 0 )
      {
         if ( arg + 1 >= argc )
         {
            showUsage( argv[ 0 ], "Argument for '-logical-coordinates' is missing" );
            return -1;
         }
         if ( !parseCoordinates( logicalCoordinatePairs, argv[ ++arg ] ) )
         {
            showUsage( argv[ 0 ], "Illegal argument for '-ij'" );
            return -1;
         }
      }
      else if ( strncmp( argv[ arg ], "-properties", Max( 5, (int)strlen( argv[ arg ] ) ) ) == 0 )
      {
         if ( arg + 1 >= argc || argv[ arg + 1 ][ 0 ] == '-' )
         {
            showUsage( argv[ 0 ], "Argument for '-properties' is missing" );
            return -1;
         }
         if ( !parseStrings( propertyNames, argv[ ++arg ] ) )
         {
            showUsage( argv[ 0 ], "Illegal argument for '-properties'" );
            return -1;
         }
      }
      else if ( strncmp( argv[ arg ], "-ages", Max( 2, (int)strlen( argv[ arg ] ) ) ) == 0 )
      {
         if ( arg + 1 >= argc || argv[ arg + 1 ][ 0 ] == '-' )
         {
            showUsage( argv[ 0 ], "Argument for '-ages' is missing" );
            return -1;
         }
         if ( !parseAges( ages, argv[ ++arg ] ) )
         {
            showUsage( argv[ 0 ], "Illegal argument for '-ages'" );
            return -1;
         }
      }
      else if ( strncmp( argv[ arg ], "-formations", Max( 5, (int)strlen( argv[ arg ] ) ) ) == 0 )
      {
         if ( arg + 1 >= argc || argv[ arg + 1 ][ 0 ] == '-' )
         {
            showUsage( argv[ 0 ], "Argument for '-formations' is missing" );
            return -1;
         }
         if ( !parseStrings( formationNames, argv[ ++arg ] ) )
         {
            showUsage( argv[ 0 ], "Illegal argument for '-formations'" );
            return -1;
         }
      }
      else if ( strncmp( argv[ arg ], "-surfaces", Max( 5, (int)strlen( argv[ arg ] ) ) ) == 0 )
      {
         if ( arg + 1 >= argc || argv[ arg + 1 ][ 0 ] == '-' )
         {
            showUsage( argv[ 0 ], "Argument for '-surfaces' is missing" );
            return -1;
         }
         if ( !parseStrings( formationSurfaceNames, argv[ ++arg ] ) )
         {
            showUsage( argv[ 0 ], "Illegal argument for '-formations'" );
            return -1;
         }
      }
      else if ( strncmp( argv[ arg ], "-topsurfaces", Max( 2, (int)strlen( argv[ arg ] ) ) ) == 0 )
      {
         if ( arg + 1 >= argc || argv[ arg + 1 ][ 0 ] == '-' )
         {
            showUsage( argv[ 0 ], "Argument for '-topsurfaces' is missing" );
            return -1;
         }
         if ( !parseStrings( topSurfaceFormationNames, argv[ ++arg ] ) )
         {
            showUsage( argv[ 0 ], "Illegal argument for '-topsurfaces'" );
            return -1;
         }
      }
      else if ( strncmp( argv[ arg ], "-bottomsurfaces", Max( 3, (int)strlen( argv[ arg ] ) ) ) == 0 )
      {
         if ( arg + 1 >= argc || argv[ arg + 1 ][ 0 ] == '-' )
         {
            showUsage( argv[ 0 ], "Argument for '-bottomsurfaces' is missing" );
            return -1;
         }
         if ( !parseStrings( bottomSurfaceFormationNames, argv[ ++arg ] ) )
         {
            showUsage( argv[ 0 ], "Illegal argument for '-bottomsurfaces'" );
            return -1;
         }
      }
      else if ( strncmp( argv[ arg ], "-history", Max( 3, (int)strlen( argv[ arg ] ) ) ) == 0 )
      {
         versusDepth = false;
      }
      else if ( strncmp( argv[ arg ], "-reverse", Max( 3, (int)strlen( argv[ arg ] ) ) ) == 0 )
      {
         reverseOutputOrder = true;
      }
      else if ( strncmp( argv[ arg ], "-basement", Max( 3, (int)strlen( argv[ arg ] ) ) ) == 0 )
      {
         basement = true;
      }
      else if ( strncmp( argv[ arg ], "-all-2D-properties", Max( 7, (int)strlen( argv[ arg ] ) ) ) == 0 )
      {
         all2Dproperties = true;
      }
      else if ( strncmp( argv[ arg ], "-all-3D-properties", Max( 7, (int)strlen( argv[ arg ] ) ) ) == 0 )
      {
         all3Dproperties = true;
      }
      else if ( strncmp( argv[ arg ], "-list-properties", Max( 7, (int)strlen( argv[ arg ] ) ) ) == 0 )
      {
         listProperties = true;
      }
      else if ( strncmp( argv[ arg ], "-list-snapshots", Max( 8, (int)strlen( argv[ arg ] ) ) ) == 0 )
      {
         listSnapshots = true;
      }
      else if ( strncmp( argv[ arg ], "-list-stratigraphy", Max( 8, (int)strlen( argv[ arg ] ) ) ) == 0 )
      {
         listStratigraphy = true;
      }
      else if ( strncmp( argv[ arg ], "-project", Max( 5, (int)strlen( argv[ arg ] ) ) ) == 0 )
      {
         if ( arg + 1 >= argc || argv[ arg + 1 ][ 0 ] == '-' )
         {
            showUsage( argv[ 0 ], "Argument for '-project' is missing" );
            return -1;
         }
         inputProjectFileName = argv[ ++arg ];
      }
      else if ( strncmp( argv[ arg ], "-debug", Max( 2, (int)strlen( argv[ arg ] ) ) ) == 0 )
      {
         debug = true;
      }
      else if ( strncmp( argv[ arg ], "-help", Max( 3, (int)strlen( argv[ arg ] ) ) ) == 0 )
      {
         showUsage( argv[ 0 ], " Standard usage." );
         return -1;
      }
      else if ( strncmp( argv[ arg ], "-?", Max( 2, (int)strlen( argv[ arg ] ) ) ) == 0 )
      {
         showUsage( argv[ 0 ], " Standard usage." );
         return -1;
      }
      else if ( strncmp( argv[ arg ], "-save", Max( 2, (int)strlen( argv[ arg ] ) ) ) == 0 )
      {
         if ( arg + 1 >= argc || argv[ arg + 1 ][ 0 ] == '-' )
         {
            showUsage( argv[ 0 ], "Argument for '-save' is missing" );
            return -1;
         }
         outputFileName = argv[ ++arg ];
      }
      else if ( strncmp( argv[ arg ], "-usage", Max( 2, (int)strlen( argv[ arg ] ) ) ) == 0 )
      {
         showUsage( argv[ 0 ], " Standard usage." );
         return -1;
      }
      else if ( strncmp( argv[ arg ], "-verbose", Max( 2, (int)strlen( argv[ arg ] ) ) ) == 0 )
      {
         verbose = true;
      }
      else if ( strncmp( argv[ arg ], "-", 1 ) != 0 )
      {
         inputProjectFileName = argv[ arg ];
      }
      else
      {
         cerr << endl << "Unknown or ambiguous option: " << argv[ arg ] << endl;
         showUsage( argv[ 0 ] );
         return -1;
      }
   }

   if ( outputFileName != "" )
   {
      outputFile.open( outputFileName.c_str() );
      if ( outputFile.fail() )
      {
         showUsage( argv[ 0 ], "Could not open specified output file" );
         return -1;
      }
      else
      {
         p_outputStream = &outputFile;
      }
   }

   ostream & outputStream = *p_outputStream;

   if ( inputProjectFileName == "" )
   {
      showUsage( argv[ 0 ], "No project file specified" );
      return -1;
   }


   GeoPhysics::ObjectFactory* factory = new GeoPhysics::ObjectFactory;
   GeoPhysics::ProjectHandle* projectHandle = ( GeoPhysics::ProjectHandle* )( OpenCauldronProject( inputProjectFileName, "r", factory ) );
   DerivedPropertyManager propertyManager ( projectHandle );

   if ( !projectHandle )
   {
      showUsage( argv[ 0 ], "Could not open specified project file" );
	  delete factory;
      return -1;
   }

   const Property * depthProperty = projectHandle->findProperty( "Depth" );
   const Snapshot * zeroSnapshot = projectHandle->findSnapshot( 0 );

   if ( listProperties )
   {
      listOutputableProperties ( projectHandle, propertyManager );
      }

   if ( all2Dproperties )
      {
      PropertyList * allProperties = projectHandle->getProperties( true );

      for ( size_t i = 0; i < allProperties->size (); ++i ) {
         const Interface::Property* property = (*allProperties)[ i ];

         bool addIt = false;

         if (( property->getPropertyAttribute () == DataModel::CONTINUOUS_3D_PROPERTY or
               property->getPropertyAttribute () == DataModel::SURFACE_2D_PROPERTY ) and 
             propertyManager.surfacePropertyIsComputable ( property )) {
            addIt = true;
         } else if ( property->getPropertyAttribute () == DataModel::DISCONTINUOUS_3D_PROPERTY and 
                     propertyManager.formationSurfacePropertyIsComputable ( property )) {
            addIt = true;
         } else if ( property->getPropertyAttribute () == DataModel::FORMATION_2D_PROPERTY and 
                     propertyManager.formationMapPropertyIsComputable ( property )) {
            addIt = true;
      }

         if ( addIt ) {
            propertyNames.push_back( property->getName() );
   }

      }

      delete allProperties;
   }

   if ( all3Dproperties )
   {
      PropertyList * allProperties = projectHandle->getProperties( true );

      for ( size_t i = 0; i < allProperties->size (); ++i ) {
         const Interface::Property* property = (*allProperties)[ i ];

         if (( property->getPropertyAttribute () == DataModel::CONTINUOUS_3D_PROPERTY or
               property->getPropertyAttribute () == DataModel::DISCONTINUOUS_3D_PROPERTY ) and 
             propertyManager.formationPropertyIsComputable ( property ))
      {
            propertyNames.push_back( property->getName() );
         }

      }

   }

   if ( listSnapshots )
   {
      cout << endl;
      SnapshotList * mySnapshots = projectHandle->getSnapshots();
      SnapshotList::iterator snapshotIter;
      cout.precision ( 8 );
      cout << "Available snapshots are: ";
      for ( snapshotIter = mySnapshots->begin(); snapshotIter != mySnapshots->end(); ++snapshotIter )
      {
         if ( snapshotIter != mySnapshots->begin() ) cout << ",";
         int oldPrecision = cout.precision();
         cout << setprecision(9);
         cout << ( *snapshotIter )->getTime();
         cout <<setprecision(oldPrecision);
      }
      cout << endl;
   }

   if ( listStratigraphy )
   {
      cout << endl;
      FormationList * myFormations = projectHandle->getFormations( zeroSnapshot, true );
      FormationList::iterator formationIter;
      cout << "Stratigraphy: ";
      cout << endl;
      cout << endl;
      bool firstFormation = true;
      bool arrivedAtBasement = false;
      for ( formationIter = myFormations->begin(); formationIter != myFormations->end(); ++formationIter )
      {
         const Formation * formation = *formationIter;
         if ( formation->kind() == BASEMENT_FORMATION && arrivedAtBasement == false )
         {
            arrivedAtBasement = true;
            cout << "Next formations and surfaces belong to the basement and only produce output when used with '-basement'" << endl;
         }
         if ( firstFormation )
         {
            cout << "\t" << formation->getTopSurface()->getName() << " (" << formation->getTopSurface()->getSnapshot()->getTime() << " Ma)" << endl;
            firstFormation = false;
         }

         cout << "\t\t" << formation->getName() << endl;
         const Surface * bottomSurface = formation->getBottomSurface();
         if ( bottomSurface )
         {
            cout << "\t" << bottomSurface->getName();
            if ( bottomSurface->getSnapshot() )
            {
               cout << " (" << bottomSurface->getSnapshot()->getTime() << " Ma)";
            }
            cout << endl;
         }
      }
      cout << endl;
   }

   if ( listProperties || listSnapshots || listStratigraphy )
   {
      return 0;
   }

   const Interface::Grid * grid = projectHandle->getLowResolutionOutputGrid();

   projectHandle->startActivity ( "track1d", grid, false, false );
   bool coupledCalculation = false; // to do.
   bool started;

   started = projectHandle->initialise ( coupledCalculation );

   if ( not started ) {
      return 1;
   }

   started = projectHandle->setFormationLithologies ( false, true );

   if ( not started ) {
      return 1;
   }

   started = projectHandle->initialiseLayerThicknessHistory ( coupledCalculation );

   if ( not started ) {
      return 1;
   }

   acquireSnapshots( projectHandle, snapshots, ages );
   acquireProperties( projectHandle, propertyManager, properties, propertyNames );
   acquireFormationSurfaces( projectHandle, formationSurfacePairs, topSurfaceFormationNames, true );
   acquireFormationSurfaces( projectHandle, formationSurfacePairs, bottomSurfaceFormationNames, false );
   acquireFormations( projectHandle, formationSurfacePairs, formationNames );
   acquireSurfaces( projectHandle, formationSurfacePairs, formationSurfaceNames );

   if ( formationSurfacePairs.empty() )
   {
      FormationList * formations = projectHandle->getFormations( zeroSnapshot, basement );
      FormationList::iterator formationIter;
      for ( formationIter = formations->begin(); formationIter != formations->end(); ++formationIter )
      {
         const Formation * formation = *formationIter;
         formationSurfacePairs.push_back( FormationSurface( formation, SubSurface( static_cast<const Surface *>(0), -1 ) ) );
      }
   }

   DoublePairVector::iterator coordinatePairIter;
   SnapshotList::iterator snapshotIter;
   PropertyList::iterator propertyIter;
   FormationList::iterator formationIter;
   FormationSurfaceVector::iterator formationSurfaceIter;


   SnapshotFormationOutputPropertyValueMap allOutputPropertyValues;

   snapshots.push_back( zeroSnapshot ); // we require depth properties for snapshot age 0

   for ( snapshotIter = snapshots.begin(); snapshotIter != snapshots.end(); ++snapshotIter )
   {
      const Snapshot * snapshot = *snapshotIter;

      for ( formationSurfaceIter = formationSurfacePairs.begin(); formationSurfaceIter != formationSurfacePairs.end(); ++formationSurfaceIter )
      {
         const Formation * formation = ( *formationSurfaceIter ).first;

         for ( propertyIter = properties.begin(); propertyIter != properties.end(); ++propertyIter )
         {

            const Property * property = *propertyIter;
            OutputPropertyValuePtr outputProperty = allocateOutputProperty ( propertyManager, property, snapshot, *formationSurfaceIter );

            if ( outputProperty != 0 ) {
               allOutputPropertyValues [ snapshot ][ formation ][ property ] = outputProperty;
            }

         }
      }
   }

   snapshots.pop_back(); // to remove the explicitly added snapshot age 0

   FormationMaxKMap formationMaxKMap;
   for ( formationSurfaceIter = formationSurfacePairs.begin();
      formationSurfaceIter != formationSurfacePairs.end(); ++formationSurfaceIter )
   {
      const Formation * formation = ( *formationSurfaceIter ).first;

      OutputPropertyValuePtr depthOutputProperty = allOutputPropertyValues[ zeroSnapshot ][ formation ][ depthProperty ];

      if ( depthOutputProperty == 0 )
      {
         cerr << "ERROR: Could not find data for depth property of formation " << formation->getName() << " at age " << zeroSnapshot->getTime() << endl;
         cerr << "       Skipping this formation!!" << endl;
         formationMaxKMap[ formation ] = 0;
      }
      else
      {
         formationMaxKMap[ formation ] = depthOutputProperty->getDepth();
      }
   }

   outputStream << "X(m),Y(m),I,J,Age(Ma),Formation,Surface,LayerIndex";

   for ( propertyIter = properties.begin(); propertyIter != properties.end(); ++propertyIter )
   {
      outputStream << "," << ( *propertyIter )->getName() << "(" << ( *propertyIter )->getUnit() << ")";
   }
   outputStream << endl;

   // convert i,j pairs into x,y pairs
   for ( coordinatePairIter = logicalCoordinatePairs.begin(); coordinatePairIter != logicalCoordinatePairs.end(); ++coordinatePairIter )
   {
      DoublePair & coordinatePair = *coordinatePairIter;

      double x, y;
      if ( !grid->getPosition( coordinatePair.first, coordinatePair.second, x, y ) )
      {
         cerr << "illegal (i,j) coordinate pair: (" << coordinatePair.first << ", " << coordinatePair.second << ")" << endl;
         continue;
      }

      realWorldCoordinatePairs.push_back( DoublePair( x, y ) );
   }

   for ( coordinatePairIter = realWorldCoordinatePairs.begin(); coordinatePairIter != realWorldCoordinatePairs.end(); ++coordinatePairIter )
   {
      DoublePair & coordinatePair = *coordinatePairIter;

      double i, j;
      if ( !grid->getGridPoint( coordinatePair.first, coordinatePair.second, i, j ) )
      {
         cerr << "illegal coordinate pair: (" << coordinatePair.first << ", " << coordinatePair.second << ")" << endl;
         continue;
      }

      if ( versusDepth )
      {

         for ( snapshotIter = snapshots.begin(); snapshotIter != snapshots.end(); ++snapshotIter )
         {

            const Snapshot * snapshot = *snapshotIter;

            for ( formationSurfaceIter = formationSurfacePairs.begin(); formationSurfaceIter != formationSurfacePairs.end(); ++formationSurfaceIter )
            {
               const Formation * formation = ( *formationSurfaceIter ).first;

               if ( allOutputPropertyValues[ snapshot ][ formation ][ properties[ 0 ]] == 0 ) continue;

               unsigned int maxK = formationMaxKMap[ formation ];
               unsigned int k;

               for ( k = 0; k < maxK; ++k )
               {
                  int kUsed = reverseOutputOrder ? maxK - 1 - k : k;
                  outputSnapshotFormationData( outputStream, coordinatePair, snapshot, ( *formationSurfaceIter ), properties, allOutputPropertyValues, formationSurfacePairs, i, j, kUsed, maxK );
               }

            }

         }

      }
      else
      {
         for ( formationSurfaceIter = formationSurfacePairs.begin();
            formationSurfaceIter != formationSurfacePairs.end(); ++formationSurfaceIter )
         {
            const Formation * formation = ( *formationSurfaceIter ).first;

            unsigned int maxK = formationMaxKMap[ formation ];
            unsigned int k;
            for ( k = 0; k < maxK; ++k )
            {
               int kUsed = reverseOutputOrder ? maxK - 1 - k : k;
               for ( snapshotIter = snapshots.begin(); snapshotIter != snapshots.end(); ++snapshotIter )
               {
                  const Snapshot * snapshot = *snapshotIter;

                  if ( allOutputPropertyValues[ snapshot ][ formation ][ properties[ 0 ]] == 0 ) continue;

                  outputSnapshotFormationData( outputStream, coordinatePair, snapshot, ( *formationSurfaceIter ), properties, allOutputPropertyValues, formationSurfacePairs, i, j, kUsed, maxK );
               }
               }
            }
         }
      }

   if ( projectHandle != 0 ) {
      projectHandle->finishActivity ( false );
   }

   if ( outputFile.is_open() )
   {
      outputFile.close();
   }

   DataAccess::Interface::CloseCauldronProject( projectHandle );
   delete factory;

   return 0;
}


OutputPropertyValuePtr allocateOutputProperty ( DerivedProperties::AbstractPropertyManager& propertyManager, 
                                                const DataModel::AbstractProperty* property, 
                                                const DataModel::AbstractSnapshot* snapshot,
                                                const FormationSurface& formationItem ) {

   OutputPropertyValuePtr outputProperty;

   const Interface::Formation* formation = formationItem.first;
   const Interface::Surface* topSurface = 0;
   const Interface::Surface* bottomSurface = 0;

   if (( property->getPropertyAttribute () == DataModel::CONTINUOUS_3D_PROPERTY or
         property->getPropertyAttribute () == DataModel::DISCONTINUOUS_3D_PROPERTY ) and 
       propertyManager.formationPropertyIsComputable ( property, snapshot, formation ))
   {
      outputProperty = OutputPropertyValuePtr ( new FormationOutputPropertyValue ( propertyManager, property, snapshot, formation ));
   }

   if ( outputProperty == 0 ) {


      if ( formation != 0 and formationItem.second.first != 0  ) {

         if ( formation->getTopSurface () != 0 and formationItem.second.first == formation->getTopSurface ()) {
            topSurface = formation->getTopSurface ();
         } else if ( formation->getBottomSurface () != 0 and formationItem.second.first == formation->getBottomSurface ()) {
            bottomSurface = formation->getBottomSurface ();
         }

      }

      // First check if the surface property is computable
      if (( property->getPropertyAttribute () == DataModel::CONTINUOUS_3D_PROPERTY or
            property->getPropertyAttribute () == DataModel::SURFACE_2D_PROPERTY ) and 
          (( topSurface != 0    and propertyManager.surfacePropertyIsComputable ( property, snapshot, topSurface )) or
           ( bottomSurface != 0 and propertyManager.surfacePropertyIsComputable ( property, snapshot, bottomSurface ))))
      {

         if ( topSurface != 0 ) {
            outputProperty = OutputPropertyValuePtr ( new SurfaceOutputPropertyValue ( propertyManager, property, snapshot, topSurface ));
         } else if ( bottomSurface != 0 ) {
            outputProperty = OutputPropertyValuePtr ( new SurfaceOutputPropertyValue ( propertyManager, property, snapshot, bottomSurface ));
         }

      }
      // Next check if the formation-surface property is computable
      else if ( property->getPropertyAttribute () == DataModel::DISCONTINUOUS_3D_PROPERTY and 
                (( topSurface != 0    and propertyManager.formationSurfacePropertyIsComputable ( property, snapshot, formation, topSurface )) or
                 ( bottomSurface != 0 and propertyManager.formationSurfacePropertyIsComputable ( property, snapshot, formation, bottomSurface ))))

      {

         if ( topSurface != 0 ) {
            outputProperty = OutputPropertyValuePtr ( new FormationSurfaceOutputPropertyValue ( propertyManager, property, snapshot, formation, topSurface ));
         } else if ( bottomSurface != 0 ) {
            outputProperty = OutputPropertyValuePtr ( new FormationSurfaceOutputPropertyValue ( propertyManager, property, snapshot, formation, bottomSurface ));
         }

      }
      // Finally check if the formation-map property is computable
      else if ( property->getPropertyAttribute () == DataModel::FORMATION_2D_PROPERTY and 
                propertyManager.formationMapPropertyIsComputable ( property, snapshot, formation ))
      {
         outputProperty = OutputPropertyValuePtr ( new FormationMapOutputPropertyValue ( propertyManager, property, snapshot, formation ));
      }

   }

   return outputProperty;
}

void outputSnapshotFormationData( ostream & outputStream, DoublePair & coordinatePair,
   const Snapshot * snapshot, FormationSurface  & formationSurface, PropertyList & properties,
                                  SnapshotFormationOutputPropertyValueMap & allOutputPropertyValues,
   FormationSurfaceVector & formationSurfacePairs, double i, double j, unsigned int k, unsigned int maxK )
{
   int kInverse = ( maxK - 1 ) - k;

   const Formation * formation = formationSurface.first;
   string formationSurfaceName;

   if ( k == 0 )
   {

      if ( specifiesFormationSurface( formationSurface, formation, formation->getTopSurface(), kInverse ) )
      {
         formationSurfaceName = formation->getTopSurface()->getName();
      }
      else return;
   }
   else if ( k == maxK - 1 )
   {
      if ( specifiesFormationSurface( formationSurface, formation, formation->getBottomSurface(), kInverse ) )
      {
         formationSurfaceName = formation->getBottomSurface()->getName();
      }
      else return;
   }
   else
   {
      if ( specifiesFormationSurface( formationSurface, formation, 0, kInverse ) )
      {
         formationSurfaceName = "";
      }
      else return;
   }

   outputStream << coordinatePair.first << "," << coordinatePair.second << "," << i << "," << j << ",";
   int oldPrecision = outputStream.precision(); 
   outputStream << setprecision(9) << snapshot->getTime() << setprecision(oldPrecision);
   outputStream << "," << formation->getName();
   outputStream << "," << formationSurfaceName;

   outputStream << "," << kInverse;

   PropertyList::iterator propertyIter;

   for ( propertyIter = properties.begin(); propertyIter != properties.end(); ++propertyIter )
   {
      const Property * property = *propertyIter;
      OutputPropertyValuePtr propertyValue = allOutputPropertyValues[ snapshot ][ formation ][ property ];
      outputStream << ",";

      if ( propertyValue != 0 )
      {
         // const GridMap * gridMap = propertyValue->getGridMap();

         double kIndex = propertyValue->getDepth() > 1 ? (double)k : 0;
         double value = propertyValue->getValue( i, j, kIndex );

         if ( value != propertyValue->getUndefinedValue ()) //gridMap->getUndefinedValue() )
         {
            outputStream << value;
         }
         else
         {
            outputStream << " ";
      }

      }
      else
      {
         outputStream << " ";
      }
   }
   outputStream << endl;
}


/// destructive!!!
bool splitString( char * string, char separator, char * & firstPart, char * & secondPart )
{
   if ( !string || strlen( string ) == 0 ) return false;
   secondPart = strchr( string, separator );
   if ( secondPart != 0 )
   {
      *secondPart = '\0';
      ++secondPart;
      if ( *secondPart == '\0' ) secondPart = 0;
   }

   firstPart = string;
   if ( strlen( firstPart ) == 0 ) return false;
   return true;
}


bool parseCoordinates( DoublePairVector & coordinatePairs, char * coordinatesString )
{
   char * strPtr = coordinatesString;
   char * sectionX;
   char * sectionY;
   while ( splitString( strPtr, ',', sectionX, strPtr ) &&
      splitString( strPtr, ',', sectionY, strPtr ) )
   {
      double numberX, numberY;

      numberX = atof( sectionX );
      numberY = atof( sectionY );

      coordinatePairs.push_back( DoublePair( numberX, numberY ) );
   }
   return true;
}

bool parseStrings( StringVector & strings, char * stringsString )
{
   char * strPtr = stringsString;
   char * section;
   while ( splitString( strPtr, ',', section, strPtr ) )
   {
      strings.push_back( string( section ) );
   }
   return true;
}


bool parseAges( DoubleVector & ages, char * agesString )
{
   char * strPtr = agesString;
   char * commasection;
   char * section;
   while ( splitString( strPtr, ',', commasection, strPtr ) )
   {
      while ( splitString( commasection, '-', section, commasection ) )
      {
         double number = atof( section );
         ages.push_back( number );
      }
      ages.push_back( -1 ); // separator
   }
   ages.push_back( -1 ); // separator
   return true;
}

bool acquireSnapshots( ProjectHandle * projectHandle, SnapshotList & snapshots, DoubleVector & ages )
{
   if ( ages.size() == 0 )
   {
      SnapshotList * allSnapshots = projectHandle->getSnapshots();
      snapshots = *allSnapshots;
      return true;
   }
   else
   {
      int index;
      double firstAge = -1;
      double secondAge = -1;
      for ( index = 0; index < ages.size(); ++index )
      {
         if ( ages[ index ] >= 0 )
         {
            if ( firstAge < 0 )
               firstAge = ages[ index ];
            else
               secondAge = ages[ index ];
         }
         else
         {
            if ( secondAge < 0 )
            {
               if ( firstAge >= 0 )
               {
                  const Snapshot * snapshot = projectHandle->findSnapshot( firstAge );
                  if ( snapshot ) snapshots.push_back( snapshot );
                  if ( debug && snapshot ) cerr << "adding single snapshot " << snapshot->getTime() << endl;
               }
            }
            else
            {
               if ( firstAge >= 0 )
               {
                  if ( firstAge > secondAge )
                  {
                     Swap( firstAge, secondAge );
                  }

                  SnapshotList * allSnapshots = projectHandle->getSnapshots();
                  SnapshotList::iterator snapshotIter;
                  for ( snapshotIter = allSnapshots->begin(); snapshotIter != allSnapshots->end(); ++snapshotIter )
                  {
                     const Snapshot * snapshot = *snapshotIter;
                     if ( snapshot->getTime() >= firstAge && snapshot->getTime() <= secondAge )
                     {
                        if ( snapshot ) snapshots.push_back( snapshot );
                        if ( debug && snapshot ) cerr << "adding range snapshot " << snapshot->getTime() << endl;
                     }
                  }
               }
            }
            firstAge = secondAge = -1;
         }
      }
   }
   sort( snapshots.begin(), snapshots.end(), snapshotSorter );

   if ( debug )
   {
      cerr << "Snapshots ordered" << endl;
      SnapshotList::iterator snapshotIter;
      for ( snapshotIter = snapshots.begin(); snapshotIter != snapshots.end(); ++snapshotIter )
      {
         cerr << ( *snapshotIter )->getTime() << endl;
      }
   }

   SnapshotList::iterator firstObsolete = unique( snapshots.begin(), snapshots.end(), snapshotIsEqual );
   snapshots.erase( firstObsolete, snapshots.end() );

   if ( debug )
   {
      cerr << "Snapshots uniquefied" << endl;
      SnapshotList::iterator snapshotIter;
      for ( snapshotIter = snapshots.begin(); snapshotIter != snapshots.end(); ++snapshotIter )
      {
         cerr << ( *snapshotIter )->getTime() << endl;
      }
   }

   return true;
}

bool acquireProperties( ProjectHandle * projectHandle,
                        const AbstractPropertyManager& propertyManager,
                        PropertyList & properties,
                        StringVector & propertyNames )
{
   const Property * depthProperty = projectHandle->findProperty( "Depth" );
   assert( depthProperty );
   properties.push_back( depthProperty );

   StringVector::iterator stringIter;

   for ( stringIter = propertyNames.begin(); stringIter != propertyNames.end(); ++stringIter )
   {

      const Property * property = projectHandle->findProperty( *stringIter );
      bool isComputable = false;

      if ( property == 0 )
      {
         cerr << "Could not find property named '" << *stringIter << "'" << endl;
         continue;
      }

      if (( property->getPropertyAttribute () == DataModel::CONTINUOUS_3D_PROPERTY or
            property->getPropertyAttribute () == DataModel::DISCONTINUOUS_3D_PROPERTY ) and 
          propertyManager.formationPropertyIsComputable ( property ))
      {
         isComputable = true;
      }
      else if (( property->getPropertyAttribute () == DataModel::CONTINUOUS_3D_PROPERTY or
                 property->getPropertyAttribute () == DataModel::SURFACE_2D_PROPERTY ) and 
               propertyManager.surfacePropertyIsComputable ( property ))
      {
         isComputable = true;
      }
      else if ( property->getPropertyAttribute () == DataModel::DISCONTINUOUS_3D_PROPERTY and 
                propertyManager.formationSurfacePropertyIsComputable ( property ))
      {
         isComputable = true;
      }
      else if ( property->getPropertyAttribute () == DataModel::FORMATION_2D_PROPERTY and 
                propertyManager.formationMapPropertyIsComputable ( property ))
      {
         isComputable = true;
      }

      if ( isComputable ) {
      properties.push_back( property );
      } else {
         cerr << "Could not find calculator for property named '" << *stringIter << "'" << endl;
      }

   }

   return true;
}

bool acquireFormationSurfaces( ProjectHandle * projectHandle, FormationSurfaceVector & formationSurfacePairs, StringVector & formationNames, bool useTop )
{
   if ( formationNames.size() != 0 )
   {
      StringVector::iterator stringIter;
      for ( stringIter = formationNames.begin(); stringIter != formationNames.end(); ++stringIter )
      {
         const Formation * formation = projectHandle->findFormation( *stringIter );
         if ( !formation )
         {
            cerr << "Could not find formation named '" << *stringIter << "'" << endl;
            continue;
         }

         const Surface * surface = useTop ? formation->getTopSurface() : formation->getBottomSurface();
         if ( !surface )
         {
            cerr << "Could not find " << ( useTop ? "Top " : "Bottom" ) << " surface for formation named '" << *stringIter << "'" << endl;
            continue;
         }
         formationSurfacePairs.push_back( FormationSurface( formation, SubSurface( surface, -1 ) ) );
      }
   }

   return true;
}

bool acquireFormations( ProjectHandle * projectHandle, FormationSurfaceVector & formationSurfacePairs, StringVector & formationNames )
{
   if ( formationNames.size() != 0 )
   {
      StringVector::iterator stringIter;
      for ( stringIter = formationNames.begin(); stringIter != formationNames.end(); ++stringIter )
      {
         const Formation * formation = projectHandle->findFormation( *stringIter );
         if ( !formation )
         {
            cerr << "Could not find formation named '" << *stringIter << "'" << endl;
            continue;
         }

         formationSurfacePairs.push_back( FormationSurface( formation, SubSurface( static_cast<const Surface *>(0), -1 ) ) );
      }
   }

   return true;
}

bool acquireSurfaces( ProjectHandle * projectHandle, FormationSurfaceVector & formationSurfacePairs, StringVector & formationSurfaceNames )
{
   if ( formationSurfaceNames.size() != 0 )
   {
      StringVector::iterator stringIter;
      for ( stringIter = formationSurfaceNames.begin(); stringIter != formationSurfaceNames.end(); ++stringIter )
      {
         string surfaceName = *stringIter;
         char * formationName;
         char * formationSurfaceName;
         char surfaceNamePtr[ 256 ];
         strcpy( surfaceNamePtr, surfaceName.c_str() );

         if ( splitString( surfaceNamePtr, ':', formationName, formationSurfaceName ) == false ) continue;
         if ( formationName == 0 || strlen( formationName ) == 0 ) continue;
         if ( formationSurfaceName == 0 || strlen( formationSurfaceName ) == 0 ) continue;

         const Formation * formation = projectHandle->findFormation( formationName );
         if ( !formation )
         {
            cerr << "Could not find formation named '" << *stringIter << "'" << endl;
            continue;
         }

         if ( formationSurfaceName[ 0 ] < '0' || formationSurfaceName[ 0 ] > '9' )
         {
            const Surface * surface = projectHandle->findSurface( formationSurfaceName );
            if ( !surface )
            {
               cerr << "Could not find surface named '" << formationSurfaceName << "'" << endl;
               continue;
            }
            formationSurfacePairs.push_back( FormationSurface( formation, SubSurface( surface, -1 ) ) );
         }
         else
         {
            formationSurfacePairs.push_back( FormationSurface( formation, SubSurface( static_cast<const Surface *>(0), atoi( formationSurfaceName ) ) ) );
         }
      }
   }

   return true;
}

#if 0
bool containsFormation( FormationSurfaceVector & formationSurfacePairs, const Formation * formation )
{
   if ( formationSurfacePairs.empty() ) return true;
   FormationSurfaceVector::iterator fsIterator;
   for ( fsIterator = formationSurfacePairs.begin(); fsIterator != formationSurfacePairs.end(); ++fsIterator )
   {
      FormationSurface & formationSurface = *fsIterator;
      if ( formationSurface.first == formation )
         return true;
   }
   return false;
}

bool containsSurface( FormationSurfaceVector & formationSurfacePairs, const Surface * surface )
{
   if ( formationSurfacePairs.empty() ) return true;
   FormationSurfaceVector::iterator fsIterator;
   for ( fsIterator = formationSurfacePairs.begin(); fsIterator != formationSurfacePairs.end(); ++fsIterator )
   {
      FormationSurface & formationSurface = *fsIterator;
      if ( formationSurface.second.first == surface )
         return true;
   }
   return false;
}

bool containsFormationSurface( FormationSurfaceVector & formationSurfacePairs, const Formation * formation, const Surface * surface, int index )
{
   if ( formationSurfacePairs.empty() ) return true;
   FormationSurfaceVector::iterator fsIterator;
   for ( fsIterator = formationSurfacePairs.begin(); fsIterator != formationSurfacePairs.end(); ++fsIterator )
   {
      FormationSurface & formationSurface = *fsIterator;
      if ( formationSurface.first == formation &&
         ( ( formationSurface.second.second < 0 && formationSurface.second.first == surface ) ||
         ( formationSurface.second.first == 0 && formationSurface.second.second == index ) ) )
         return true;
   }
   return false;
}
#endif

static bool specifiesFormationSurface( FormationSurface & formationSurface, const Formation * formation, const Surface * surface, int subSurfaceIndex )
{
   return ( formationSurface.first == formation &&
      ( formationSurface.second.first != 0 && formationSurface.second.first == surface ) ||
      ( formationSurface.second.second >= 0 && formationSurface.second.second == subSurfaceIndex ) ||
      ( formationSurface.second.first == 0 && formationSurface.second.second < 0 ) );
}

bool snapshotIsEqual( const Snapshot * snapshot1, const Snapshot * snapshot2 )
{
   return snapshot1->getTime() == snapshot2->getTime();
}

bool snapshotSorter( const Snapshot * snapshot1, const Snapshot * snapshot2 )
{
   return snapshot1->getTime() > snapshot2->getTime();
}

void showUsage( const char* command, const char* message )
{

   cout << endl;

   if ( message != 0 )
   {
      cout << command << ": " << message << endl;
   }

   cout << "Usage (case sensitive!!): " << command << endl << endl
      << "\t[-coordinates x1,y1,x2,y2....]                     real-world coordinates to produce output for" << endl
      << "\t[-logical-coordinates i1,j1,i2,j2....]             logical coordinates to produce output for" << endl
      << "\t[-properties name1,name2...]                       properties to produce output for" << endl
      << "\t[-ages age1[-age2],...]                            select snapshot ages using single values and/or ranges" << endl << endl
      << "\t[-history]                                         produce output in a time-centric instead of a depth-centric fashion" << endl << endl
      << "\t[-reverse]                                         reverse the depth order (to bottom-up) in which output is produced" << endl << endl
      << "\t[-topsurfaces formation1,formation2...]            produce output for the surfaces at the top of the given formations" << endl
      << "\t[-bottomsurfaces formation1,formation2...]         produce output for the surfaces at the bottom of the given formations" << endl
      << "\t[-formations formation1,formation2...]             produce output for the given formations" << endl
      << "\t[-surfaces formation1:<index|surface>,...]         produce output for the given formation surfaces" << endl << endl
      << "\t                                                   the four options above can include Crust or Mantle" << endl << endl
      << "\t[-basement]                                        produce output for the basement as well," << endl
      << "\t                                                   only needed if none of the three options above have been specified" << endl << endl
      << "\t[-project] projectname                             name of 3D Cauldron project file to produce output for" << endl
      << "\t[-save filename]                                   name of file to save output (*.csv format) table to, otherwise save to stdout" << endl
      << endl
      << "\t[-all-3D-properties]                               produce output for all 3D properties" << endl
      << "\t[-all-2D-properties]                               produce output for all 2D properties" << endl
      << "\t[-list-properties]                                 print a list of available properties and exit" << endl
      << "\t[-list-snapshots]                                  print a list of available snapshots and exit" << endl
      << "\t[-list-stratigraphy]                               print a list of available surfaces and formations and exit" << endl << endl
      << "\t[-help]                                            print this message and exit" << endl << endl;
   cout << "If names in an argument list contain spaces, put the list between double or single quotes, e.g:"
      << "\t-formations \"Dissolved Salt,Al Khalata\"" << endl;
   cout << "Bracketed options are optional and options may be abbreviated" << endl << endl;

}

void listOutputableProperties ( const GeoPhysics::ProjectHandle* projectHandle,
                                const DerivedPropertyManager&    propertyManager ) {


   PropertyList * allProperties = projectHandle->getProperties ( true );

   cout << "Available 3D output properties are: ";

   for ( size_t i = 0; i < allProperties->size (); ++i ) {
      const Interface::Property* property = (*allProperties)[ i ];

      if (( property->getPropertyAttribute () == DataModel::CONTINUOUS_3D_PROPERTY or
            property->getPropertyAttribute () == DataModel::DISCONTINUOUS_3D_PROPERTY ) and 
          propertyManager.formationPropertyIsComputable ( property )) {
         cout << property->getName () << "  ";            
      }

   }

   cout << endl;
   cout << endl;
   cout << "Available 2D output properties are: ";

   for ( size_t i = 0; i < allProperties->size (); ++i ) {
      const Interface::Property* property = (*allProperties)[ i ];

      if (( property->getPropertyAttribute () == DataModel::CONTINUOUS_3D_PROPERTY or
            property->getPropertyAttribute () == DataModel::SURFACE_2D_PROPERTY ) and 
          propertyManager.surfacePropertyIsComputable ( property )) {
         cout << property->getName () << "  ";            
      } else if ( property->getPropertyAttribute () == DataModel::DISCONTINUOUS_3D_PROPERTY and 
                  propertyManager.formationSurfacePropertyIsComputable ( property )) {
         cout << property->getName () << "  ";            
      } else if ( property->getPropertyAttribute () == DataModel::FORMATION_2D_PROPERTY and 
                  propertyManager.formationMapPropertyIsComputable ( property )) {
         cout << property->getName () << "  ";            
      }

   }



   cout << endl;
   cout << endl;
   delete allProperties;
}




