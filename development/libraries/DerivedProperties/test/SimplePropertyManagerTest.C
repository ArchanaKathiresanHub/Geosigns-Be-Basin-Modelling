#include <iostream>
#include <vector>
#include <string>
#include "../src/DerivedPropertyManager.h"
#include "../src/SurfacePropertyCalculator.h"
#include "../src/DerivedSurfaceProperty.h"

#include "MockSurface.h"
#include "MockFormation.h"
#include "MockSnapshot.h"
#include "MockProperty.h"
#include "MockGrid.h"

#include <gtest/gtest.h>

using namespace DataModel;
using namespace DerivedProperties;

class TestPropertyManager : public DerivedProperties::DerivedPropertyManager {

public :

   TestPropertyManager ();

   ~TestPropertyManager ();

   const DataModel::AbstractGrid* getMapGrid () const;

private :

   std::vector<MockProperty*> m_mockProperties;
   const DataModel::AbstractGrid* m_mapGrid;

};

class Property1Calculator : public DerivedProperties::SurfacePropertyCalculator {

public :

   void calculate ( DerivedProperties::DerivedPropertyManager& propertyManager,
                    const DataModel::AbstractSnapshot*        snapshot,
                    const DataModel::AbstractSurface*         surface,
                          SurfacePropertyList&                derivedProperties ) const;

};

TEST ( DerivedPropertyManagerTest,  Test1 )
{
   TestPropertyManager propertyManager;

   const DataModel::AbstractProperty* property = propertyManager.getProperty ( "Property1" );

   const DataModel::AbstractSnapshot*  snapshot = new MockSnapshot ( 0.0 );
   const DataModel::AbstractSurface*   surface = new MockSurface ( "TopSurface" );
   const DataModel::AbstractFormation* formation = new MockFormation ( "Formation1" );

   SurfacePropertyPtr surfaceProperty1 = propertyManager.getSurfaceProperty ( property, snapshot, surface );

   double value = 0.0;

   EXPECT_EQ ( property->getName (), "Property1" );
   EXPECT_EQ ( property, surfaceProperty1->getProperty ());
   EXPECT_EQ ( surface, surfaceProperty1->getSurface ());
   EXPECT_EQ ( snapshot, surfaceProperty1->getSnapshot ());

   for ( unsigned int i = surfaceProperty1->firstI ( true ); i <= surfaceProperty1->lastI ( true ); ++i ) {

      for ( unsigned int j = surfaceProperty1->firstJ ( true ); j <= surfaceProperty1->lastJ ( true ); ++j ) {
         EXPECT_DOUBLE_EQ ( value, surfaceProperty1->get ( i, j ));
         value += 1.0;
      }

   }

}


TestPropertyManager::TestPropertyManager () {
   // These will come from the project handle.
   m_mockProperties.push_back ( new DataModel::MockProperty ( "Property1" ) );

   // Add all properties to the property manager.
   for ( size_t i = 0; i < m_mockProperties.size (); ++i ) {
      addProperty ( m_mockProperties [ i ]);
   }

   // This will come frmo the project handle.
   m_mapGrid = new DataModel::MockGrid ( 0, 0, 0, 0, 10, 10, 10, 10 );

   addCalculator ( m_mockProperties [ 0 ], SurfacePropertyCalculatorPtr ( new Property1Calculator ));
}


TestPropertyManager::~TestPropertyManager () {

   for ( size_t i = 0; i < m_mockProperties.size (); ++i ) {
      delete m_mockProperties [ i ];
   }

   delete m_mapGrid;
}

const DataModel::AbstractGrid* TestPropertyManager::getMapGrid () const {
   return m_mapGrid;
}

void Property1Calculator::calculate ( DerivedProperties::DerivedPropertyManager& propertyManager,
                                      const DataModel::AbstractSnapshot*        snapshot,
                                      const DataModel::AbstractSurface*         surface,
                                      SurfacePropertyList&                derivedProperties ) const {

   const DataModel::AbstractProperty* property = propertyManager.getProperty ( "Property1" );

   DerivedSurfacePropertyPtr derivedProp = DerivedSurfacePropertyPtr ( new DerivedProperties::DerivedSurfaceProperty ( property, snapshot, surface, propertyManager.getMapGrid ()));
   double value = 0.0;

   for ( unsigned int i = derivedProp->firstI ( true ); i <= derivedProp->lastI ( true ); ++i ) {

      for ( unsigned int j = derivedProp->firstJ ( true ); j <= derivedProp->lastJ ( true ); ++j ) {
         derivedProp->set ( i, j, value );
         value += 1.0;
      }

   }

   derivedProperties.push_back ( derivedProp );
}


