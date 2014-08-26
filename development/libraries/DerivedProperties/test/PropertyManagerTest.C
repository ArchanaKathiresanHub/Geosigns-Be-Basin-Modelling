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
                    const DataModel::AbstractProperty*        property,
                    const DataModel::AbstractSnapshot*        snapshot,
                    const DataModel::AbstractSurface*         surface,
                          SurfacePropertyList&                derivedProperties ) const;

};

class Property2Calculator : public DerivedProperties::SurfacePropertyCalculator {

public :

   Property2Calculator ( const double value );

   void calculate ( DerivedProperties::DerivedPropertyManager& propertyManager,
                    const DataModel::AbstractProperty*        property,
                    const DataModel::AbstractSnapshot*        snapshot,
                    const DataModel::AbstractSurface*         surface,
                    SurfacePropertyList&                derivedProperties ) const;

private :

   double m_value;

};

class Property4Calculator : public DerivedProperties::SurfacePropertyCalculator {

public :

   void calculate ( DerivedProperties::DerivedPropertyManager& propertyManager,
                    const DataModel::AbstractProperty*        property,
                    const DataModel::AbstractSnapshot*        snapshot,
                    const DataModel::AbstractSurface*         surface,
                    SurfacePropertyList&                derivedProperties ) const;

private :

   double m_value;

};

int main () {

   TestPropertyManager propertyManager;

   const DataModel::AbstractProperty* p1 = propertyManager.getProperty ( "Property1" );
   const DataModel::AbstractProperty* p2 = propertyManager.getProperty ( "Property2" );
   const DataModel::AbstractProperty* p3 = propertyManager.getProperty ( "Property3" );
   const DataModel::AbstractProperty* p4 = propertyManager.getProperty ( "Property4" );

   const DataModel::AbstractSnapshot*  s1 = new MockSnapshot ( 0.0 );
   const DataModel::AbstractSurface*   surface1 = new MockSurface ( "TopSurface" );
   const DataModel::AbstractFormation* formation1 = new MockFormation ( "Formation1" );

   SurfacePropertyPtr sp4 = propertyManager.getSurfaceProperty ( p4, s1, surface1 );
   SurfacePropertyPtr sp2 = propertyManager.getSurfaceProperty ( p2, s1, surface1 );
   SurfacePropertyPtr sp1 = propertyManager.getSurfaceProperty ( p1, s1, surface1 );
   SurfacePropertyPtr sp3 = propertyManager.getSurfaceProperty ( p3, s1, surface1 );

   std::cout << " test " << sp1->getProperty ()->getName ()<< std::endl;
   std::cout << " test " << sp1->get ( 2, 2 ) << std::endl;

   std::cout << " test " << sp2->getProperty ()->getName ()<< std::endl;
   std::cout << " test " << sp2->get ( 2, 2 ) << std::endl;

   std::cout << " prop3 "  << ( sp3 == 0 ? " is null" : " aint null" ) << std::endl;
   std::cout << " test " << sp3->getProperty ()->getName ()<< std::endl;
   std::cout << " test " << sp3->get ( 2, 2 ) << std::endl;


   std::cout << " test " << sp4->getProperty ()->getName ()<< std::endl;
   std::cout << " test " << sp4->get ( 2, 2 ) << std::endl;



   return 0;
}


TestPropertyManager::TestPropertyManager () {
   // These will come from the project handle.
   m_mockProperties.push_back ( new DataModel::MockProperty ( "Property1" ) );
   m_mockProperties.push_back ( new DataModel::MockProperty ( "Property2" ) );
   m_mockProperties.push_back ( new DataModel::MockProperty ( "Property3" ) );
   m_mockProperties.push_back ( new DataModel::MockProperty ( "Property4" ) );

   // addEncompassingProperty ( m_mockProperties [ 2 ], m_mockProperties [ 1 ]);

   // Add all properties to the property manager.
   for ( size_t i = 0; i < m_mockProperties.size (); ++i ) {
      addProperty ( m_mockProperties [ i ]);
   }

   // This will come frmo the project handle.
   m_mapGrid = new DataModel::MockGrid ( 0, 0, 0, 0, 10, 10, 10, 10 );

   addCalculator ( m_mockProperties [ 0 ], SurfacePropertyCalculatorPtr ( new Property1Calculator ));

   // calc = SurfacePropertyCalculatorPtr ( new Property2Calculator ( 10.0 ))

   // addCalculator ( m_mockProperties [ 1 ], calc );
   // addCalculator ( m_mockProperties [ 2 ], calc );


   addCalculator ( m_mockProperties [ 1 ], SurfacePropertyCalculatorPtr ( new Property2Calculator ( 10.0 )));

   addCalculator ( m_mockProperties [ 2 ], SurfacePropertyCalculatorPtr ( new Property2Calculator ( 10.0 )));

   addCalculator ( m_mockProperties [ 3 ], SurfacePropertyCalculatorPtr ( new Property4Calculator ));

}


TestPropertyManager::~TestPropertyManager () {

   for ( size_t i = 0; i < m_mockProperties.size (); ++i ) {
      delete m_mockProperties [ i ];
   }

}

const DataModel::AbstractGrid* TestPropertyManager::getMapGrid () const {
   return m_mapGrid;
}

void Property1Calculator::calculate ( DerivedProperties::DerivedPropertyManager& propertyManager,
                                      const DataModel::AbstractProperty*        property,
                                      const DataModel::AbstractSnapshot*        snapshot,
                                      const DataModel::AbstractSurface*         surface,
                                      SurfacePropertyList&                derivedProperties ) const {

   boost::shared_ptr<DerivedProperties::DerivedSurfaceProperty> derivedProp = boost::shared_ptr<DerivedProperties::DerivedSurfaceProperty>( new DerivedProperties::DerivedSurfaceProperty ( property, snapshot, surface, propertyManager.getMapGrid ()));
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
                                      const DataModel::AbstractProperty*        property,
                                      const DataModel::AbstractSnapshot*        snapshot,
                                      const DataModel::AbstractSurface*         surface,
                                      SurfacePropertyList&                derivedProperties ) const {

   const DataModel::AbstractProperty* property1 = propertyManager.getProperty ( "Property1" );
   const DataModel::AbstractProperty* property2 = propertyManager.getProperty ( "Property2" );
   const DataModel::AbstractProperty* property3 = propertyManager.getProperty ( "Property3" );

   const SurfacePropertyPtr prop1 = propertyManager.getSurfaceProperty ( property1, snapshot, surface );

   boost::shared_ptr<DerivedProperties::DerivedSurfaceProperty> derivedProp2 = 
      boost::shared_ptr<DerivedProperties::DerivedSurfaceProperty>( new DerivedProperties::DerivedSurfaceProperty ( property2, snapshot, surface, propertyManager.getMapGrid ()));

   boost::shared_ptr<DerivedProperties::DerivedSurfaceProperty> derivedProp3 = 
      boost::shared_ptr<DerivedProperties::DerivedSurfaceProperty>( new DerivedProperties::DerivedSurfaceProperty ( property3, snapshot, surface, propertyManager.getMapGrid ()));

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
                                      const DataModel::AbstractProperty*        property,
                                      const DataModel::AbstractSnapshot*        snapshot,
                                      const DataModel::AbstractSurface*         surface,
                                      SurfacePropertyList&                derivedProperties ) const {

   const DataModel::AbstractProperty* property1 = propertyManager.getProperty ( "Property1" );
   const DataModel::AbstractProperty* property2 = propertyManager.getProperty ( "Property3" );
   const DataModel::AbstractProperty* property4 = propertyManager.getProperty ( "Property4" );

   const SurfacePropertyPtr prop1 = propertyManager.getSurfaceProperty ( property1, snapshot, surface );
   const SurfacePropertyPtr prop2 = propertyManager.getSurfaceProperty ( property2, snapshot, surface );

   boost::shared_ptr<DerivedProperties::DerivedSurfaceProperty> derivedProp = 
      boost::shared_ptr<DerivedProperties::DerivedSurfaceProperty>( new DerivedProperties::DerivedSurfaceProperty ( property4, snapshot, surface, propertyManager.getMapGrid ()));


   for ( unsigned int i = derivedProp->firstI ( true ); i <= derivedProp->lastI ( true ); ++i ) {

      for ( unsigned int j = derivedProp->firstJ ( true ); j <= derivedProp->lastJ ( true ); ++j ) {
         derivedProp->set ( i, j, prop2->get ( i, j ) - prop1->get ( i, j ));
      }

   }

   derivedProperties.push_back ( derivedProp );
}


