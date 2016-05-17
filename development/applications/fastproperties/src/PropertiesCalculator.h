//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#ifndef FASTPROPERTIES__PROPERTIES_CALCULATOR__H
#define FASTPROPERTIES__PROPERTIES_CALCULATOR__H

//std library
#include <string>
#include <sstream>
#include <vector>
#include <utility>
#include <stdexcept>

//DataAccess library
#include "Interface/Snapshot.h"
#include "Interface/Surface.h"
#include "Interface/Formation.h"
#include "Interface/ObjectFactory.h"

#include "Interface/Property.h"
#include "Interface/PropertyValue.h"

#include "PropertyAttribute.h"
#include "AbstractPropertyManager.h"
#include "DerivedPropertyManager.h"
#include "OutputUtilities.h"

#include "GeoPhysicsObjectFactory.h"
#include "GeoPhysicsProjectHandle.h"
#include "OutputPropertyValue.h"
#include "FormationOutputPropertyValue.h"
#include "FormationMapOutputPropertyValue.h"
#include "SurfaceOutputPropertyValue.h"

#include "VisualizationAPI.h"
#include "ImportExport.h"
#include "ImportProjectHandle.h"

using namespace std;

using namespace DataAccess;
using namespace Interface;
using namespace DerivedProperties;

typedef vector < double > DoubleVector;

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
void Swap( T & x, T & y )
{
   T v;
   v = x;
   x = y;
   y = v;
}

class PropertiesCalculator {


public :
   PropertiesCalculator( int aRank );

   ~PropertiesCalculator();

   int m_rank;

private:
   GeoPhysics::ProjectHandle* m_projectHandle;
   DerivedPropertyManager * m_propertyManager;
   
   bool m_convert;
   bool m_primaryPod;
   bool m_extract2D;         ///< true if 2D primary/derived properties to be calculated and saved
   bool m_no3Dproperties;    ///< true if no 3d properties are defined to be calculated

   bool m_debug;
   bool m_basement;
   bool m_copy;
   bool m_all2Dproperties;    ///< If true: computes all 2D derived properties
   bool m_all3Dproperties;    ///< If true: computes all 3D derived properties
   bool m_listProperties;     ///< If true: prints all outputable properties
   bool m_listSnapshots;      ///< If true: prints all snapshots from project file
   bool m_listStratigraphy;   ///< If true: prints all stratigraphy from project file

   string m_projectFileName;

   StringVector m_propertyNames;
   DoubleVector m_ages;
   StringVector m_formationNames;

   string m_activityName;   ///< The name of the current activity producing output values
   string m_simulationMode; ///< The name of the last simulation fastcauldron mode

   int m_snapshotsType; ///< The type of snapshots to calculate derived properties at

public:

   GeoPhysics::ProjectHandle* getProjectHandle() const;
   DerivedPropertyManager * getPropertyManager() const;

   bool showLists();

   bool startActivity();
   bool finalise ( bool isComplete );
   bool CreateFrom( DataAccess::Interface::ObjectFactory* factory);

   bool setFastcauldronActivityName();

   bool acquireSnapshots( SnapshotList & snapshots );
   /// @brief Acquire outputable 3D properties from project handle
   /// @pre -all-3D-properties must be specified as a command line parameter
   void acquireAll3Dproperties();
   /// @brief Acquire outputable 2D properties from project handle
   /// @pre -all-2D-properties must be specified as a command line parameter
   void acquireAll2Dproperties();
   void acquireFormationsSurfaces( FormationSurfaceVector & formationSurfaceItems );
   void acquireProperties( PropertyList & properties );

   /// @brief Print all outputable 2D and 3D properties from project handle
   /// @pre -list-properties must be specified as a command line parameter
   void printOutputableProperties ();
   void printListSnapshots ();
   void printListStratigraphy ();

   void calculateProperties( FormationSurfaceVector& formationItems, PropertyList properties, SnapshotList & snapshots );
   
   void convertToVisualizationIO();
   bool convert() const;

   bool parseCommandLine ( int argc, char ** argv );
   void showUsage( const char* command, const char* message = 0 );

   bool copyFiles();
 
};

#endif // FASTPROPERTIES__PROPERTIES_CALCULATOR__H
