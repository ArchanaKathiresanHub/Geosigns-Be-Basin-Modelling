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

#include "GeoPhysicsObjectFactory.h"
#include "GeoPhysicsProjectHandle.h"
#include "OutputPropertyValue.h"
#include "FormationOutputPropertyValue.h"
#include "FormationMapOutputPropertyValue.h"
#include "SurfaceOutputPropertyValue.h"

using namespace std;

using namespace DataAccess;
using namespace Interface;
using namespace DerivedProperties;

typedef vector < string > StringVector;
typedef vector < double > DoubleVector;

typedef map < const Property *, OutputPropertyValuePtr > OutputPropertyValueMap;
typedef map < const Formation *, OutputPropertyValueMap > FormationOutputPropertyValueMap;
typedef map < const Snapshot *, FormationOutputPropertyValueMap> SnapshotFormationOutputPropertyValueMap;

typedef vector < const Formation * > FormationVector;

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

   bool m_debug;
   bool m_basement;
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

   bool showLists();

   bool startActivity();
   void finalise ( bool isComplete );
   bool CreateFrom( DataAccess::Interface::ObjectFactory* factory);

   bool setFastcauldronActivityName();

   void outputSnapshotFormationData( const Snapshot * snapshot,
                                     const Formation * formation, PropertyList & properties,
                                     SnapshotFormationOutputPropertyValueMap & allOutputPropertyValues );


   bool createSnapshotResultPropertyValue ( OutputPropertyValuePtr propertyValue,
                                            const Snapshot* theSnapshot, 
                                           const Formation * formation );

   OutputPropertyValuePtr allocateOutputProperty ( DerivedProperties::AbstractPropertyManager& propertyManager, 
                                                   const DataModel::AbstractProperty* property, 
                                                   const DataModel::AbstractSnapshot* snapshot,
                                                   const Interface::Formation* formationItem );   
 
   const GridMap * getPropertyGridMap ( const string & propertyName,
                                        const Interface::Snapshot * snapshot,
                                        const Formation * formation );

   bool toBeSaved ( const string & propertyName,
                    const Interface::Snapshot * snapshot,
                    const Formation * formation );

   bool toBeSaved ( const string & propertyName,
                    const Interface::Snapshot * snapshot,
                    const Surface* surface );
   
   /// @brief Acquire outputable 3D properties from project handle
   /// @pre -all-3D-properties must be specified as a command line parameter
   void acquireAll3Dproperties();
   /// @brief Acquire outputable 2D properties from project handle
   /// @pre -all-2D-properties must be specified as a command line parameter
   void acquireAll2Dproperties();

   bool acquireSnapshots  ( SnapshotList    & snapshots     );
   bool acquireProperties ( PropertyList    & properties    );
   bool acquireFormations ( FormationVector & formationItem );

   /// @brief Print all outputable 2D and 3D properties from project handle
   /// @pre -list-properties must be specified as a command line parameter
   void printOutputableProperties ();
   void printListSnapshots ();
   void printListStratigraphy ();

   void calculateProperties( FormationVector& formationItems, PropertyList properties, SnapshotList & snapshots );
   
   bool parseCommandLine ( int argc, char ** argv );
   void showUsage( const char* command, const char* message = 0 );
 
};

#endif // FASTPROPERTIES__PROPERTIES_CALCULATOR__H
