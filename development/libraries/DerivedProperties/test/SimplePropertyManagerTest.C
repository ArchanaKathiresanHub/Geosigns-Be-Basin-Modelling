#include <iostream>
#include <vector>
#include <string>
#include "../src/DerivedPropertyManager.h"
#include "../src/SurfacePropertyCalculator.h"
#include "../src/DerivedSurfaceProperty.h"
#include "../src/DerivedFormationProperty.h"
#include "../src/DerivedFormationSurfaceProperty.h"

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

   Property1Calculator ();

   void calculate ( DerivedProperties::DerivedPropertyManager& propertyManager,
                    const DataModel::AbstractSnapshot*        snapshot,
                    const DataModel::AbstractSurface*         surface,
                          SurfacePropertyList&                derivedProperties ) const;

   const std::vector<std::string>& getPropertyNames () const;

private :

   std::vector<std::string> m_propertyNames;

};

class FormationProperty1Calculator : public DerivedProperties::FormationPropertyCalculator {

public :

   FormationProperty1Calculator ();

   void calculate ( DerivedProperties::DerivedPropertyManager& propertyManager,
                    const DataModel::AbstractSnapshot*        snapshot,
                    const DataModel::AbstractFormation*       formation,
                          FormationPropertyList&              derivedProperties ) const;

   const std::vector<std::string>& getPropertyNames () const;

private :

   std::vector<std::string> m_propertyNames;

};
class FormationSurfaceProperty1Calculator : public DerivedProperties::FormationSurfacePropertyCalculator {

public :

   FormationSurfaceProperty1Calculator ();

   void calculate ( DerivedProperties::DerivedPropertyManager& propertyManager,
                    const DataModel::AbstractSnapshot*        snapshot,
                    const DataModel::AbstractFormation*       formation,
                    const DataModel::AbstractSurface*         surface,
                          FormationSurfacePropertyList&       derivedProperties ) const;

   const std::vector<std::string>& getPropertyNames () const;

private :

   std::vector<std::string> m_propertyNames;

};

// Tests whether properties are retrieved correctly from the property-manager.
//
// Tests whether surface property values are retrieved correctly from the property-manager.
TEST ( DerivedPropertyManagerTest,  Test1 )
{
   TestPropertyManager propertyManager;

   const DataModel::AbstractProperty* property = propertyManager.getProperty ( "Property1" );

   const DataModel::AbstractSnapshot*  snapshot = new MockSnapshot ( 0.0 );
   const DataModel::AbstractSurface*   surface = new MockSurface ( "TopSurface" );

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

   delete snapshot;
   delete surface;
}

TEST ( DerivedPropertyManagerTest,  Test2 )
{
   TestPropertyManager propertyManager;

   const DataModel::AbstractProperty* property = propertyManager.getProperty ( "Property1" );

   const DataModel::AbstractSnapshot*  snapshot = new MockSnapshot ( 0.0 );
   const DataModel::AbstractFormation* formation = new MockFormation ( "Formation1" );

   FormationPropertyPtr formationProperty1 = propertyManager.getFormationProperty ( property, snapshot, formation );

   double value = 0.0;

   EXPECT_EQ ( property->getName (), "Property1" );
   EXPECT_EQ ( property, formationProperty1->getProperty ());
   EXPECT_EQ ( formation, formationProperty1->getFormation ());
   EXPECT_EQ ( snapshot,  formationProperty1->getSnapshot ());

   for ( unsigned int i = formationProperty1->firstI ( true ); i <= formationProperty1->lastI ( true ); ++i ) {

      for ( unsigned int j = formationProperty1->firstJ ( true ); j <= formationProperty1->lastJ ( true ); ++j ) {

         for ( unsigned int k = formationProperty1->firstK (); k <= formationProperty1->lastK (); ++ k ) {
            EXPECT_DOUBLE_EQ ( value, formationProperty1->get ( i, j, k ));
            value += 1.0;
         }
      }
   }

   delete snapshot;
   delete formation;
}


TEST ( DerivedPropertyManagerTest,  Test3 )
{
   TestPropertyManager propertyManager;

   const DataModel::AbstractProperty* property = propertyManager.getProperty ( "Property1" );

   const DataModel::AbstractSnapshot*  snapshot = new MockSnapshot ( 0.0 );
   const DataModel::AbstractFormation* formation = new MockFormation ( "Formation1" );
   const DataModel::AbstractSurface*   surface   = new MockSurface ( "Top Sutrface" );

   FormationSurfacePropertyPtr fsProperty1 = propertyManager.getFormationSurfaceProperty ( property, snapshot, formation, surface );

   double value = 0.0;

   EXPECT_EQ ( property->getName (), "Property1" );
   EXPECT_EQ ( property,  fsProperty1->getProperty ());
   EXPECT_EQ ( formation, fsProperty1->getFormation ());
   EXPECT_EQ ( snapshot,  fsProperty1->getSnapshot ());
   EXPECT_EQ ( surface,   fsProperty1->getSurface ());

   for ( unsigned int i = fsProperty1->firstI ( true ); i <= fsProperty1->lastI ( true ); ++i ) {

      for ( unsigned int j = fsProperty1->firstJ ( true ); j <= fsProperty1->lastJ ( true ); ++j ) {
         
         EXPECT_DOUBLE_EQ ( value, fsProperty1->get ( i, j ));
         value += 1.0;
      }
   }

   delete snapshot;
   delete formation;
   delete surface;
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

   addCalculator ( SurfacePropertyCalculatorPtr ( new Property1Calculator ));
   addCalculator ( FormationPropertyCalculatorPtr ( new FormationProperty1Calculator ));
   addCalculator ( FormationSurfacePropertyCalculatorPtr ( new FormationSurfaceProperty1Calculator ));
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

Property1Calculator::Property1Calculator () {
   m_propertyNames.push_back ( "Property1" );
}

const std::vector<std::string>& Property1Calculator::getPropertyNames () const {
   return m_propertyNames;
}

void Property1Calculator::calculate ( DerivedProperties::DerivedPropertyManager& propertyManager,
                                      const DataModel::AbstractSnapshot*        snapshot,
                                      const DataModel::AbstractSurface*         surface,
                                      SurfacePropertyList&                derivedProperties ) const {

   const DataModel::AbstractProperty* property = propertyManager.getProperty ( "Property1" );

   DerivedSurfacePropertyPtr derivedProp = DerivedSurfacePropertyPtr ( new DerivedProperties::DerivedSurfaceProperty ( property, snapshot, surface, propertyManager.getMapGrid ()));
   double value = 0.0;

   derivedProperties.clear ();

   for ( unsigned int i = derivedProp->firstI ( true ); i <= derivedProp->lastI ( true ); ++i ) {

      for ( unsigned int j = derivedProp->firstJ ( true ); j <= derivedProp->lastJ ( true ); ++j ) {
         derivedProp->set ( i, j, value );
         value += 1.0;
      }

   }

   derivedProperties.push_back ( derivedProp );
}


FormationProperty1Calculator::FormationProperty1Calculator () {
   m_propertyNames.push_back ( "Property1" );
}

const std::vector<std::string>& FormationProperty1Calculator::getPropertyNames () const {
   return m_propertyNames;
}

void FormationProperty1Calculator::calculate ( DerivedProperties::DerivedPropertyManager& propertyManager,
                                               const DataModel::AbstractSnapshot*         snapshot,
                                               const DataModel::AbstractFormation*        formation,
                                               FormationPropertyList&                derivedProperties ) const {
   
   const DataModel::AbstractProperty* property = propertyManager.getProperty ( "Property1" );

   DerivedFormationPropertyPtr derivedProp = DerivedFormationPropertyPtr ( new DerivedProperties::DerivedFormationProperty ( property, snapshot, formation, propertyManager.getMapGrid (), 10 ));
   double value = 0.0; 

   derivedProperties.clear ();

   for ( unsigned int i = derivedProp->firstI ( true ); i <= derivedProp->lastI ( true ); ++i ) {

      for ( unsigned int j = derivedProp->firstJ ( true ); j <= derivedProp->lastJ ( true ); ++j ) {

         for ( unsigned int k = derivedProp->firstK (); k <= derivedProp->lastK (); ++ k ) {
            derivedProp->set ( i, j, k, value );
            value += 1.0;
         }
      }
   }

   derivedProperties.push_back ( derivedProp );
}


FormationSurfaceProperty1Calculator::FormationSurfaceProperty1Calculator () {
   m_propertyNames.push_back ( "Property1" );
}

const std::vector<std::string>& FormationSurfaceProperty1Calculator::getPropertyNames () const {
   return m_propertyNames;
}

void FormationSurfaceProperty1Calculator::calculate ( DerivedProperties::DerivedPropertyManager& propertyManager,
                                                      const DataModel::AbstractSnapshot*         snapshot,
                                                      const DataModel::AbstractFormation*        formation,
                                                      const DataModel::AbstractSurface*          surface,
                                                            FormationSurfacePropertyList&        derivedProperties ) const {
   
   const DataModel::AbstractProperty* property = propertyManager.getProperty ( "Property1" );

   DerivedFormationSurfacePropertyPtr derivedProp = DerivedFormationSurfacePropertyPtr ( new DerivedProperties::DerivedFormationSurfaceProperty ( property, snapshot, formation, surface, propertyManager.getMapGrid () ));

   double value = 0.0; 

   derivedProperties.clear ();

   for ( unsigned int i = derivedProp->firstI ( true ); i <= derivedProp->lastI ( true ); ++i ) {

      for ( unsigned int j = derivedProp->firstJ ( true ); j <= derivedProp->lastJ ( true ); ++j ) {
         
         derivedProp->set ( i, j, value );
         value += 1.0;
      }
   }

   derivedProperties.push_back ( derivedProp );
}


