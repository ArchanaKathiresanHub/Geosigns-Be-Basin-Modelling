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

static const double ValueToAdd = 10.0;

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

class Property2Calculator : public DerivedProperties::SurfacePropertyCalculator {

public :

   Property2Calculator ( const double value );

   void calculate ( DerivedProperties::DerivedPropertyManager& propertyManager,
                    const DataModel::AbstractSnapshot*        snapshot,
                    const DataModel::AbstractSurface*         surface,
                    SurfacePropertyList&                derivedProperties ) const;

private :

   double m_value;

};


// Tests whether properties are retrieved correctly from the property-manager.
//
// Tests whether compound surface property values are correctly retrieved from the property-manager.
//
// A compound property is a property that has more than one set of values, 
// e.g. heat-flow is made up of heat-flow-x, -y and -z.
//
// Tests whether the dependencies are computed correctly. In this case property2 and proerty3 both
// depend on property1. However, at the time of the request of property2, property1 has not been calculated
// so it must also be calculated. 
TEST ( DerivedPropertyManagerTest,  Test1 )
{
   TestPropertyManager propertyManager;

   const DataModel::AbstractProperty* property1 = propertyManager.getProperty ( "Property1" );
   const DataModel::AbstractProperty* property2 = propertyManager.getProperty ( "Property2" );
   const DataModel::AbstractProperty* property3 = propertyManager.getProperty ( "Property3" );

   const DataModel::AbstractSnapshot*  snapshot = new MockSnapshot ( 0.0 );
   const DataModel::AbstractSurface*   surface = new MockSurface ( "TopSurface" );
   const DataModel::AbstractFormation* formation = new MockFormation ( "Formation1" );

   SurfacePropertyPtr surfaceProperty2 = propertyManager.getSurfaceProperty ( property2, snapshot, surface );
   SurfacePropertyPtr surfaceProperty1 = propertyManager.getSurfaceProperty ( property1, snapshot, surface );
   SurfacePropertyPtr surfaceProperty3 = propertyManager.getSurfaceProperty ( property3, snapshot, surface );

   EXPECT_EQ ( property1, surfaceProperty1->getProperty ());
   EXPECT_EQ ( surface, surfaceProperty1->getSurface ());
   EXPECT_EQ ( snapshot, surfaceProperty1->getSnapshot ());

   EXPECT_EQ ( property2, surfaceProperty2->getProperty ());
   EXPECT_EQ ( surface, surfaceProperty2->getSurface ());
   EXPECT_EQ ( snapshot, surfaceProperty2->getSnapshot ());

   EXPECT_EQ ( property3, surfaceProperty3->getProperty ());
   EXPECT_EQ ( surface, surfaceProperty3->getSurface ());
   EXPECT_EQ ( snapshot, surfaceProperty3->getSnapshot ());

   for ( unsigned int i = surfaceProperty1->firstI ( true ); i <= surfaceProperty1->lastI ( true ); ++i ) {

      for ( unsigned int j = surfaceProperty1->firstJ ( true ); j <= surfaceProperty1->lastJ ( true ); ++j ) {
         EXPECT_DOUBLE_EQ ( surfaceProperty1->get ( i, j ) + ValueToAdd, surfaceProperty2->get ( i, j ));
      }

   }

   for ( unsigned int i = surfaceProperty1->firstI ( true ); i <= surfaceProperty1->lastI ( true ); ++i ) {

      for ( unsigned int j = surfaceProperty1->firstJ ( true ); j <= surfaceProperty1->lastJ ( true ); ++j ) {
         EXPECT_DOUBLE_EQ ( surfaceProperty1->get ( i, j ) + ValueToAdd * ValueToAdd, surfaceProperty3->get ( i, j ));
      }

   }

}


TestPropertyManager::TestPropertyManager () {
   // These will come from the project handle.
   m_mockProperties.push_back ( new DataModel::MockProperty ( "Property1" ) );
   m_mockProperties.push_back ( new DataModel::MockProperty ( "Property2" ) );
   m_mockProperties.push_back ( new DataModel::MockProperty ( "Property3" ) );

   // Add all properties to the property manager.
   for ( size_t i = 0; i < m_mockProperties.size (); ++i ) {
      addProperty ( m_mockProperties [ i ]);
   }

   // This will come frmo the project handle.
   m_mapGrid = new DataModel::MockGrid ( 0, 0, 0, 0, 10, 10, 10, 10 );

   addCalculator ( m_mockProperties [ 0 ], SurfacePropertyCalculatorPtr ( new Property1Calculator ));

   SurfacePropertyCalculatorPtr calc = SurfacePropertyCalculatorPtr ( new Property2Calculator ( ValueToAdd ));

   addCalculator ( m_mockProperties [ 1 ], calc );
   addCalculator ( m_mockProperties [ 2 ], calc );


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

Property2Calculator::Property2Calculator ( const double value ) : m_value ( value ) {
}

void Property2Calculator::calculate ( DerivedProperties::DerivedPropertyManager& propertyManager,
                                      const DataModel::AbstractSnapshot*        snapshot,
                                      const DataModel::AbstractSurface*         surface,
                                      SurfacePropertyList&                derivedProperties ) const {

   const DataModel::AbstractProperty* property1 = propertyManager.getProperty ( "Property1" );
   const DataModel::AbstractProperty* property2 = propertyManager.getProperty ( "Property2" );
   const DataModel::AbstractProperty* property3 = propertyManager.getProperty ( "Property3" );

   const SurfacePropertyPtr prop1 = propertyManager.getSurfaceProperty ( property1, snapshot, surface );

   DerivedSurfacePropertyPtr derivedProp2 = DerivedSurfacePropertyPtr( new DerivedProperties::DerivedSurfaceProperty ( property2, snapshot, surface, propertyManager.getMapGrid ()));
   DerivedSurfacePropertyPtr derivedProp3 = DerivedSurfacePropertyPtr( new DerivedProperties::DerivedSurfaceProperty ( property3, snapshot, surface, propertyManager.getMapGrid ()));

   double value = 0.0;

   for ( unsigned int i = derivedProp2->firstI ( true ); i <= derivedProp2->lastI ( true ); ++i ) {

      for ( unsigned int j = derivedProp2->firstJ ( true ); j <= derivedProp2->lastJ ( true ); ++j ) {
         derivedProp2->set ( i, j, prop1->get ( i, j ) + m_value );
         derivedProp3->set ( i, j, prop1->get ( i, j ) + m_value * m_value );
      }

   }

   derivedProperties.push_back ( derivedProp2 );
   derivedProperties.push_back ( derivedProp3 );
}
