#include <iostream>
#include <vector>
#include <string>
#include "../src/AbstractPropertyManager.h"
#include "../src/FormationPropertyCalculator.h"
#include "../src/DerivedFormationProperty.h"
#include "../src/DerivedFormationMapProperty.h"
#include "../src/ThicknessFormationMapCalculator.h"

#include "MockSurface.h"
#include "MockFormation.h"
#include "MockSnapshot.h"
#include "MockProperty.h"
#include "MockGrid.h"

#include <gtest/gtest.h>

using namespace DataModel;
using namespace DerivedProperties;

static const double initialLayerThickness = 500;
static const double depthDelta = 100.0;
static const double topDepth = 0.0;
static const double topDepthXDiff = 10.0;
static const double topDepthYDiff = 10.0;
static const int numberOfNodes = 11;

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

class DepthCalculator : public DerivedProperties::FormationPropertyCalculator {

public :

   DepthCalculator ();

   void calculate ( DerivedProperties::AbstractPropertyManager& propertyManager,
                    const DataModel::AbstractSnapshot*          snapshot,
                    const DataModel::AbstractFormation*         formation,
                          FormationPropertyList&                derivedProperties ) const;

};

// Constant depth => constant thickness test.
TEST ( ThicknessCalculatorTest, Test1 )
{

   TestPropertyManager propertyManager;

   const DataModel::AbstractProperty*  thicknessProp = propertyManager.getProperty ( "Thickness" );
   const DataModel::AbstractSnapshot*  snapshot = new MockSnapshot ( 0.0 );
   const DataModel::AbstractFormation* formation1 = new MockFormation ( "Formation1" );

   double expectedThickness = depthDelta * static_cast<double>( numberOfNodes - 1 );

   const FormationMapPropertyPtr thickness1 = propertyManager.getFormationMapProperty ( thicknessProp, snapshot, formation1 );

   for ( unsigned int i = thickness1->firstI ( true ); i <= thickness1->lastI ( true ); ++i ) {

      for ( unsigned int j = thickness1->firstJ ( true ); j <= thickness1->lastJ ( true ); ++j ) {
         EXPECT_DOUBLE_EQ ( thickness1->get ( i, j ), expectedThickness );
      }

   }

   delete formation1;
   delete snapshot;
}

// Varying depth with constant thickness test.
TEST ( ThicknessCalculatorTest, Test2 )
{

   TestPropertyManager propertyManager;

   const DataModel::AbstractProperty*  thicknessProp = propertyManager.getProperty ( "Thickness" );
   const DataModel::AbstractProperty*  depthProp = propertyManager.getProperty ( "Depth" );
   const DataModel::AbstractSnapshot*  snapshot = new MockSnapshot ( 0.0 );
   const DataModel::AbstractFormation* formation = new MockFormation ( "Formation2" );

   double expectedThickness = depthDelta * static_cast<double>( numberOfNodes - 1 );

   const FormationMapPropertyPtr thickness = propertyManager.getFormationMapProperty ( thicknessProp, snapshot, formation );

   for ( unsigned int i = thickness->firstI ( true ); i <= thickness->lastI ( true ); ++i ) {

      for ( unsigned int j = thickness->firstJ ( true ); j <= thickness->lastJ ( true ); ++j ) {
         EXPECT_DOUBLE_EQ ( thickness->get ( i, j ), expectedThickness );
      }

   }

   delete formation;
   delete snapshot;
}

// Varying depth with varying thickness test.
TEST ( ThicknessCalculatorTest, Test3 )
{

   TestPropertyManager propertyManager;

   const DataModel::AbstractProperty*  thicknessProp = propertyManager.getProperty ( "Thickness" );
   const DataModel::AbstractProperty*  depthProp = propertyManager.getProperty ( "Depth" );
   const DataModel::AbstractSnapshot*  snapshot = new MockSnapshot ( 0.0 );
   const DataModel::AbstractFormation* formation = new MockFormation ( "Formation3" );

   const FormationMapPropertyPtr thickness = propertyManager.getFormationMapProperty ( thicknessProp, snapshot, formation );
   double expectedThickness;

   for ( unsigned int i = thickness->firstI ( true ); i <= thickness->lastI ( true ); ++i ) {

      for ( unsigned int j = thickness->firstJ ( true ); j <= thickness->lastJ ( true ); ++j ) {
         expectedThickness = initialLayerThickness + static_cast<double>( i ) * topDepthXDiff + static_cast<double>( j ) * topDepthYDiff;
         EXPECT_DOUBLE_EQ ( expectedThickness, thickness->get ( i, j ));

      }

   }

   delete formation;
   delete snapshot;
}


TestPropertyManager::TestPropertyManager () {
   // These will come from the project handle.
   m_mockProperties.push_back ( new DataModel::MockProperty ( "Depth" ) );
   m_mockProperties.push_back ( new DataModel::MockProperty ( "Thickness" ) );

   // Add all properties to the property manager.
   for ( size_t i = 0; i < m_mockProperties.size (); ++i ) {
      addProperty ( m_mockProperties [ i ]);
   }

   // This will come from the project handle.
   m_mapGrid = new DataModel::MockGrid ( 0, 0, 0, 0, 10, 10, 10, 10 );

   addFormationPropertyCalculator ( FormationPropertyCalculatorPtr ( new DepthCalculator )); 
   addFormationMapPropertyCalculator ( FormationMapPropertyCalculatorPtr ( new DerivedProperties::ThicknessFormationMapCalculator ));
}


TestPropertyManager::~TestPropertyManager () {

   for ( size_t i = 0; i < m_mockProperties.size (); ++i ) {
      delete m_mockProperties [ i ];
   }

   delete m_mapGrid;
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

void TestPropertyManager::addProperty ( const DataModel::AbstractProperty* property ) {
   
   if ( std::find ( m_properties.begin (), m_properties.end (), property ) == m_properties.end ()) {
      m_properties.push_back ( property );
   }

}



DepthCalculator::DepthCalculator () {
   addPropertyName ( "Depth" );
}

void DepthCalculator::calculate ( DerivedProperties::AbstractPropertyManager& propertyManager,
                                  const DataModel::AbstractSnapshot*          snapshot,
                                  const DataModel::AbstractFormation*         formation,
                                        FormationPropertyList&                derivedProperties ) const {

   const DataModel::AbstractProperty* depth = propertyManager.getProperty ( "Depth" );

   DerivedFormationPropertyPtr depthProp = DerivedFormationPropertyPtr( new DerivedProperties::DerivedFormationProperty ( depth,
                                                                                                                          snapshot,
                                                                                                                          formation,
                                                                                                                          propertyManager.getMapGrid (),
                                                                                                                          numberOfNodes ));

   if ( formation->getName () == "Formation1" ) {

      for ( unsigned int i = depthProp->firstI ( true ); i <= depthProp->lastI ( true ); ++i ) {

         for ( unsigned int j = depthProp->firstJ ( true ); j <= depthProp->lastJ ( true ); ++j ) {
            double depthValue = topDepth;

            for ( unsigned int k = depthProp->firstK (); k <= depthProp->lastK (); ++k ) {
               unsigned int mk = depthProp->lastK () - k;
               depthProp->set ( i, j, mk, depthValue );
               depthValue += depthDelta;
            }

         }

      }

   } else if ( formation->getName () == "Formation2" ) {
      double depthValue = topDepth;

      for ( unsigned int i = depthProp->firstI ( true ); i <= depthProp->lastI ( true ); ++i ) {

         for ( unsigned int j = depthProp->firstJ ( true ); j <= depthProp->lastJ ( true ); ++j ) {
            depthValue = static_cast<double>( i ) * topDepthXDiff + static_cast<double>( j ) * topDepthYDiff;

            for ( unsigned int k = depthProp->firstK (); k <= depthProp->lastK (); ++k ) {
               unsigned int mk = depthProp->lastK () - k;
               depthProp->set ( i, j, mk, depthValue );
               depthValue += depthDelta;
            }

         }

      }

   } else if ( formation->getName () == "Formation3" ) {
      double depthValue;
      double thicknessValue;
      double thicknessDelta;

      for ( unsigned int i = depthProp->firstI ( true ); i <= depthProp->lastI ( true ); ++i ) {

         for ( unsigned int j = depthProp->firstJ ( true ); j <= depthProp->lastJ ( true ); ++j ) {
            thicknessValue = initialLayerThickness + static_cast<double>( i ) * topDepthXDiff + static_cast<double>( j ) * topDepthYDiff;
            thicknessDelta = thicknessValue / static_cast<double>(depthProp->lengthK () - 1 );
            depthValue = 0.0;

            for ( unsigned int k = depthProp->firstK (); k <= depthProp->lastK (); ++k ) {
               unsigned int mk = depthProp->lastK () - k;
               depthProp->set ( i, j, mk, depthValue );
               depthValue += thicknessDelta;
            }

         }

      }

   }

   derivedProperties.push_back ( depthProp );


}
