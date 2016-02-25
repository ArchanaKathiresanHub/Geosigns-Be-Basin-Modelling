//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include <iostream>
#include <vector>
#include <string>
#include "../src/AbstractPropertyManager.h"
#include "../src/SurfacePropertyCalculator.h"
#include "../src/DerivedSurfaceProperty.h"
#include "../src/VesSurfaceCalculator.h"

#include "MockSurface.h"
#include "MockSnapshot.h"
#include "MockProperty.h"
#include "MockGrid.h"

// utilitites library
#include "FormattingException.h"

#include <gtest/gtest.h>

using namespace DataModel;
using namespace DerivedProperties;

static const double ValueToAdd = 10.0;

typedef formattingexception::GeneralException MultiplePropertyException;

class TestPropertyManager : public DerivedProperties::AbstractPropertyManager {

public :

   TestPropertyManager ();

   ~TestPropertyManager ();

   const DataModel::AbstractProperty* getProperty ( const std::string& name ) const;

   const DataModel::AbstractGrid* getMapGrid () const;

   /// \brief Return whether or not the node is defined.
   bool getNodeIsValid ( const unsigned int i, const unsigned int j ) const;
private :

   /// \brief List of all available properties.
   typedef std::vector <const DataModel::AbstractProperty*> PropertyList;


   /// \brief Add an abstract property to the list of available properties.
   ///
   /// If a property has been added already then it will not be added a second time.
   void addProperty ( const DataModel::AbstractProperty* property );


   /// \brief Contains list of all known properties.
   PropertyList                 m_properties;

   std::vector<MockProperty*> m_mockProperties;
   const DataModel::AbstractGrid* m_mapGrid;

};


class Property1Calculator : public DerivedProperties::SurfacePropertyCalculator {

public :

   Property1Calculator ();

   void calculate ( DerivedProperties::AbstractPropertyManager& propertyManager,
                    const DataModel::AbstractSnapshot*        snapshot,
                    const DataModel::AbstractSurface*         surface,
                          SurfacePropertyList&                derivedProperties ) const;


};

class Property2Calculator : public DerivedProperties::SurfacePropertyCalculator {

public :

   Property2Calculator ( const double value );

   void calculate ( DerivedProperties::AbstractPropertyManager& propertyManager,
                    const DataModel::AbstractSnapshot*        snapshot,
                    const DataModel::AbstractSurface*         surface,
                    SurfacePropertyList&                derivedProperties ) const;

private :

   double m_value;

};

class Property4Calculator : public DerivedProperties::SurfacePropertyCalculator {

public :

   Property4Calculator ();

   void calculate ( DerivedProperties::AbstractPropertyManager& propertyManager,
                    const DataModel::AbstractSnapshot*        snapshot,
                    const DataModel::AbstractSurface*         surface,
                    SurfacePropertyList&                derivedProperties ) const;

private :

   double m_value;

};

// Tests whether properties are retrieved correctly from the property-manager.
//
// Tests whether compound surface property values are retrieved correctly from the property-manager.
//
// A compound property is a property that has more than one set of values, 
// e.g. heat-flow is made up of heat-flow-x, -y and -z.
//
// Tests whether the dependencies are computed correctly. In this case property2 and proerty3 both
// depend on property1 and property4 depends on both property1 and property2. However, at the time
// of the request of property4, none of property1, property2 and property3 have been calculated
// so they must also be calculated.
TEST ( AbstractPropertyManagerTest,  Test1 )
{

   TestPropertyManager propertyManager;

   const DataModel::AbstractProperty* property1 = propertyManager.getProperty ( "LithoStaticPressure" );
   const DataModel::AbstractProperty* property2 = propertyManager.getProperty ( "Property2" );
   const DataModel::AbstractProperty* property3 = propertyManager.getProperty ( "Property3" );
   const DataModel::AbstractProperty* property4 = propertyManager.getProperty ( "Pressure" );

   const DataModel::AbstractSnapshot*  snapshot = new MockSnapshot ( 0.0 );
   const DataModel::AbstractSnapshot*  anotherSnapshot = new MockSnapshot ( 10.0 );
   const DataModel::AbstractSurface*   surface = new MockSurface ( "TopSurface" );

   const DataModel::AbstractSurface*   bottomSurface = new MockSurface ( "BottomSurface" );


   SurfacePropertyPtr surfaceProperty4 = propertyManager.getSurfaceProperty ( property4, snapshot, surface );
   SurfacePropertyPtr surfaceProperty2 = propertyManager.getSurfaceProperty ( property2, snapshot, surface );
   SurfacePropertyPtr surfaceProperty1 = propertyManager.getSurfaceProperty ( property1, snapshot, surface );
   SurfacePropertyPtr surfaceProperty3 = propertyManager.getSurfaceProperty ( property3, snapshot, surface );

   // These following 2 surface properties should be null after the get surface property call.
   SurfacePropertyPtr surfaceProperty5 = propertyManager.getSurfaceProperty( property3, snapshot, bottomSurface );
   SurfacePropertyPtr surfaceProperty6 = propertyManager.getSurfaceProperty( property3, anotherSnapshot, bottomSurface );


   EXPECT_EQ ( property4, surfaceProperty4->getProperty ());
   EXPECT_EQ ( surface, surfaceProperty4->getSurface ());
   EXPECT_EQ ( snapshot, surfaceProperty4->getSnapshot ());

   for ( unsigned int i = surfaceProperty1->firstI ( true ); i <= surfaceProperty1->lastI ( true ); ++i ) {

      for ( unsigned int j = surfaceProperty1->firstJ ( true ); j <= surfaceProperty1->lastJ ( true ); ++j ) {
         EXPECT_DOUBLE_EQ ( ValueToAdd, surfaceProperty4->get ( i, j ));
      }

   }

   EXPECT_EQ( true, surfaceProperty5 == 0 );
   EXPECT_EQ( true, surfaceProperty6 == 0 );

   delete snapshot;
   delete surface;
}

TEST ( AbstractPropertyManagerTest, VesTest )
{

   TestPropertyManager propertyManager;

   const DataModel::AbstractProperty* lithostaticPressure = propertyManager.getProperty ( "LithoStaticPressure" );
   const DataModel::AbstractProperty* pressure = propertyManager.getProperty ( "Pressure" );

   const DataModel::AbstractSnapshot*  snapshot = new MockSnapshot ( 0.0 );
   const DataModel::AbstractSurface*   surface = new MockSurface ( "TopSurface" );

   SurfacePropertyPtr lithostaticPressureSurfaceProperty = propertyManager.getSurfaceProperty ( lithostaticPressure, snapshot, surface );
   SurfacePropertyPtr pressureSurfaceProperty = propertyManager.getSurfaceProperty ( pressure, snapshot, surface );

   const DataModel::AbstractProperty* ves = propertyManager.getProperty ( "Ves" );
   SurfacePropertyPtr vesSurfaceProperty = propertyManager.getSurfaceProperty ( ves, snapshot, surface );

   EXPECT_EQ ( ves, vesSurfaceProperty->getProperty ());
   EXPECT_DOUBLE_EQ ( 52000000, vesSurfaceProperty->get ( 5, 7 ));
   EXPECT_DOUBLE_EQ ( 72000000, vesSurfaceProperty->get ( 7, 5 ));

   delete snapshot;
   delete surface;
}


TestPropertyManager::TestPropertyManager () {
   // These will come from the project handle.
   m_mockProperties.push_back ( new DataModel::MockProperty ( "LithoStaticPressure", DataModel::CONTINUOUS_3D_PROPERTY ) );
   m_mockProperties.push_back ( new DataModel::MockProperty ( "Pressure", DataModel::CONTINUOUS_3D_PROPERTY ) );
   m_mockProperties.push_back ( new DataModel::MockProperty ( "Ves", DataModel::CONTINUOUS_3D_PROPERTY ) );
   m_mockProperties.push_back ( new DataModel::MockProperty ( "Property2", DataModel::CONTINUOUS_3D_PROPERTY ) );
   m_mockProperties.push_back ( new DataModel::MockProperty ( "Property3", DataModel::CONTINUOUS_3D_PROPERTY ) );

   // Add all properties to the property manager.
   for ( size_t i = 0; i < m_mockProperties.size (); ++i ) {
      addProperty ( m_mockProperties [ i ]);
   }

   // This will come from the project handle.
   m_mapGrid = new DataModel::MockGrid ( 0, 0, 0, 0, 10, 10, 10, 10 );

   addSurfacePropertyCalculator ( SurfacePropertyCalculatorPtr ( new Property1Calculator )); 
   addSurfacePropertyCalculator ( SurfacePropertyCalculatorPtr ( new Property2Calculator ( ValueToAdd )));
   addSurfacePropertyCalculator ( SurfacePropertyCalculatorPtr ( new Property4Calculator )); 
   addSurfacePropertyCalculator ( SurfacePropertyCalculatorPtr ( new VesSurfaceCalculator ));  
}


TestPropertyManager::~TestPropertyManager () {

   for ( size_t i = 0; i < m_mockProperties.size (); ++i ) {
      delete m_mockProperties [ i ];
   }

   delete m_mapGrid;
}


void TestPropertyManager::addProperty ( const DataModel::AbstractProperty* property ) {
   
   if ( std::find ( m_properties.begin (), m_properties.end (), property ) == m_properties.end ()) {
      m_properties.push_back ( property );
   }

}


const DataModel::AbstractProperty* TestPropertyManager::getProperty ( const std::string& name ) const {

   for ( size_t i = 0; i < m_properties.size (); ++i ) {

      if ( m_properties [ i ]->getName () == name ) {
         return m_properties [ i ];
      }

   }

   return 0;
}

const DataModel::AbstractGrid* TestPropertyManager::getMapGrid () const {
   return m_mapGrid;
}

bool TestPropertyManager::getNodeIsValid ( const unsigned int i, const unsigned int j ) const { 
   (void) i;
   (void) j;
   return true; 
}

Property1Calculator::Property1Calculator () {
   addPropertyName ( "LithoStaticPressure" );
}

void Property1Calculator::calculate ( DerivedProperties::AbstractPropertyManager& propertyManager,
                                      const DataModel::AbstractSnapshot*        snapshot,
                                      const DataModel::AbstractSurface*         surface,
                                      SurfacePropertyList&                derivedProperties ) const {

   derivedProperties.clear ();

   if ( surface->getName () == "TopSurface" ) {
      const DataModel::AbstractProperty* property = propertyManager.getProperty ( "LithoStaticPressure" );

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

}

Property2Calculator::Property2Calculator ( const double value ) : m_value ( value ) {
   addPropertyName ( "Property2" );
   addPropertyName ( "Property3" );
}

void Property2Calculator::calculate ( DerivedProperties::AbstractPropertyManager& propertyManager,
                                      const DataModel::AbstractSnapshot*        snapshot,
                                      const DataModel::AbstractSurface*         surface,
                                      SurfacePropertyList&                derivedProperties ) const {

   const DataModel::AbstractProperty* property1 = propertyManager.getProperty ( "LithoStaticPressure" );
   const DataModel::AbstractProperty* property2 = propertyManager.getProperty ( "Property2" );
   const DataModel::AbstractProperty* property3 = propertyManager.getProperty ( "Property3" );

   const SurfacePropertyPtr prop1 = propertyManager.getSurfaceProperty ( property1, snapshot, surface );

   derivedProperties.clear ();

   if ( prop1 != 0 ) {
      DerivedSurfacePropertyPtr derivedProp2 = DerivedSurfacePropertyPtr( new DerivedProperties::DerivedSurfaceProperty ( property2, snapshot, surface, propertyManager.getMapGrid ()));
      DerivedSurfacePropertyPtr derivedProp3 = DerivedSurfacePropertyPtr( new DerivedProperties::DerivedSurfaceProperty ( property3, snapshot, surface, propertyManager.getMapGrid ()));

      for ( unsigned int i = derivedProp2->firstI ( true ); i <= derivedProp2->lastI ( true ); ++i ) {

         for ( unsigned int j = derivedProp2->firstJ ( true ); j <= derivedProp2->lastJ ( true ); ++j ) {
            derivedProp2->set ( i, j, prop1->get ( i, j ) + m_value );
            derivedProp3->set ( i, j, prop1->get ( i, j ) + m_value * m_value );
         }

      }

      derivedProperties.push_back ( derivedProp2 );
      derivedProperties.push_back ( derivedProp3 );
   }

}

Property4Calculator::Property4Calculator () {
   addPropertyName ( "Pressure" );
}

void Property4Calculator::calculate ( DerivedProperties::AbstractPropertyManager& propertyManager,
                                      const DataModel::AbstractSnapshot*          snapshot,
                                      const DataModel::AbstractSurface*           surface,
                                            SurfacePropertyList&                  derivedProperties ) const {

   const DataModel::AbstractProperty* property1 = propertyManager.getProperty ( "LithoStaticPressure" );
   const DataModel::AbstractProperty* property2 = propertyManager.getProperty ( "Property2" );
   const DataModel::AbstractProperty* property4 = propertyManager.getProperty ( "Pressure" );

   const SurfacePropertyPtr prop2 = propertyManager.getSurfaceProperty ( property2, snapshot, surface );
   const SurfacePropertyPtr prop1 = propertyManager.getSurfaceProperty ( property1, snapshot, surface );

   derivedProperties.clear ();

   if ( prop1 != 0 and prop2 != 0 ) {
      DerivedSurfacePropertyPtr derivedProp = DerivedSurfacePropertyPtr( new DerivedProperties::DerivedSurfaceProperty ( property4, snapshot, surface, propertyManager.getMapGrid ()));

      for ( unsigned int i = derivedProp->firstI ( true ); i <= derivedProp->lastI ( true ); ++i ) {

         for ( unsigned int j = derivedProp->firstJ ( true ); j <= derivedProp->lastJ ( true ); ++j ) {
            derivedProp->set ( i, j, prop2->get ( i, j ) - prop1->get ( i, j ));
         }

      }

      derivedProperties.push_back ( derivedProp );
   }

}


