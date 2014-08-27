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

class Property4Calculator : public DerivedProperties::SurfacePropertyCalculator {

public :

   void calculate ( DerivedProperties::DerivedPropertyManager& propertyManager,
                    const DataModel::AbstractSnapshot*        snapshot,
                    const DataModel::AbstractSurface*         surface,
                    SurfacePropertyList&                derivedProperties ) const;

private :

   double m_value;

};

TEST ( DerivedPropertyManagerTest,  Test1 )
{
   TestPropertyManager propertyManager;

   const DataModel::AbstractProperty* property1 = propertyManager.getProperty ( "Property1" );
   const DataModel::AbstractProperty* property2 = propertyManager.getProperty ( "Property2" );
   const DataModel::AbstractProperty* property3 = propertyManager.getProperty ( "Property3" );
   const DataModel::AbstractProperty* property4 = propertyManager.getProperty ( "Property4" );

   const DataModel::AbstractSnapshot*  snapshot = new MockSnapshot ( 0.0 );
   const DataModel::AbstractSurface*   surface1 = new MockSurface ( "TopSurface" );
   const DataModel::AbstractFormation* formation1 = new MockFormation ( "Formation1" );

   SurfacePropertyPtr surfaceProperty4 = propertyManager.getSurfaceProperty ( property4, snapshot, surface1 );
   SurfacePropertyPtr surfaceProperty2 = propertyManager.getSurfaceProperty ( property2, snapshot, surface1 );
   SurfacePropertyPtr surfaceProperty1 = propertyManager.getSurfaceProperty ( property1, snapshot, surface1 );
   SurfacePropertyPtr surfaceProperty3 = propertyManager.getSurfaceProperty ( property3, snapshot, surface1 );


   EXPECT_EQ ( property4, surfaceProperty4->getProperty ());
   EXPECT_EQ ( surface1, surfaceProperty4->getSurface ());
   EXPECT_EQ ( snapshot, surfaceProperty4->getSnapshot ());

   for ( unsigned int i = surfaceProperty1->firstI ( true ); i <= surfaceProperty1->lastI ( true ); ++i ) {

      for ( unsigned int j = surfaceProperty1->firstJ ( true ); j <= surfaceProperty1->lastJ ( true ); ++j ) {
         EXPECT_DOUBLE_EQ ( ValueToAdd, surfaceProperty4->get ( i, j ));
      }

   }

}


TestPropertyManager::TestPropertyManager () {
   // These will come from the project handle.
   m_mockProperties.push_back ( new DataModel::MockProperty ( "Property1" ) );
   m_mockProperties.push_back ( new DataModel::MockProperty ( "Property2" ) );
   m_mockProperties.push_back ( new DataModel::MockProperty ( "Property3" ) );
   m_mockProperties.push_back ( new DataModel::MockProperty ( "Property4" ) );

   // Add all properties to the property manager.
   for ( size_t i = 0; i < m_mockProperties.size (); ++i ) {
      addProperty ( m_mockProperties [ i ]);
   }

   // This will come frmo the project handle.
   // m_mapGrid = new DataModel::MockGrid ( 0, 0, 0, 0, 10, 10, 10, 10 );
   m_mapGrid = new DataModel::MockGrid ( 0, 0, 0, 0, 4, 4, 4, 4 );

   addCalculator ( m_mockProperties [ 0 ], SurfacePropertyCalculatorPtr ( new Property1Calculator ));

   SurfacePropertyCalculatorPtr calc = SurfacePropertyCalculatorPtr ( new Property2Calculator ( ValueToAdd ));

   addCalculator ( m_mockProperties [ 1 ], calc );
   addCalculator ( m_mockProperties [ 2 ], calc );

   addCalculator ( m_mockProperties [ 3 ], SurfacePropertyCalculatorPtr ( new Property4Calculator ));
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


void Property4Calculator::calculate ( DerivedProperties::DerivedPropertyManager& propertyManager,
                                      const DataModel::AbstractSnapshot*        snapshot,
                                      const DataModel::AbstractSurface*         surface,
                                      SurfacePropertyList&                derivedProperties ) const {

   const DataModel::AbstractProperty* property1 = propertyManager.getProperty ( "Property1" );
   const DataModel::AbstractProperty* property2 = propertyManager.getProperty ( "Property2" );
   const DataModel::AbstractProperty* property4 = propertyManager.getProperty ( "Property4" );

   const SurfacePropertyPtr prop1 = propertyManager.getSurfaceProperty ( property1, snapshot, surface );
   const SurfacePropertyPtr prop2 = propertyManager.getSurfaceProperty ( property2, snapshot, surface );

   DerivedSurfacePropertyPtr derivedProp = DerivedSurfacePropertyPtr( new DerivedProperties::DerivedSurfaceProperty ( property4, snapshot, surface, propertyManager.getMapGrid ()));

   for ( unsigned int i = derivedProp->firstI ( true ); i <= derivedProp->lastI ( true ); ++i ) {

      for ( unsigned int j = derivedProp->firstJ ( true ); j <= derivedProp->lastJ ( true ); ++j ) {
         derivedProp->set ( i, j, prop2->get ( i, j ) - prop1->get ( i, j ));
      }

   }

   derivedProperties.push_back ( derivedProp );
}


