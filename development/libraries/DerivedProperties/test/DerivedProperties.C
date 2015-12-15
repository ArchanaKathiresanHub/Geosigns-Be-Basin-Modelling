#include "../src/DerivedSurfaceProperty.h"
#include "../src/DerivedFormationProperty.h"
#include "../src/DerivedFormationSurfaceProperty.h"
#include "MockProperty.h"
#include "MockSnapshot.h"
#include "MockFormation.h"
#include "MockSurface.h"
#include "MockGrid.h"

#include <gtest/gtest.h>

#include <iostream>
using namespace std;

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
   static const unsigned int NumberK = 10;

   static const std::string SurfaceName   = "Surface Name";
   static const std::string FormationName = "Formation Name";
   static const std::string PropertyName  = "Property Name";
   static const double SnapshotAge  = 10.0;
}

TEST( DerivedSurfaceProperty, Test2 ) 
{
   MockGrid testGrid( FirstI, FirstGhostI, FirstJ, FirstGhostJ, LastI, LastGhostI, LastJ, LastGhostJ );

   EXPECT_EQ( FirstI, testGrid.firstI( false ));
   EXPECT_EQ( FirstGhostI, testGrid.firstI( true ));
   EXPECT_EQ( FirstJ, testGrid.firstJ( false ));
   EXPECT_EQ( FirstGhostJ, testGrid.firstJ( true ));
}
   

TEST( DerivedSurfaceProperty, Test1 ) 
{
   MockGrid      testGrid( FirstI, FirstGhostI, FirstJ, FirstGhostJ, LastI, LastGhostI, LastJ, LastGhostJ );
   MockSurface   testSurface( SurfaceName );
   MockSnapshot  testSnapshot( SnapshotAge );
   MockProperty  testProperty( PropertyName );
   MockFormation testFormation( FormationName );
   
   DerivedProperties::DerivedSurfaceProperty s ( &testProperty, &testSnapshot, &testSurface, &testGrid );
   DerivedProperties::DerivedFormationProperty f ( &testProperty, &testSnapshot, &testFormation, &testGrid, NumberK );
   DerivedProperties::DerivedFormationSurfaceProperty sf ( &testProperty, &testSnapshot, &testFormation, &testSurface, &testGrid );

   double surfaceValue   = 0.0;
   double formationValue = 0.0;
   for( int i = testGrid.firstI( true ); i < testGrid.lastI( true ) + 1; ++ i ) {
      for( int j = testGrid.firstJ( true ); j < testGrid.lastJ( true ) + 1; ++ j ) {
         s.set(  i, j, surfaceValue  );
         sf.set( i, j, surfaceValue ++ );
         for( int k = f.firstK(); k < f.lastK() + 1; ++ k ) {
            f.set( i, j, k, formationValue ++ );
         }
      }
   }
   EXPECT_DOUBLE_EQ( 24.0,  s.get ( 3, 3 ) );
   EXPECT_DOUBLE_EQ( 243.0, f.get ( 3, 3, 3 ));
   EXPECT_DOUBLE_EQ( 17.0,  sf.get( 2, 3 ) );
}
