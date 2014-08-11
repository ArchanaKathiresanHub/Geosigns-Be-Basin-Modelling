#include "../src/DerivedSurfaceProperty.h"
#include <gtest/gtest.h>

using namespace DataModel;

namespace
{
   static const int FirstI      = 2;
   static const int FirstJ      = 2;
   static const int FirstGhostI = 0;
   static const int FirstGhostJ = 0;
   static const int LastI       = 4;
   static const int LastJ       = 4;
   static const int LastGhostI  = 6;
   static const int LastGhostJ  = 6;

   static const std::string SurfaceName  = "Surface Test";
   static const std::string PropertyName = "Property Test";
   static const double SnapshotAge  = 10.0;
}

class TestAbstractGrid : public AbstractGrid {

public:
   int firstI (void) const { return FirstI; }; 
   int firstI (bool withGhosts) const { return FirstGhostI; };
   
   /// return the first local vertical index
   int firstJ (void) const { return FirstJ; };
   int firstJ (bool withGhosts) const { return FirstGhostJ; };
   
   /// return the last local horizontal index
   int lastI (void) const { return LastI; };
   int lastI (bool withGhosts) const { return LastJ; };
   
   /// return the last local vertical index
   int lastJ (void) const { return LastGhostI; };
   int lastJ (bool withGhosts) const { return LastGhostJ ;};
   
};

class TestAbstractSnapshot : public AbstractSnapshot {

public:
   double getTime () const { return SnapshotAge; };
};

class TestAbstractSurface : public AbstractSurface {

public:
   const std::string& getName () const { return SurfaceName; };
};

class TestAbstractProperty : public AbstractProperty {

public:
   const std::string& getName () const { return PropertyName; };
};

class TestSurfaceProperty : public DerivedProperties::DerivedSurfaceProperty {

public:
   TestSurfaceProperty ( const TestAbstractProperty* prop,
                         const TestAbstractSnapshot* ss,
                         const TestAbstractSurface*  surf,
                         const TestAbstractGrid*     g ) : DerivedSurfaceProperty ( prop, ss, surf, g ) { 

      double value = 0.0;
      for( int i = g->firstI( true); i < g->lastI (true ) + 1; ++ i ) {
         for( int j = g->firstJ(true); j < g->lastJ(true) + 1; ++j ) {
            set( i, j, value ++ );
         }
      }
   };

};


TEST( DerivedSurfaceProperty, Test1 )
{
   TestAbstractGrid     testGrid;
   TestAbstractSurface  testSurface;
   TestAbstractSnapshot testSnapshot;
   TestAbstractProperty testProperty;

   TestSurfaceProperty m ( &testProperty, &testSnapshot, &testSurface, &testGrid );

   EXPECT_DOUBLE_EQ( 24.0, m.get ( 3, 3 ) );
}
