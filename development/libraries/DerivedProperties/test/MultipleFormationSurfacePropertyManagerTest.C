#include <iostream>
#include <vector>
#include <string>
#include "../src/AbstractPropertyManager.h"
#include "../src/FormationSurfacePropertyCalculator.h"
#include "../src/DerivedFormationSurfaceProperty.h"

#include "MockSurface.h"
#include "MockFormation.h"
#include "MockSnapshot.h"
#include "MockProperty.h"
#include "MockGrid.h"

#include <gtest/gtest.h>

using namespace DataModel;
using namespace DerivedProperties;

static const double ValueToAdd = 10.0;

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


class Property1Calculator : public DerivedProperties::FormationSurfacePropertyCalculator {

public :

   Property1Calculator ();

   void calculate ( DerivedProperties::AbstractPropertyManager& propertyManager,
                    const DataModel::AbstractSnapshot*          snapshot,
                    const DataModel::AbstractFormation*         formation,
                    const DataModel::AbstractSurface*           surface,
                          FormationSurfacePropertyList&         derivedProperties ) const;

};

class Property2Calculator : public DerivedProperties::FormationSurfacePropertyCalculator {

public :

   Property2Calculator ( const double value );

   void calculate ( DerivedProperties::AbstractPropertyManager& propertyManager,
                    const DataModel::AbstractSnapshot*          snapshot,
                    const DataModel::AbstractFormation*         formation,
                    const DataModel::AbstractSurface*           surface,
                    FormationSurfacePropertyList&               derivedProperties ) const;

private :

   double m_value;

};

class Property4Calculator : public DerivedProperties::FormationSurfacePropertyCalculator {

public :

   Property4Calculator ();

   void calculate ( DerivedProperties::AbstractPropertyManager& propertyManager,
                    const DataModel::AbstractSnapshot*          snapshot,
                    const DataModel::AbstractFormation*         formation,
                    const DataModel::AbstractSurface*           surface,
                    FormationSurfacePropertyList&               derivedProperties ) const;

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

   const DataModel::AbstractProperty* property1 = propertyManager.getProperty ( "Property1" );
   const DataModel::AbstractProperty* property2 = propertyManager.getProperty ( "Property2" );
   const DataModel::AbstractProperty* property3 = propertyManager.getProperty ( "Property3" );
   const DataModel::AbstractProperty* property4 = propertyManager.getProperty ( "Property4" );

   const DataModel::AbstractSnapshot*  snapshot = new MockSnapshot ( 0.0 );
   const DataModel::AbstractSnapshot*  anotherSnapshot = new MockSnapshot ( 10.0 );
   const DataModel::AbstractSurface*   surface = new MockSurface ( "TopSurface" );
   const DataModel::AbstractFormation* formation = new MockFormation ( "Formation1" );

   const DataModel::AbstractSurface*   bottomSurface = new MockSurface ( "BottomSurface" );


   FormationSurfacePropertyPtr surfaceProperty4 = propertyManager.getFormationSurfaceProperty ( property4, snapshot, formation, surface );
   FormationSurfacePropertyPtr surfaceProperty2 = propertyManager.getFormationSurfaceProperty ( property2, snapshot, formation, surface );
   FormationSurfacePropertyPtr surfaceProperty1 = propertyManager.getFormationSurfaceProperty ( property1, snapshot, formation, surface );
   FormationSurfacePropertyPtr surfaceProperty3 = propertyManager.getFormationSurfaceProperty ( property3, snapshot, formation, surface );

   // These following 2 surface properties should be null after the get surface property call.
   FormationSurfacePropertyPtr surfaceProperty5 = propertyManager.getFormationSurfaceProperty ( property3, snapshot, formation, bottomSurface );
   FormationSurfacePropertyPtr surfaceProperty6 = propertyManager.getFormationSurfaceProperty ( property3, anotherSnapshot, formation, bottomSurface );


   EXPECT_EQ ( property4, surfaceProperty4->getProperty ());
   EXPECT_EQ ( surface, surfaceProperty4->getSurface ());
   EXPECT_EQ ( snapshot, surfaceProperty4->getSnapshot ());

   for ( unsigned int i = surfaceProperty1->firstI ( true ); i <= surfaceProperty1->lastI ( true ); ++i ) {

      for ( unsigned int j = surfaceProperty1->firstJ ( true ); j <= surfaceProperty1->lastJ ( true ); ++j ) {
         EXPECT_DOUBLE_EQ ( ValueToAdd, surfaceProperty4->get ( i, j ));
      }

   }

   EXPECT_EQ ( true, surfaceProperty5 == 0 );
   EXPECT_EQ ( true, surfaceProperty6 == 0 );

   delete snapshot;
   delete surface;
   delete formation;
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

   // This will come from the project handle.
   m_mapGrid = new DataModel::MockGrid ( 0, 0, 0, 0, 10, 10, 10, 10 );

   addFormationSurfacePropertyCalculator ( FormationSurfacePropertyCalculatorPtr ( new Property1Calculator ));
   addFormationSurfacePropertyCalculator ( FormationSurfacePropertyCalculatorPtr ( new Property2Calculator ( ValueToAdd )));
   addFormationSurfacePropertyCalculator ( FormationSurfacePropertyCalculatorPtr ( new Property4Calculator ));
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


bool TestPropertyManager::getNodeIsValid ( const unsigned int i, const unsigned int j ) const { 
   (void) i;
   (void) j;
   return true; 
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

Property1Calculator::Property1Calculator () {
   addPropertyName ( "Property1" );
}

void Property1Calculator::calculate ( DerivedProperties::AbstractPropertyManager& propertyManager,
                                      const DataModel::AbstractSnapshot*          snapshot,
                                      const DataModel::AbstractFormation*         formation,
                                      const DataModel::AbstractSurface*           surface,
                                            FormationSurfacePropertyList&         derivedProperties ) const {

   derivedProperties.clear ();

   if ( surface->getName () == "TopSurface" ) {
      const DataModel::AbstractProperty* property = propertyManager.getProperty ( "Property1" );

      DerivedFormationSurfacePropertyPtr derivedProp = DerivedFormationSurfacePropertyPtr ( new DerivedProperties::DerivedFormationSurfaceProperty ( property, snapshot, formation, surface, propertyManager.getMapGrid ()));
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
                                      const DataModel::AbstractSnapshot*          snapshot,
                                      const DataModel::AbstractFormation*         formation,
                                      const DataModel::AbstractSurface*           surface,
                                            FormationSurfacePropertyList&         derivedProperties ) const {

   const DataModel::AbstractProperty* property1 = propertyManager.getProperty ( "Property1" );
   const DataModel::AbstractProperty* property2 = propertyManager.getProperty ( "Property2" );
   const DataModel::AbstractProperty* property3 = propertyManager.getProperty ( "Property3" );

   const FormationSurfacePropertyPtr prop1 = propertyManager.getFormationSurfaceProperty ( property1, snapshot, formation, surface );

   derivedProperties.clear ();

   if ( prop1 != 0 ) {
      DerivedFormationSurfacePropertyPtr derivedProp2 = DerivedFormationSurfacePropertyPtr( new DerivedProperties::DerivedFormationSurfaceProperty ( property2, snapshot, formation, surface, propertyManager.getMapGrid ()));
      DerivedFormationSurfacePropertyPtr derivedProp3 = DerivedFormationSurfacePropertyPtr( new DerivedProperties::DerivedFormationSurfaceProperty ( property3, snapshot, formation, surface, propertyManager.getMapGrid ()));

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
   addPropertyName ( "Property4" );
}

void Property4Calculator::calculate ( DerivedProperties::AbstractPropertyManager& propertyManager,
                                      const DataModel::AbstractSnapshot*          snapshot,
                                      const DataModel::AbstractFormation*         formation,
                                      const DataModel::AbstractSurface*           surface,
                                            FormationSurfacePropertyList&         derivedProperties ) const {

   const DataModel::AbstractProperty* property1 = propertyManager.getProperty ( "Property1" );
   const DataModel::AbstractProperty* property2 = propertyManager.getProperty ( "Property2" );
   const DataModel::AbstractProperty* property4 = propertyManager.getProperty ( "Property4" );

   const FormationSurfacePropertyPtr prop2 = propertyManager.getFormationSurfaceProperty ( property2, snapshot, formation, surface );
   const FormationSurfacePropertyPtr prop1 = propertyManager.getFormationSurfaceProperty ( property1, snapshot, formation, surface );

   derivedProperties.clear ();

   if ( prop1 != 0 and prop2 != 0 ) {
      DerivedFormationSurfacePropertyPtr derivedProp = DerivedFormationSurfacePropertyPtr( new DerivedProperties::DerivedFormationSurfaceProperty ( property4, snapshot, formation, surface, propertyManager.getMapGrid ()));

      for ( unsigned int i = derivedProp->firstI ( true ); i <= derivedProp->lastI ( true ); ++i ) {

         for ( unsigned int j = derivedProp->firstJ ( true ); j <= derivedProp->lastJ ( true ); ++j ) {
            derivedProp->set ( i, j, prop2->get ( i, j ) - prop1->get ( i, j ));
         }

      }

      derivedProperties.push_back ( derivedProp );
   }

}


