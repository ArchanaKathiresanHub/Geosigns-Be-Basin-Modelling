#include <iostream>
#include <vector>
#include <string>
#include "../src/AbstractPropertyManager.h"
#include "../src/AbstractPropertyValues.h"
#include "../src/SurfacePropertyCalculator.h"
#include "../src/DerivedSurfaceProperty.h"
#include "../src/DerivedFormationMapProperty.h"
#include "../src/DerivedFormationProperty.h"
#include "../src/DerivedFormationSurfaceProperty.h"

#include "MockSurface.h"
#include "MockFormation.h"
#include "MockSnapshot.h"
#include "MockProperty.h"
#include "MockGrid.h"

#include "../src/PropertyRetriever.h"

#include <gtest/gtest.h>

using namespace DataModel;
using namespace DerivedProperties;

static const double UndefinedValue = 99999.0;

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

class TestFormationProperty : public DerivedProperties::DerivedFormationProperty {

public :

   TestFormationProperty ( const DataModel::AbstractProperty*  property,
                           const DataModel::AbstractSnapshot*  snapshot,
                           const DataModel::AbstractFormation* formation,
                           const DataModel::AbstractGrid*      grid,
                           const unsigned int                  nk );

   /// \brief Get the value of the property at the position i,j,k.
   virtual double get ( unsigned int i,
                        unsigned int j,
                        unsigned int k ) const;

   /// \brief Get the undefined value.
   virtual double getUndefinedValue () const;

   /// \brief Determine if the map has been retrieved or not.
   virtual bool isRetrieved () const;

   /// \brief Retreive the grid map
   virtual void retrieveData () const;

   /// \brief Restore the grid map.
   virtual void restoreData () const;


private :

   mutable bool m_hasBeenRetrieved;

};


class Property1Calculator : public DerivedProperties::SurfacePropertyCalculator {

public :

   Property1Calculator ();

   void calculate ( DerivedProperties::AbstractPropertyManager& propertyManager,
                    const DataModel::AbstractSnapshot*          snapshot,
                    const DataModel::AbstractSurface*           surface,
                          SurfacePropertyList&                  derivedProperties ) const;

};

class FormationProperty1Calculator : public DerivedProperties::FormationPropertyCalculator {

public :

   FormationProperty1Calculator ();

   void calculate ( DerivedProperties::AbstractPropertyManager& propertyManager,
                    const DataModel::AbstractSnapshot*          snapshot,
                    const DataModel::AbstractFormation*         formation,
                          FormationPropertyList&                derivedProperties ) const;

};


class FormationMapProperty1Calculator : public DerivedProperties::FormationMapPropertyCalculator {

public :

   FormationMapProperty1Calculator ();

   void calculate ( DerivedProperties::AbstractPropertyManager& propertyManager,
                    const DataModel::AbstractSnapshot*          snapshot,
                    const DataModel::AbstractFormation*         formation,
                          FormationMapPropertyList&             derivedProperties ) const;

};

class FormationSurfaceProperty1Calculator : public DerivedProperties::FormationSurfacePropertyCalculator {

public :

   FormationSurfaceProperty1Calculator ();

   void calculate ( DerivedProperties::AbstractPropertyManager& propertyManager,
                    const DataModel::AbstractSnapshot*          snapshot,
                    const DataModel::AbstractFormation*         formation,
                    const DataModel::AbstractSurface*           surface,
                          FormationSurfacePropertyList&         derivedProperties ) const;

};

// Tests whether properties are retrieved correctly from the property-manager.
//
// Tests whether surface property values are retrieved correctly from the property-manager.
TEST ( AbstractPropertyManagerTest,  Test1 )
{
   TestPropertyManager propertyManager;

   const DataModel::AbstractProperty* property = propertyManager.getProperty ( "Property1" );

   const DataModel::AbstractSnapshot*  snapshot     = new MockSnapshot ( 0.0 );
   const DataModel::AbstractFormation* topFormation = new MockFormation ( "TopFormation", "TopSurface", "MiddleSurface" );
   const DataModel::AbstractFormation* botFormation = new MockFormation ( "BottomFormation", "MiddleSurface", "BottomSurface" );
   const DataModel::AbstractSurface*   surface      = new MockSurface ( "MiddleSurface", topFormation, botFormation );

   SurfacePropertyPtr surfaceProperty1 = propertyManager.getSurfaceProperty ( property, snapshot, surface );
   PropertyRetriever propRet ( surfaceProperty1 );

   double value = 1089.0;

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

TEST ( AbstractPropertyManagerTest,  Test2 )
{
   TestPropertyManager propertyManager;

   const DataModel::AbstractProperty* property = propertyManager.getProperty ( "Property1" );

   const DataModel::AbstractSnapshot*  snapshot = new MockSnapshot ( 0.0 );
   const DataModel::AbstractFormation* formation = new MockFormation ( "Formation1" );

   FormationPropertyPtr formationProperty1 = propertyManager.getFormationProperty ( property, snapshot, formation );
   PropertyRetriever propRet ( formationProperty1 );

   double value = 0.0;

   EXPECT_EQ ( property->getName (), "Property1" );
   EXPECT_EQ ( property, formationProperty1->getProperty ());
   EXPECT_EQ ( formation, formationProperty1->getFormation ());
   EXPECT_EQ ( snapshot,  formationProperty1->getSnapshot ());

   for ( unsigned int k = formationProperty1->firstK (); k <= formationProperty1->lastK (); ++ k ) {

      for ( unsigned int i = formationProperty1->firstI ( true ); i <= formationProperty1->lastI ( true ); ++i ) {

         for ( unsigned int j = formationProperty1->firstJ ( true ); j <= formationProperty1->lastJ ( true ); ++j ) {
            EXPECT_DOUBLE_EQ ( value, formationProperty1->get ( i, j, k ));
            value += 1.0;
         }
      }
   }

   delete snapshot;
   delete formation;
}


TEST ( AbstractPropertyManagerTest,  Test3 )
{
   TestPropertyManager propertyManager;

   const DataModel::AbstractProperty* property = propertyManager.getProperty ( "Property3" );

   const DataModel::AbstractSnapshot*  snapshot = new MockSnapshot ( 0.0 );
   const DataModel::AbstractFormation* formation = new MockFormation ( "Formation1" );
   const DataModel::AbstractSurface*   surface   = new MockSurface ( "Top Sutrface" );

   FormationMapPropertyPtr fmProperty1 = propertyManager.getFormationMapProperty ( property, snapshot, formation );
   PropertyRetriever propRet ( fmProperty1 );

   double value = 0.0;

   EXPECT_EQ ( property->getName (), "Property3" );
   EXPECT_EQ ( property,  fmProperty1->getProperty ());
   EXPECT_EQ ( formation, fmProperty1->getFormation ());
   EXPECT_EQ ( snapshot,  fmProperty1->getSnapshot ());

   for ( unsigned int i = fmProperty1->firstI ( true ); i <= fmProperty1->lastI ( true ); ++i ) {

      for ( unsigned int j = fmProperty1->firstJ ( true ); j <= fmProperty1->lastJ ( true ); ++j ) {
         
         EXPECT_DOUBLE_EQ ( value, fmProperty1->get ( i, j ));
         value += 1.0;
      }
   }

   delete snapshot;
   delete formation;
   delete surface;
}


TEST ( AbstractPropertyManagerTest,  Test4 )
{
   TestPropertyManager propertyManager;

   const DataModel::AbstractProperty* property = propertyManager.getProperty ( "Property2" );

   const DataModel::AbstractSnapshot*  snapshot = new MockSnapshot ( 0.0 );
   const DataModel::AbstractFormation* formation = new MockFormation ( "Formation1" );
   const DataModel::AbstractSurface*   surface   = new MockSurface ( "Top Sutrface" );

   FormationSurfacePropertyPtr fsProperty1 = propertyManager.getFormationSurfaceProperty ( property, snapshot, formation, surface );
   PropertyRetriever propRet ( fsProperty1 );

   double value = 0.0;

   EXPECT_EQ ( property->getName (), "Property2" );
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
   m_mockProperties.push_back ( new DataModel::MockProperty ( "Property1", DataModel::CONTINUOUS_3D_PROPERTY ) );
   m_mockProperties.push_back ( new DataModel::MockProperty ( "Property2", DataModel::DISCONTINUOUS_3D_PROPERTY ) );
   m_mockProperties.push_back ( new DataModel::MockProperty ( "Property3", DataModel::FORMATION_2D_PROPERTY ) );

   // Add all properties to the property manager.
   for ( size_t i = 0; i < m_mockProperties.size (); ++i ) {
      addProperty ( m_mockProperties [ i ]);
   }

   // This will come frmo the project handle.
   m_mapGrid = new DataModel::MockGrid ( 0, 0, 0, 0, 10, 10, 10, 10 );

   addSurfacePropertyCalculator ( SurfacePropertyCalculatorPtr ( new Property1Calculator ));
   addFormationMapPropertyCalculator ( FormationMapPropertyCalculatorPtr ( new FormationMapProperty1Calculator ));
   addFormationPropertyCalculator ( FormationPropertyCalculatorPtr ( new FormationProperty1Calculator ));
   addFormationSurfacePropertyCalculator ( FormationSurfacePropertyCalculatorPtr ( new FormationSurfaceProperty1Calculator ));
}


TestPropertyManager::~TestPropertyManager () {

   for ( size_t i = 0; i < m_mockProperties.size (); ++i ) {
      delete m_mockProperties [ i ];
   }

   delete m_mapGrid;
}

bool TestPropertyManager::getNodeIsValid ( const unsigned int i, const unsigned int j ) const { 
   (void ) i;
   (void ) j;
   return true; 
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

Property1Calculator::Property1Calculator () {
   addPropertyName ( "Property1" );
}

void Property1Calculator::calculate ( DerivedProperties::AbstractPropertyManager& propertyManager,
                                      const DataModel::AbstractSnapshot*          snapshot,
                                      const DataModel::AbstractSurface*           surface,
                                            SurfacePropertyList&                  derivedProperties ) const {

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



FormationMapProperty1Calculator::FormationMapProperty1Calculator () {
   addPropertyName ( "Property3" );
}

void FormationMapProperty1Calculator::calculate ( DerivedProperties::AbstractPropertyManager& propertyManager,
                                                  const DataModel::AbstractSnapshot*          snapshot,
                                                  const DataModel::AbstractFormation*         formation,
                                                        FormationMapPropertyList&             derivedProperties ) const {
   
   const DataModel::AbstractProperty* property = propertyManager.getProperty ( "Property3" );

   DerivedFormationMapPropertyPtr derivedProp = DerivedFormationMapPropertyPtr ( new DerivedProperties::DerivedFormationMapProperty ( property, snapshot, formation, propertyManager.getMapGrid ()));
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
   addPropertyName ( "Property1" );
}

void FormationProperty1Calculator::calculate ( DerivedProperties::AbstractPropertyManager& propertyManager,
                                               const DataModel::AbstractSnapshot*          snapshot,
                                               const DataModel::AbstractFormation*         formation,
                                                     FormationPropertyList&                derivedProperties ) const {
   
   const DataModel::AbstractProperty* property = propertyManager.getProperty ( "Property1" );

   DerivedFormationPropertyPtr derivedProp = DerivedFormationPropertyPtr ( new TestFormationProperty ( property, snapshot, formation, propertyManager.getMapGrid (), 10 ));
   double value = 0.0; 

   derivedProperties.clear ();

   for ( unsigned int k = derivedProp->firstK (); k <= derivedProp->lastK (); ++ k ) {

      for ( unsigned int i = derivedProp->firstI ( true ); i <= derivedProp->lastI ( true ); ++i ) {

         for ( unsigned int j = derivedProp->firstJ ( true ); j <= derivedProp->lastJ ( true ); ++j ) {
            derivedProp->set ( i, j, k, value );
            value += 1.0;
         }
      }
   }

   derivedProperties.push_back ( derivedProp );
}


FormationSurfaceProperty1Calculator::FormationSurfaceProperty1Calculator () {
   addPropertyName ( "Property2" );
}

void FormationSurfaceProperty1Calculator::calculate ( DerivedProperties::AbstractPropertyManager& propertyManager,
                                                      const DataModel::AbstractSnapshot*          snapshot,
                                                      const DataModel::AbstractFormation*         formation,
                                                      const DataModel::AbstractSurface*           surface,
                                                            FormationSurfacePropertyList&         derivedProperties ) const {
   
   const DataModel::AbstractProperty* property = propertyManager.getProperty ( "Property2" );

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


TestFormationProperty::TestFormationProperty ( const DataModel::AbstractProperty*  property,
                                               const DataModel::AbstractSnapshot*  snapshot,
                                               const DataModel::AbstractFormation* formation,
                                               const DataModel::AbstractGrid*      grid,
                                               const unsigned int                  nk ) :
   DerivedProperties::DerivedFormationProperty ( property, snapshot, formation, grid, nk )
{
   m_hasBeenRetrieved = false;
}

double TestFormationProperty::get ( unsigned int i,
                                    unsigned int j,
                                    unsigned int k ) const {

   if ( m_hasBeenRetrieved ) {
      return DerivedProperties::DerivedFormationProperty::get ( i, j, k );
   } else {
      return UndefinedValue;
   }

}

double TestFormationProperty::getUndefinedValue () const {
   return UndefinedValue;
}

bool TestFormationProperty::isRetrieved () const {
   return m_hasBeenRetrieved;
}

void TestFormationProperty::retrieveData () const {
   m_hasBeenRetrieved = true;
}

void TestFormationProperty::restoreData () const {
   m_hasBeenRetrieved = false;
}

