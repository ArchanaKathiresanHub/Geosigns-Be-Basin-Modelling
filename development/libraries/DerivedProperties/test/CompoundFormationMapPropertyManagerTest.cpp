//
// Copyright (C) 2013-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell.
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
#include "../src/FormationMapPropertyCalculator.h"
#include "../src/DerivedFormationMapProperty.h"
#include "../src/FormationPropertyCalculator.h"
#include "../src/DerivedFormationProperty.h"
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
using namespace AbstractDerivedProperties;

static const double ValueToAdd = 10.0;

class TestPropertyManager : public AbstractPropertyManager {

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


class SurfaceProperty1Calculator : public SurfacePropertyCalculator {

public :

   SurfaceProperty1Calculator ();

   void calculate ( AbstractPropertyManager& propertyManager,
                    const DataModel::AbstractSnapshot*        snapshot,
                    const DataModel::AbstractSurface*         surface,
                          SurfacePropertyList&                derivedProperties ) const;

};

class SurfaceProperty2Calculator : public SurfacePropertyCalculator {

public :

   SurfaceProperty2Calculator ( const double value );

   void calculate ( AbstractPropertyManager& propertyManager,
                    const DataModel::AbstractSnapshot*        snapshot,
                    const DataModel::AbstractSurface*         surface,
                    SurfacePropertyList&                derivedProperties ) const;

private :

   double m_value;

};




class FormationMapProperty1Calculator : public FormationMapPropertyCalculator {

public :

   FormationMapProperty1Calculator ();

   void calculate ( AbstractPropertyManager& propertyManager,
                    const DataModel::AbstractSnapshot*          snapshot,
                    const DataModel::AbstractFormation*         formation,
                          FormationMapPropertyList&             derivedProperties ) const;

};

class FormationMapProperty2Calculator : public FormationMapPropertyCalculator {

public :

   FormationMapProperty2Calculator ( const double value );

   void calculate ( AbstractPropertyManager& propertyManager,
                    const DataModel::AbstractSnapshot*          snapshot,
                    const DataModel::AbstractFormation*         formation,
                    FormationMapPropertyList&                   derivedProperties ) const;

private :

   double m_value;

};


class FormationProperty1Calculator : public FormationPropertyCalculator {

public :

   FormationProperty1Calculator ();

   void calculate ( AbstractPropertyManager& propertyManager,
                    const DataModel::AbstractSnapshot*          snapshot,
                    const DataModel::AbstractFormation*         formation,
                          FormationPropertyList&                derivedProperties ) const;

};

class FormationProperty2Calculator : public FormationPropertyCalculator {

public :

   FormationProperty2Calculator ( const double value );

   void calculate ( AbstractPropertyManager& propertyManager,
                    const DataModel::AbstractSnapshot*          snapshot,
                    const DataModel::AbstractFormation*         formation,
                    FormationPropertyList&                      derivedProperties ) const;

private :

   double m_value;

};


class FormationSurfaceProperty1Calculator : public FormationSurfacePropertyCalculator {

public :

   FormationSurfaceProperty1Calculator ();

   void calculate ( AbstractPropertyManager& propertyManager,
                    const DataModel::AbstractSnapshot*          snapshot,
                    const DataModel::AbstractFormation*         formation,
                    const DataModel::AbstractSurface*           surface,
                          FormationSurfacePropertyList&         derivedProperties ) const;

};

class FormationSurfaceProperty2Calculator : public FormationSurfacePropertyCalculator {

public :

   FormationSurfaceProperty2Calculator ( const double value );

   void calculate ( AbstractPropertyManager& propertyManager,
                    const DataModel::AbstractSnapshot*          snapshot,
                    const DataModel::AbstractFormation*         formation,
                    const DataModel::AbstractSurface*           surface,
                          FormationSurfacePropertyList&         derivedProperties ) const;

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
// depend on property1. However, at the time of the request of property2, property1 has not been calculated
// so it must also be calculated. 
TEST ( CompoundFormationMapPropertyManagerTest,  Test2 )
{
   TestPropertyManager propertyManager;

   const DataModel::AbstractProperty* property1 = propertyManager.getProperty ( "Property1" );
   const DataModel::AbstractProperty* property2 = propertyManager.getProperty ( "Property2" );
   const DataModel::AbstractProperty* property3 = propertyManager.getProperty ( "Property3" );

   const DataModel::AbstractSnapshot*  snapshot = new MockSnapshot ( 0.0 );
   const DataModel::AbstractSurface*   surface = new MockSurface ( "TopSurface" );
   const DataModel::AbstractFormation* formation = new MockFormation ( "Formation1" );

   FormationMapPropertyPtr formationProperty2 = propertyManager.getFormationMapProperty ( property2, snapshot, formation );
   FormationMapPropertyPtr formationProperty1 = propertyManager.getFormationMapProperty ( property1, snapshot, formation );
   FormationMapPropertyPtr formationProperty3 = propertyManager.getFormationMapProperty ( property3, snapshot, formation );

   EXPECT_EQ ( property1, formationProperty1->getProperty ());
   EXPECT_EQ ( formation, formationProperty1->getFormation ());
   EXPECT_EQ ( snapshot, formationProperty1->getSnapshot ());

   EXPECT_EQ ( property2, formationProperty2->getProperty ());
   EXPECT_EQ ( formation, formationProperty2->getFormation ());
   EXPECT_EQ ( snapshot, formationProperty2->getSnapshot ());

   EXPECT_EQ ( property3, formationProperty3->getProperty ());
   EXPECT_EQ ( formation, formationProperty3->getFormation ());
   EXPECT_EQ ( snapshot, formationProperty3->getSnapshot ());

   for ( unsigned int i = formationProperty1->firstI ( true ); i <= formationProperty1->lastI ( true ); ++i ) {

      for ( unsigned int j = formationProperty1->firstJ ( true ); j <= formationProperty1->lastJ ( true ); ++j ) {
         EXPECT_DOUBLE_EQ ( formationProperty1->get ( i, j ) + ValueToAdd, formationProperty2->get ( i, j ));
      }

   }

   for ( unsigned int i = formationProperty1->firstI ( true ); i <= formationProperty1->lastI ( true ); ++i ) {

      for ( unsigned int j = formationProperty1->firstJ ( true ); j <= formationProperty1->lastJ ( true ); ++j ) {
         EXPECT_DOUBLE_EQ ( formationProperty1->get ( i, j ) + ValueToAdd * ValueToAdd, formationProperty3->get ( i, j ));
      }

   }

   delete snapshot;
   delete surface;
   delete formation;
}



TestPropertyManager::TestPropertyManager () {
   // These will come from the project handle.
   m_mockProperties.push_back ( new DataModel::MockProperty ( "Property1", DataModel::FORMATION_2D_PROPERTY ) );
   m_mockProperties.push_back ( new DataModel::MockProperty ( "Property2", DataModel::FORMATION_2D_PROPERTY ) );
   m_mockProperties.push_back ( new DataModel::MockProperty ( "Property3", DataModel::FORMATION_2D_PROPERTY ) );

   // Add all properties to the property manager.
   for ( size_t i = 0; i < m_mockProperties.size (); ++i ) {
      addProperty ( m_mockProperties [ i ]);
   }

   // This will come frmo the project handle.
   m_mapGrid = new DataModel::MockGrid ( 0, 0, 0, 0, 10, 10, 10, 10 );

   addSurfacePropertyCalculator ( SurfacePropertyCalculatorPtr ( new SurfaceProperty1Calculator ));
   addSurfacePropertyCalculator ( SurfacePropertyCalculatorPtr ( new SurfaceProperty2Calculator ( ValueToAdd )));

   addFormationMapPropertyCalculator ( FormationMapPropertyCalculatorPtr ( new FormationMapProperty1Calculator ));
   addFormationMapPropertyCalculator ( FormationMapPropertyCalculatorPtr ( new FormationMapProperty2Calculator ( ValueToAdd )));

   addFormationPropertyCalculator ( FormationPropertyCalculatorPtr ( new FormationProperty1Calculator ));
   addFormationPropertyCalculator ( FormationPropertyCalculatorPtr ( new FormationProperty2Calculator ( ValueToAdd )));

   addFormationSurfacePropertyCalculator ( FormationSurfacePropertyCalculatorPtr ( new FormationSurfaceProperty1Calculator ));
   addFormationSurfacePropertyCalculator ( FormationSurfacePropertyCalculatorPtr ( new FormationSurfaceProperty2Calculator ( ValueToAdd )));


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

bool TestPropertyManager::getNodeIsValid ( const unsigned int i, const unsigned int j ) const { 
   (void) i;
   (void) j;

   return true; 
}

const DataModel::AbstractGrid* TestPropertyManager::getMapGrid () const {
   return m_mapGrid;
}

SurfaceProperty1Calculator::SurfaceProperty1Calculator () {
   addPropertyName ( "Property1" );
}

void SurfaceProperty1Calculator::calculate ( AbstractPropertyManager& propertyManager,
                                             const DataModel::AbstractSnapshot*        snapshot,
                                             const DataModel::AbstractSurface*         surface,
                                             SurfacePropertyList&                derivedProperties ) const {

   const DataModel::AbstractProperty* property = propertyManager.getProperty ( "Property1" );

   DerivedSurfacePropertyPtr derivedProp = DerivedSurfacePropertyPtr ( new DerivedSurfaceProperty ( property, snapshot, surface, propertyManager.getMapGrid ()));
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

SurfaceProperty2Calculator::SurfaceProperty2Calculator ( const double value ) : m_value ( value ) {
   addPropertyName ( "Property2" );
   addPropertyName ( "Property3" );
}

void SurfaceProperty2Calculator::calculate ( AbstractPropertyManager& propertyManager,
                                             const DataModel::AbstractSnapshot*        snapshot,
                                             const DataModel::AbstractSurface*         surface,
                                             SurfacePropertyList&                derivedProperties ) const {

   const DataModel::AbstractProperty* property1 = propertyManager.getProperty ( "Property1" );
   const DataModel::AbstractProperty* property2 = propertyManager.getProperty ( "Property2" );
   const DataModel::AbstractProperty* property3 = propertyManager.getProperty ( "Property3" );

   const SurfacePropertyPtr prop1 = propertyManager.getSurfaceProperty ( property1, snapshot, surface );

   DerivedSurfacePropertyPtr derivedProp2 = DerivedSurfacePropertyPtr( new DerivedSurfaceProperty ( property2, snapshot, surface, propertyManager.getMapGrid ()));
   DerivedSurfacePropertyPtr derivedProp3 = DerivedSurfacePropertyPtr( new DerivedSurfaceProperty ( property3, snapshot, surface, propertyManager.getMapGrid ()));

   derivedProperties.clear ();

   for ( unsigned int i = derivedProp2->firstI ( true ); i <= derivedProp2->lastI ( true ); ++i ) {

      for ( unsigned int j = derivedProp2->firstJ ( true ); j <= derivedProp2->lastJ ( true ); ++j ) {
         derivedProp2->set ( i, j, prop1->get ( i, j ) + m_value );
         derivedProp3->set ( i, j, prop1->get ( i, j ) + m_value * m_value );
      }

   }

   derivedProperties.push_back ( derivedProp2 );
   derivedProperties.push_back ( derivedProp3 );
}




FormationMapProperty1Calculator::FormationMapProperty1Calculator () {
   addPropertyName ( "Property1" );
}

void FormationMapProperty1Calculator::calculate ( AbstractPropertyManager& propertyManager,
                                                  const DataModel::AbstractSnapshot*          snapshot,
                                                  const DataModel::AbstractFormation*         formation,
                                                        FormationMapPropertyList&             derivedProperties ) const {

   const DataModel::AbstractProperty* property = propertyManager.getProperty ( "Property1" );

   DerivedFormationMapPropertyPtr derivedProp = DerivedFormationMapPropertyPtr ( new DerivedFormationMapProperty ( property, snapshot, formation, propertyManager.getMapGrid ()));
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

FormationMapProperty2Calculator::FormationMapProperty2Calculator ( const double value ) : m_value ( value ) {
   addPropertyName ( "Property2" );
   addPropertyName ( "Property3" );
}

void FormationMapProperty2Calculator::calculate ( AbstractPropertyManager& propertyManager,
                                                  const DataModel::AbstractSnapshot*          snapshot,
                                                  const DataModel::AbstractFormation*         formation,
                                                        FormationMapPropertyList&             derivedProperties ) const {

   const DataModel::AbstractProperty* property1 = propertyManager.getProperty ( "Property1" );
   const DataModel::AbstractProperty* property2 = propertyManager.getProperty ( "Property2" );
   const DataModel::AbstractProperty* property3 = propertyManager.getProperty ( "Property3" );

   const FormationMapPropertyPtr prop1 = propertyManager.getFormationMapProperty ( property1, snapshot, formation );

   DerivedFormationMapPropertyPtr derivedProp2 = DerivedFormationMapPropertyPtr( new DerivedFormationMapProperty ( property2, snapshot, formation, propertyManager.getMapGrid ()));
   DerivedFormationMapPropertyPtr derivedProp3 = DerivedFormationMapPropertyPtr( new DerivedFormationMapProperty ( property3, snapshot, formation, propertyManager.getMapGrid ()));

   derivedProperties.clear ();

   for ( unsigned int i = derivedProp2->firstI ( true ); i <= derivedProp2->lastI ( true ); ++i ) {

      for ( unsigned int j = derivedProp2->firstJ ( true ); j <= derivedProp2->lastJ ( true ); ++j ) {
         derivedProp2->set ( i, j, prop1->get ( i, j ) + m_value );
         derivedProp3->set ( i, j, prop1->get ( i, j ) + m_value * m_value );
      }

   }

   derivedProperties.push_back ( derivedProp2 );
   derivedProperties.push_back ( derivedProp3 );
}





FormationProperty1Calculator::FormationProperty1Calculator () {
   addPropertyName ( "Property1" );
}

void FormationProperty1Calculator::calculate ( AbstractPropertyManager& propertyManager,
                                                  const DataModel::AbstractSnapshot*       snapshot,
                                                  const DataModel::AbstractFormation*      formation,
                                                        FormationPropertyList&             derivedProperties ) const {

   const DataModel::AbstractProperty* property = propertyManager.getProperty ( "Property1" );

   DerivedFormationPropertyPtr derivedProp = DerivedFormationPropertyPtr ( new DerivedFormationProperty ( property, snapshot, formation, propertyManager.getMapGrid (), 10 ));
   double value = 0.0;

   derivedProperties.clear ();

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

FormationProperty2Calculator::FormationProperty2Calculator ( const double value ) : m_value ( value ) {
   addPropertyName ( "Property2" );
   addPropertyName ( "Property3" );
}

void FormationProperty2Calculator::calculate ( AbstractPropertyManager& propertyManager,
                                                  const DataModel::AbstractSnapshot*       snapshot,
                                                  const DataModel::AbstractFormation*      formation,
                                                        FormationPropertyList&             derivedProperties ) const {

   const DataModel::AbstractProperty* property1 = propertyManager.getProperty ( "Property1" );
   const DataModel::AbstractProperty* property2 = propertyManager.getProperty ( "Property2" );
   const DataModel::AbstractProperty* property3 = propertyManager.getProperty ( "Property3" );

   const FormationPropertyPtr prop1 = propertyManager.getFormationProperty ( property1, snapshot, formation );

   DerivedFormationPropertyPtr derivedProp2 = DerivedFormationPropertyPtr( new DerivedFormationProperty ( property2, snapshot, formation, propertyManager.getMapGrid (), 10 ));
   DerivedFormationPropertyPtr derivedProp3 = DerivedFormationPropertyPtr( new DerivedFormationProperty ( property3, snapshot, formation, propertyManager.getMapGrid (), 10 ));

   derivedProperties.clear ();

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


FormationSurfaceProperty1Calculator::FormationSurfaceProperty1Calculator () {
   addPropertyName ( "Property1" );
}

void FormationSurfaceProperty1Calculator::calculate ( AbstractPropertyManager& propertyManager,
                                                      const DataModel::AbstractSnapshot*          snapshot,
                                                      const DataModel::AbstractFormation*         formation,
                                                      const DataModel::AbstractSurface*           surface,
                                                            FormationSurfacePropertyList&         derivedProperties ) const {

   const DataModel::AbstractProperty* property = propertyManager.getProperty ( "Property1" );

   DerivedFormationSurfacePropertyPtr derivedProp = DerivedFormationSurfacePropertyPtr ( new DerivedFormationSurfaceProperty ( property, snapshot, formation, surface, propertyManager.getMapGrid ()));
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

FormationSurfaceProperty2Calculator::FormationSurfaceProperty2Calculator ( const double value ) : m_value ( value ) {
   addPropertyName ( "Property2" );
   addPropertyName ( "Property3" );
}

void FormationSurfaceProperty2Calculator::calculate ( AbstractPropertyManager& propertyManager,
                                                      const DataModel::AbstractSnapshot*          snapshot,
                                                      const DataModel::AbstractFormation*         formation,
                                                      const DataModel::AbstractSurface*           surface,
                                                            FormationSurfacePropertyList&         derivedProperties ) const {

   const DataModel::AbstractProperty* property1 = propertyManager.getProperty ( "Property1" );
   const DataModel::AbstractProperty* property2 = propertyManager.getProperty ( "Property2" );
   const DataModel::AbstractProperty* property3 = propertyManager.getProperty ( "Property3" );

   const FormationSurfacePropertyPtr prop1 = propertyManager.getFormationSurfaceProperty ( property1, snapshot, formation, surface );

   DerivedFormationSurfacePropertyPtr derivedProp2 = DerivedFormationSurfacePropertyPtr( new DerivedFormationSurfaceProperty ( property2, snapshot, formation, surface, propertyManager.getMapGrid ()));
   DerivedFormationSurfacePropertyPtr derivedProp3 = DerivedFormationSurfacePropertyPtr( new DerivedFormationSurfaceProperty ( property3, snapshot, formation, surface, propertyManager.getMapGrid ()));

   derivedProperties.clear ();

   for ( unsigned int i = derivedProp2->firstI ( true ); i <= derivedProp2->lastI ( true ); ++i ) {

      for ( unsigned int j = derivedProp2->firstJ ( true ); j <= derivedProp2->lastJ ( true ); ++j ) {
         derivedProp2->set ( i, j, prop1->get ( i, j ) + m_value );
         derivedProp3->set ( i, j, prop1->get ( i, j ) + m_value * m_value );
      }

   }

   derivedProperties.push_back ( derivedProp2 );
   derivedProperties.push_back ( derivedProp3 );
}



