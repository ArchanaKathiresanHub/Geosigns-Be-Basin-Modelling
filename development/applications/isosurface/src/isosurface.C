#include <stdlib.h>
#include <unistd.h>
#include <math.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <assert.h>

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <map>
using namespace std;


// DataAccess API includes
#include "Interface/GridMap.h"
#include "Interface/Grid.h"
#include "Interface/ProjectHandle.h"
#include "Interface/Snapshot.h"
#include "Interface/Surface.h"
#include "Interface/Formation.h"
#include "Interface/Reservoir.h"
#include "Interface/LithoType.h"
#include "Interface/Property.h"
#include "Interface/PropertyValue.h"

#include "NumericFunctions.h"

#include <string>
#include <vector>
#include "array.h"

using namespace std;
using namespace ibs;

using namespace DataAccess;
using namespace Interface;

const double CauldronUndefined = 99999;

double UndefinedValue = CauldronUndefined;



const std::string XYZMapwriterName = "xyz";
const std::string ZycorMapwriterName = "zycor";

const std::string DefaultMapType = ZycorMapwriterName;

const std::string DefaultIsoSurfacePropertyName = "Temperature";

#define Max(a,b) ((a) > (b) ? (a) : (b))

class GridDescription {

public :

   GridDescription ( const unsigned int numberX,
                     const unsigned int numberY,
                     const double       originX,
                     const double       originY,
                     const double       deltaX,
                     const double       deltaY );

   GridDescription ( const Grid* grid );


   unsigned int getNumberInX () const;

   unsigned int getNumberInY () const;

   double getOriginX () const;

   double getOriginY () const;

   double getDeltaX () const;

   double getDeltaY () const;

   double getLastX () const;

   double getLastY () const;


private :

   unsigned int m_numberX;
   unsigned int m_numberY;

   double m_deltaX;
   double m_deltaY;

   double m_originX;
   double m_originY;

};


class IsoMapWriter {

public :

   virtual ~IsoMapWriter () {}

   virtual bool write ( const Grid*    grid,
                        double** data,
                        const std::string& fileName ) = 0;

};


/// Functor class for writing data in the XYZ format.
class XYZMapWriter : public IsoMapWriter {

public :

   bool write ( const Grid*    grid,
                      double** data,
                const std::string& fileName );

};



/// Functor class for writing data in the Zycor format.
class ZycorMapWriter : public IsoMapWriter {

public :

   bool write ( const Grid*    grid,
                      double** data,
                const std::string& fileName );

private :

   static const unsigned int NumberOfColumns;

   static const unsigned int NumberWidth;


   void writeHeader ( ofstream& mapFile,
                      const Grid*     grid );

};


IsoMapWriter* allocateXYZMapWriter ();
IsoMapWriter* allocateZycorMapWriter ();


/// Factory class for allocating a map-writer functor.
class MapWriterFactory {

   typedef IsoMapWriter* (*MapWriterAllocator)();

public :

   static MapWriterFactory& getInstance ();

   IsoMapWriter* allocateMapWriter ( const std::string& mapType ) const;

   bool isValidMapType ( const std::string& mapType ) const;

private :

   static MapWriterFactory* s_mapWriterFactory;

   MapWriterFactory ();

   std::map<string, MapWriterAllocator> m_allocators;

};

/// Functor class for comparing the property-value->formation with the internal-formation.
class PropertyValueFormationComparator {

public :

   PropertyValueFormationComparator ();

   void setFormation ( const Formation* newFormation );

   bool operator ()( const PropertyValue* prop ) const;


private :

   const Formation* m_formation;

};


static char * argv0 = 0;
static void showUsage (const char * message = 0);

// Upper-Value here means the value at the upper-node, this may not necessarily mean the largest value.
// Lower-value is defined simimarly for the lower-node.
double computeIsoparametricFraction ( const double value, 
                                      const double upperValue,
                                      const double lowerValue );

double computeIsoparametricValue    ( const double fraction,
                                      const double upperDepth,
                                      const double lowerDepth );


bool verbose = false;
bool debug = false;

int main (int argc, char ** argv)
{
   string projectFileName;
   string outputFileName;
   string isoSurfacePropertyName;
   string outputMapType;
   double isoSurfacePropertyValue = CauldronUndefined;
   bool includeBasement = true;
   bool listProperties = false;
   bool listSnapshots = false;


   if ((argv0 = strrchr (argv[0], '/')) != 0)
   {
      ++argv0;
   }
   else
   {
      argv0 = argv[0];
   }

   double snapshotTime = 0;
   int arg;
   for (arg = 1; arg < argc; arg++)
   {
      if (strncmp (argv[arg], "-output", Max (2, strlen (argv[arg]))) == 0)
      {
         if (arg + 1 >= argc)
         {
            showUsage ("Argument for '-output' is missing");
            return 1;
         }
         outputFileName = argv[++arg];
      }
      else if (strncmp (argv[arg], "-snapshot", Max (2, strlen (argv[arg]))) == 0)
      {
         if (arg + 1 >= argc)
         {
            showUsage ("Argument for '-snapshot' is missing");
            return 1;
         }
         snapshotTime = atof (argv[++arg]);
      }
      else if (strncmp (argv[arg], "-project", Max (2, strlen (argv[arg]))) == 0)
      {
         if (arg + 1 >= argc)
         {
            showUsage ("Argument for '-project' is missing");
            return 1;
         }
         projectFileName = argv[++arg];
      }
      else if (strncmp (argv[arg], "-maptype", Max (2, strlen (argv[arg]))) == 0)
      {
         if (arg + 1 >= argc)
         {
            showUsage ("Argument for '-maptype' is missing");
            return 1;
         }
         outputMapType = argv[++arg];
      }
      else if (strncmp (argv[arg], "-isp", Max (2, strlen (argv[arg]))) == 0)
      {
         if (arg + 1 >= argc)
         {
            showUsage ("Argument for '-isp' is missing");
            return 1;
         }
         isoSurfacePropertyName = argv[++arg];
      }
      else if (strncmp (argv[arg], "-ispval", Max (2, strlen (argv[arg]))) == 0)
      {
         if (arg + 1 >= argc)
         {
            showUsage ("Argument for '-ispval' is missing");
            return 1;
         }
         isoSurfacePropertyValue = atof(argv[++arg]);
      }
      else if (strncmp (argv[arg], "-nullval", Max (2, strlen (argv[arg]))) == 0)
      {
         if (arg + 1 >= argc)
         {
            showUsage ("Argument for '-nullval' is missing");
            return 1;
         }
         UndefinedValue = atof(argv[++arg]);
      }
      else if (strncmp (argv[arg], "-debug", Max (2, strlen (argv[arg]))) == 0)
      {
         debug = true;
      }
      else if (strncmp (argv[arg], "-verbose", Max (2, strlen (argv[arg]))) == 0)
      {
         verbose = true;
      }
      else if (strncmp (argv[arg], "-nobasement", Max (2, strlen (argv[arg]))) == 0)
      {
         includeBasement = false;
      }
      else if (strncmp (argv[arg], "-listprops", Max (2, strlen (argv[arg]))) == 0)
      {
         listProperties = true;
      }
      else if (strncmp (argv[arg], "-listsnap", Max (2, strlen (argv[arg]))) == 0)
      {
         listSnapshots = true;
      }
      else
      {
         showUsage ();
         return 1;
      }
   }

   if ( outputMapType == "" ) {
      outputMapType = DefaultMapType;
   }

   if ( isoSurfacePropertyName == "" ) {
      isoSurfacePropertyName = DefaultIsoSurfacePropertyName;
   }


   if (projectFileName == "")
   {
      showUsage ("No project file specified");
      return 1;
   }

   ProjectHandle *projectHandle = OpenCauldronProject (projectFileName, "r");

   if (projectFileName.length () == 0)
   {
      cerr << "Could not open project file " << projectFileName << endl;
      showUsage ();
      return 1;
   }

   if ( projectHandle == 0 ) {
      // Error message comes from DAL.
      return 1;
   }

   const Snapshot *snapshot = projectHandle->findSnapshot (snapshotTime);

   if (!snapshot)
   {
      cerr << "No calculations have been made for snapshot time " << snapshotTime << endl;
      return 1;
   }

   bool outputAndQuit = false;

   if ( listProperties ) {

      PropertyList* allProperties = projectHandle->getProperties ( false, FORMATION, snapshot, 0, 0, 0, VOLUME );
      PropertyList::const_iterator propIter;

      cout << endl << "Available properties are:" << endl << endl;
      cout << setw ( 37 ) << "Name"  << setw ( 8 ) << "Unit" << endl;
      cout << setw ( 37 ) << "----"  << setw ( 8 ) << "----" << endl;

      for ( propIter = allProperties->begin (); propIter != allProperties->end (); ++propIter ) {
         cout << setw ( 12 ) << "Property:" << setw ( 25 ) << (*propIter)->getName () << setw ( 8 ) << (*propIter)->getUnit () << endl;
      }

      cout << endl;

      delete allProperties;

      outputAndQuit = true;
      // return 0;
   }

   if ( listSnapshots ) {

      SnapshotList* allSnapshots = projectHandle->getSnapshots ();
      SnapshotList::const_iterator ssIter;

      cout << endl << "Available snapshot ages are:" << endl << endl;
      cout << setw ( 27 ) << "Age"  << endl;
      cout << setw ( 27 ) << "---"  << endl;

      for ( ssIter = allSnapshots->begin (); ssIter != allSnapshots->end (); ++ssIter ) {
         cout << setw ( 12 ) << "Snapshot:" << setw ( 15 ) << (*ssIter)->getTime () << endl;
      }

      cout << endl;

      delete allSnapshots;
      outputAndQuit = true;

      // return 0;
   }

   if ( outputAndQuit ) {
      return 0;
   }

   if (outputFileName.length () == 0)
   {
      showUsage ("No output file specified");
      return 1;
   }

   if (verbose)
   {
      cout << "Using snapshot " << setprecision (10) << snapshot->getTime () << endl;
   }

   if ( isoSurfacePropertyValue == CauldronUndefined ) {
      cerr << " No iso-surface value for property '" << isoSurfacePropertyName << "' specified." << endl;
      showUsage ("Argument for '-ispval' is missing");
      return 1;
   }

   const Property *depthProperty = projectHandle->findProperty ("Depth");

   if (!depthProperty)
   {
      cerr << "Could not find the Depth property in the project file " << endl
            << "Are you sure the project file contains output data?" << endl;
      return 1;
   }

   const Property *isoSurfaceProperty = projectHandle->findProperty ( isoSurfacePropertyName );

   if ( isoSurfaceProperty == 0 )
   {
      cerr << "Could not find the iso-surface property, " << isoSurfacePropertyName << ", in the project file " << endl
            << "Are you sure the project file contains this output data?" << endl;
      return 1;
   }

   const Grid *grid = projectHandle->getLowResolutionOutputGrid ();

   // Get the depth volumes for all formations to build the eclipse grid.
   PropertyValueList *depthPropertyValueList =
         projectHandle->getPropertyValues (FORMATION, depthProperty, snapshot, 0, 0, 0);

   if (depthPropertyValueList->size () == 0)
   {
      cerr << "Could not find the Depth property results in the project file " << endl
           << "Are you sure the project file contains output data?" << endl
           << "Use -listprops to see available properties." << endl;
      return 1;
   }

   // Get the depth volumes for all formations to build the eclipse grid.
   PropertyValueList *isoSurfacePropertyValueList =
         projectHandle->getPropertyValues (FORMATION, isoSurfaceProperty, snapshot, 0, 0, 0);

   if ( isoSurfacePropertyValueList->size () == 0)
   {
      cerr << "Could not find the iso-surface property, " << isoSurfacePropertyName << ", results in the project file " << endl
            << "Are you sure the project file contains output data?" << endl;
      return 1;
   
   }

   if ( not MapWriterFactory::getInstance ().isValidMapType ( outputMapType )) {
      cerr << "Undefined map-type: " << outputMapType << endl;
      return 1;
   }

   unsigned int totalDepth = 0;
   unsigned int i;
   unsigned int j;
   unsigned int k;
   double topIsoProp;
   double botIsoProp;
   double fraction;

   double** depthMap;

   IsoMapWriter* outputMap = MapWriterFactory::getInstance ().allocateMapWriter ( outputMapType );

   assert ( outputMap != 0 );

   FormationList* formations = projectHandle->getFormations ( snapshot, includeBasement );
   FormationList::const_iterator formationIter;

   const GridMap *gridMap = 0;

   if (debug)
   {
      cout << "Formations with Depth property found: " << endl;
   }

   PropertyValueFormationComparator formationComparator;

   //----------------------------//

   depthMap = Array<double>::create2d ( grid->numI (), grid->numJ (), UndefinedValue );

   for ( formationIter = formations->begin (); formationIter != formations->end (); ++formationIter ) {

      if ( verbose ) {
         cout << " formation: " << (*formationIter)->getName () << endl;
      }

      formationComparator.setFormation ( *formationIter );

      PropertyValueList::const_iterator depthProperty = std::find_if ( depthPropertyValueList->begin (),      depthPropertyValueList->end (),      formationComparator );
      PropertyValueList::const_iterator isoProperty   = std::find_if ( isoSurfacePropertyValueList->begin (), isoSurfacePropertyValueList->end (), formationComparator );

      if ( depthProperty == depthPropertyValueList->end ()) {
         cerr << " Depth value for formation " << (*formationIter)->getName () << " cannot be found."  << endl;
         cerr << " Using depths from the remaining formations." << endl;
         continue;
      }

      if ( isoProperty == isoSurfacePropertyValueList->end ()) {
         cerr << " property " << isoSurfacePropertyName << " for formation " << (*formationIter)->getName () << " cannot be found."  << endl;
         cerr << " Using data from the remaining formations." << endl;
         continue;
      }

      const GridMap* depthGrid   = (*depthProperty)->getGridMap ();
      const GridMap* isoPropGrid = (*isoProperty)->getGridMap ();

      for ( i = grid->firstI (); i <= grid->lastI (); ++i ) {

         for ( j = grid->firstJ (); j <= grid->lastJ (); ++j ) {
            topIsoProp = isoPropGrid->getValue ( i, j, (unsigned int) 0 );

            for ( k = 1; k < isoPropGrid->getDepth (); ++k ) {
               botIsoProp = isoPropGrid->getValue ( i, j, k );

               if ( NumericFunctions::inRange<double> ( isoSurfacePropertyValue,
                                                        NumericFunctions::Minimum<double> ( topIsoProp, botIsoProp ),
                                                        NumericFunctions::Maximum<double> ( topIsoProp, botIsoProp ))) {

                  // Take the depth of the top-most iso-surface property value only.
                  if ( depthMap [ i ][ j ] == UndefinedValue ) {
                     fraction = computeIsoparametricFraction ( isoSurfacePropertyValue,
                                                               topIsoProp,
                                                               botIsoProp );

                     depthMap [ i ][ j ] = computeIsoparametricValue ( fraction,
                                                                       depthGrid->getValue ( i, j, k - 1 ),
                                                                       depthGrid->getValue ( i, j, k ));

                     if ( debug ) {
                        cout << " iso-surface location: " << setw ( 5 ) << i << "  " << setw ( 5 ) << j << "  " << setw ( 5 ) << k << "  " 
                             << setw ( 15 ) << fraction << "  " 
                             << setw ( 15 ) << topIsoProp << "  " << setw ( 15 ) << botIsoProp << "  " 
                             << setw ( 15 ) << depthGrid->getValue ( i, j, k - 1 ) << "  " 
                             << setw ( 15 ) << depthGrid->getValue ( i, j, k )  << "  " 
                             << setw ( 15 ) << depthMap [ i ][ j ] << endl;
                     }

                  }

               } else {
                  topIsoProp = botIsoProp;
               }

            }

         }

      }

   }

   // Should perform interpolation here if necessary.

   outputMap->write ( grid, depthMap, outputFileName );

   delete outputMap;
   delete depthPropertyValueList;
   delete isoSurfacePropertyValueList;
   delete formations;
   Array<double>::delete2d ( depthMap );

   CloseCauldronProject (projectHandle);
   if (debug) cout << "Project closed" << endl;

   return 0;
}

void showUsage (const char * message)
{
   cerr << endl;

   if (message)
   {
      cerr << argv0 << ": " << message << endl;
   }

   cerr << "Usage: " << argv0
      << " -project <cauldron-project-file> -output <output-file-no-extension> [-verbose] [-snapshot age] [-isp <property-name>] -ispval <iso-surface value> [-maptype <map-type>] [-nullval <null-value>] [-nobasement] [-listprops] [-listsnap] [-help]"
      << endl;

   cerr << endl;
   cerr << "   -project <project-file>     a cauldron project with results (required)." << endl;
   cerr << "   -output <output-file>       the name of the output file WITH NO EXTENSION, extension will be added depending on map-type (required)." << endl;
   cerr << "   -snapshot <age>             will take nearest snapshot to age specified, (optional) default 0.0." << endl;
   cerr << "   -isp <property-name>        any property for which there are results, (optional) default " << DefaultIsoSurfacePropertyName << "." << endl;
   cerr << "   -ispval <iso-surface-value> the value of the property for the iso-surface (required)." << endl;
   cerr << "   -maptype <map-type>         valid map-types are: xyz, zycor, (optional) default " << DefaultMapType << "." << endl;
   cerr << "   -nullval <null-value>       change the null value from its default, (optional) default " << CauldronUndefined << "." << endl;
   cerr << "   -nobasement                 do not include the basement in the iso-surface, (optional) default includes basement." << endl;
   cerr << "   -listprops                  list all available properties and exit." << endl;
   cerr << "   -listsnap                   list all available major snapshots and exit." << endl;
   cerr << "   -help                       display this help and exit." << endl;
   cerr << endl;

   exit (1);
}


PropertyValueFormationComparator::PropertyValueFormationComparator () {
   m_formation = 0;
}

void PropertyValueFormationComparator::setFormation ( const Formation* newFormation ) {
   m_formation = newFormation;
}

bool PropertyValueFormationComparator::operator ()( const PropertyValue* prop ) const {
   return prop->getFormation () == m_formation;
}


double computeIsoparametricFraction ( const double value, 
                                      const double upperValue,
                                      const double lowerValue ) {

   return ( 2.0 * value - lowerValue - upperValue ) / ( lowerValue - upperValue );
}


double computeIsoparametricValue ( const double fraction,
                                   const double upperDepth,
                                   const double lowerDepth ) {

   return 0.5 * ( 1.0 - fraction ) * upperDepth + 0.5 * ( 1.0 + fraction ) * lowerDepth;
}

bool XYZMapWriter::write ( const Grid*    grid,
                                 double** data,
                           const std::string& fileName ) {

   std::string fullFileName = fileName + ".XYZ";

   unsigned int i;
   unsigned int j;

   double x;
   double y = grid->minJ ();
   double deltaX = grid->deltaI ();
   double deltaY = grid->deltaJ ();

   ofstream mapFile;

   mapFile.open ( fullFileName.c_str ());

   if ( mapFile.fail ()) {
      return false;
   }

   mapFile.precision ( 10 );
   mapFile.flags ( ios::scientific );

   for ( j = grid->firstJ (); j <= grid->lastJ (); ++j ) {
      x = grid->minI ();

      for ( i = grid->firstI (); i <= grid->lastI (); ++i ) {
         mapFile << setw ( 20 ) << x << setw ( 20 ) << y << setw ( 20 ) << data [ i ][ j ] << endl;
         x += deltaX;
      }

      y += deltaY;
   }

   mapFile.close ();

   return true;
}

const unsigned int ZycorMapWriter::NumberOfColumns = 4;
const unsigned int ZycorMapWriter::NumberWidth = 20;

void ZycorMapWriter::writeHeader ( ofstream& mapFile,
                                   const Grid*     grid ) {

   mapFile << "@      ISO_SURFACE   HEADER     , GRID,    " << NumberOfColumns << endl;

   // I have no idea what these other numbers are for: 6 and 1.
   mapFile << "  " << NumberWidth << ", "  << UndefinedValue << ", , 6, 1" << endl;
   mapFile << "   " << grid->numJ () << ", " << grid->numI () << ", " 
           << grid->minI () << ", " 
           << grid->maxI () << ", " 
           << grid->minJ () << ", " 
           << grid->maxJ () << ", " 
           << endl;
   mapFile << "      0,      0,     0 " << endl << "@" << endl << endl;

}



bool ZycorMapWriter::write ( const Grid*    grid,
                                   double** data,
                             const std::string& fileName ) {

   std::string fullFileName = fileName + ".ZYC";

   ofstream mapFile;

   mapFile.open ( fullFileName.c_str ());

   if ( mapFile.fail ()) {
      return false;
   }

   mapFile.precision ( 10 );
   mapFile.flags ( ios::scientific );

   writeHeader ( mapFile, grid );

   unsigned int count = 1;
   unsigned int i;
   unsigned int j;

   for ( i = grid->firstI (); i <= grid->lastI (); ++i ) {
      count = 1;

      for ( j = grid->firstJ (); j <= grid->lastJ (); ++j, ++count ) {
         mapFile << setw ( NumberWidth ) << data [ i ][ grid->lastJ () - j + grid->firstJ ()];

         if ( count % NumberOfColumns == 0 ) {
            mapFile << endl;
         }

      }

      if (( count - 1 ) % NumberOfColumns != 0 ) {
         mapFile << endl;
      }

   }

   mapFile << endl;

   mapFile.close ();

   return true;
}



IsoMapWriter* allocateXYZMapWriter () {
   return new XYZMapWriter;
}

IsoMapWriter* allocateZycorMapWriter () {
   return new ZycorMapWriter;
}

MapWriterFactory* MapWriterFactory::s_mapWriterFactory = 0;

MapWriterFactory& MapWriterFactory::getInstance () {

   if ( s_mapWriterFactory == 0 ) {
      s_mapWriterFactory = new MapWriterFactory;
   }

   return *s_mapWriterFactory;
}

MapWriterFactory::MapWriterFactory () {
   m_allocators [ XYZMapwriterName ] = allocateXYZMapWriter;
   m_allocators [ ZycorMapwriterName ] = allocateZycorMapWriter;
}

IsoMapWriter* MapWriterFactory::allocateMapWriter ( const std::string& mapType ) const {

   std::map<string, MapWriterAllocator>::const_iterator mapAllocator = m_allocators.find ( mapType );

   if ( mapAllocator == m_allocators.end ()) {
      return 0;
   } else {
      return (*mapAllocator->second)();
   }

}


bool MapWriterFactory::isValidMapType ( const std::string& mapType ) const {

   std::map<string, MapWriterAllocator>::const_iterator mapAllocator = m_allocators.find ( mapType );

   return mapAllocator != m_allocators.end ();
}


GridDescription::GridDescription ( const unsigned int numberX,
                                   const unsigned int numberY,
                                   const double       originX,
                                   const double       originY,
                                   const double       deltaX,
                                   const double       deltaY ) {

   m_numberX = numberX;
   m_numberY = numberY;

   m_deltaX = deltaX;
   m_deltaY = deltaY;

   m_originX = originX;
   m_originY = originY;
   
}

GridDescription::GridDescription ( const Grid* grid ) {

 m_numberX = grid->numI ();
 m_numberY = grid->numJ ();

 m_originX = grid->firstI ();
 m_originY = grid->firstJ ();

 m_deltaX  = grid->deltaI ();
 m_deltaY  = grid->deltaJ ();

}


inline unsigned int GridDescription::getNumberInX () const {
   return m_numberX;
}

unsigned int GridDescription::getNumberInY () const {
   return m_numberY;
}

double GridDescription::getOriginX () const {
   return m_originX;
}

double GridDescription::getOriginY () const {
   return m_originY;
}

double GridDescription::getDeltaX () const {
   return m_deltaX;
}

double GridDescription::getDeltaY () const {
   return m_deltaY;
}

double GridDescription::getLastX () const {
   return m_originX + m_deltaX * double ( m_numberX );
}

double GridDescription::getLastY () const {
   return m_originY + m_deltaY * double ( m_numberY );
}
