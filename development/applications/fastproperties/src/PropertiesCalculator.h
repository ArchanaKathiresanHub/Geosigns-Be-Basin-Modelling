
#ifndef FASTPROPERTIES__PROPERTIES_CALCULATOR__H
#define FASTPROPERTIES__PROPERTIES_CALCULATOR__H

#include <string>
#include <sstream>
#include <vector>
#include <utility>
#include <stdexcept>

#include "Interface/Snapshot.h"
#include "Interface/Surface.h"
#include "Interface/Formation.h"

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
   PropertiesCalculator( );

   ~PropertiesCalculator();

   int m_rank;

private:
   GeoPhysics::ProjectHandle* m_projectHandle;
   DerivedPropertyManager * m_propertyManager;

   bool m_debug;
   bool m_basement;
   bool m_all2Dproperties;
   bool m_all3Dproperties;
   bool m_listProperties;
   bool m_listSnapshots;
   bool m_listStratigraphy;
   string m_projectFileName;
 
   StringVector m_propertyNames;
   DoubleVector m_ages;
   StringVector m_formationNames;

public:

   bool showLists();

   bool startActivity();
   void finalise ( bool isComplete );
   bool CreateFrom( int aRank );

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

   void acquireAll3Dproperties();
   void acquireAll2Dproperties();

   bool acquireSnapshots( SnapshotList & snapshots );
   bool acquireProperties ( PropertyList & properties );
   bool acquireFormations( FormationVector & formationItem );

   void printOutputableProperties ();
   void printListSnapshots ();
   void printListStratigraphy ();

   void calculateProperties( FormationVector& formationItems, PropertyList properties, SnapshotList & snapshots );
   
   bool parseCommandLine ( int argc, char ** argv );
   void showUsage( const char* command, const char* message = 0 );
 
};

#endif // FASTPROPERTIES__PROPERTIES_CALCULATOR__H
