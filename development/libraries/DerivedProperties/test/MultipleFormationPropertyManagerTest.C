#include <iostream>
#include <vector>
#include <string>
#include "../src/AbstractPropertyManager.h"
#include "../src/FormationPropertyCalculator.h"
#include "../src/DerivedFormationProperty.h"

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


class Property1Calculator : public DerivedProperties::FormationPropertyCalculator {

public :

   Property1Calculator ();

   void calculate ( DerivedProperties::AbstractPropertyManager& propertyManager,
                    const DataModel::AbstractSnapshot*          snapshot,
                    const DataModel::AbstractFormation*         formation,
                          FormationPropertyList&                derivedProperties ) const;

};

class Property2Calculator : public DerivedProperties::FormationPropertyCalculator {

public :

   Property2Calculator ( const double value );

   void calculate ( DerivedProperties::AbstractPropertyManager& propertyManager,
                    const DataModel::AbstractSnapshot*          snapshot,
                    const DataModel::AbstractFormation*         formation,
                          FormationPropertyList&                derivedProperties ) const;

private :

   double m_value;

};

class Property4Calculator : public DerivedProperties::FormationPropertyCalculator {

public :

   Property4Calculator ();

   void calculate ( DerivedProperties::AbstractPropertyManager& propertyManager,
                    const DataModel::AbstractSnapshot*          snapshot,
                    const DataModel::AbstractFormation*         formation,
                          FormationPropertyList&                derivedProperties ) const;

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
   const DataModel::AbstractFormation* formation2 = new MockFormation ( "Formation2" );


   FormationPropertyPtr formationProperty4 = propertyManager.getFormationProperty ( property4, snapshot, formation );
   FormationPropertyPtr formationProperty2 = propertyManager.getFormationProperty ( property2, snapshot, formation );
   FormationPropertyPtr formationProperty1 = propertyManager.getFormationProperty ( property1, snapshot, formation );
   FormationPropertyPtr formationProperty3 = propertyManager.getFormationProperty ( property3, snapshot, formation );

   // These following 2 formation properties should be null after the get formation property call.
   FormationPropertyPtr formationProperty5 = propertyManager.getFormationProperty ( property3, snapshot, formation2 );
   FormationPropertyPtr formationProperty6 = propertyManager.getFormationProperty ( property3, anotherSnapshot, formation2 );


   EXPECT_EQ ( property4, formationProperty4->getProperty ());
   EXPECT_EQ ( formation, formationProperty4->getFormation ());
   EXPECT_EQ ( snapshot, formationProperty4->getSnapshot ());

   for ( unsigned int i = formationProperty1->firstI ( true ); i <= formationProperty1->lastI ( true ); ++i ) {

      for ( unsigned int j = formationProperty1->firstJ ( true ); j <= formationProperty1->lastJ ( true ); ++j ) {

         for ( unsigned int k = formationProperty1->firstK (); k <= formationProperty1->lastK (); ++k ) {
            EXPECT_DOUBLE_EQ ( ValueToAdd, formationProperty4->get ( i, j, k ));
         }

      }

   }

   EXPECT_EQ ( true, formationProperty5 == 0 );
   EXPECT_EQ ( true, formationProperty6 == 0 );

   delete snapshot;
   delete anotherSnapshot;
   delete surface;
   delete formation;
   delete formation2;
}


TestPropertyManager::TestPropertyManager () {
   // These will come from the project handle.
   m_mockProperties.push_back ( new DataModel::MockProperty ( "Property1", DataModel::CONTINUOUS_3D_PROPERTY ) );
   m_mockProperties.push_back ( new DataModel::MockProperty ( "Property2", DataModel::CONTINUOUS_3D_PROPERTY ) );
   m_mockProperties.push_back ( new DataModel::MockProperty ( "Property3", DataModel::CONTINUOUS_3D_PROPERTY ) );
   m_mockProperties.push_back ( new DataModel::MockProperty ( "Property4", DataModel::CONTINUOUS_3D_PROPERTY ) );

   // Add all properties to the property manager.
   for ( size_t i = 0; i < m_mockProperties.size (); ++i ) {
      addProperty ( m_mockProperties [ i ]);
   }

   // This will come from the project handle.
   m_mapGrid = new DataModel::MockGrid ( 0, 0, 0, 0, 10, 10, 10, 10 );

   addFormationPropertyCalculator ( FormationPropertyCalculatorPtr ( new Property1Calculator ));
   addFormationPropertyCalculator ( FormationPropertyCalculatorPtr ( new Property2Calculator ( ValueToAdd )));
   addFormationPropertyCalculator ( FormationPropertyCalculatorPtr ( new Property4Calculator ));
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
                                            FormationPropertyList&                derivedProperties ) const {

   derivedProperties.clear ();

   if ( formation->getName () == "Formation1" ) {
      const DataModel::AbstractProperty* property = propertyManager.getProperty ( "Property1" );

      DerivedFormationPropertyPtr derivedProp = DerivedFormationPropertyPtr ( new DerivedProperties::DerivedFormationProperty ( property, snapshot, formation, propertyManager.getMapGrid (), 10 ));
      double value = 0.0;

      for ( unsigned int i = derivedProp->firstI ( true ); i <= derivedProp->lastI ( true ); ++i ) {

         for ( unsigned int j = derivedProp->firstJ ( true ); j <= derivedProp->lastJ ( true ); ++j ) {

            for ( unsigned int k = derivedProp->firstK (); k <= derivedProp->lastK (); ++k ) {
               derivedProp->set ( i, j, k, value );
               value += 1.0;
            }

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
                                            FormationPropertyList&                derivedProperties ) const {

   const DataModel::AbstractProperty* property1 = propertyManager.getProperty ( "Property1" );
   const DataModel::AbstractProperty* property2 = propertyManager.getProperty ( "Property2" );
   const DataModel::AbstractProperty* property3 = propertyManager.getProperty ( "Property3" );

   const FormationPropertyPtr prop1 = propertyManager.getFormationProperty ( property1, snapshot, formation );

   derivedProperties.clear ();

   if ( prop1 != 0 ) {
      DerivedFormationPropertyPtr derivedProp2 = DerivedFormationPropertyPtr( new DerivedProperties::DerivedFormationProperty ( property2, snapshot, formation, propertyManager.getMapGrid (), 10 ));
      DerivedFormationPropertyPtr derivedProp3 = DerivedFormationPropertyPtr( new DerivedProperties::DerivedFormationProperty ( property3, snapshot, formation, propertyManager.getMapGrid (), 10 ));

      for ( unsigned int i = derivedProp2->firstI ( true ); i <= derivedProp2->lastI ( true ); ++i ) {

         for ( unsigned int j = derivedProp2->firstJ ( true ); j <= derivedProp2->lastJ ( true ); ++j ) {

            for ( unsigned int k = derivedProp2->firstK (); k <= derivedProp2->lastK (); ++k ) {
               derivedProp2->set ( i, j, k, prop1->get ( i, j, k ) + m_value );
               derivedProp3->set ( i, j, k, prop1->get ( i, j, k ) + m_value * m_value );
            }

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
                                            FormationPropertyList&                derivedProperties ) const {

   const DataModel::AbstractProperty* property1 = propertyManager.getProperty ( "Property1" );
   const DataModel::AbstractProperty* property2 = propertyManager.getProperty ( "Property2" );
   const DataModel::AbstractProperty* property4 = propertyManager.getProperty ( "Property4" );

   const FormationPropertyPtr prop2 = propertyManager.getFormationProperty ( property2, snapshot, formation );
   const FormationPropertyPtr prop1 = propertyManager.getFormationProperty ( property1, snapshot, formation );

   derivedProperties.clear ();

   if ( prop1 != 0 and prop2 != 0 ) {
      DerivedFormationPropertyPtr derivedProp = DerivedFormationPropertyPtr( new DerivedProperties::DerivedFormationProperty ( property4, snapshot, formation, propertyManager.getMapGrid (), 10 ));

      for ( unsigned int i = derivedProp->firstI ( true ); i <= derivedProp->lastI ( true ); ++i ) {

         for ( unsigned int j = derivedProp->firstJ ( true ); j <= derivedProp->lastJ ( true ); ++j ) {

            for ( unsigned int k = derivedProp->firstK (); k <= derivedProp->lastK (); ++k ) {
               derivedProp->set ( i, j, k, prop2->get ( i, j, k ) - prop1->get ( i, j, k ));
            }

         }

      }

      derivedProperties.push_back ( derivedProp );
   }

}


