#include <iostream>
#include <vector>
#include <string>
#include "../src/AbstractPropertyManager.h"
#include "../src/FormationPropertyCalculator.h"
#include "../src/DerivedFormationProperty.h"
#include "../src/DerivedFormationMapProperty.h"
#include "../src/OverpressureFormationCalculator.h"

#include "MockFormation.h"
#include "MockSurface.h"
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

static const double gravity = 10.0;
static const double waterDensity = 1000.0;
static const double pressureGradient = 1.5;


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

class HydrostaticCalculator : public DerivedProperties::FormationPropertyCalculator {

public :

   HydrostaticCalculator ();

   void calculate ( DerivedProperties::AbstractPropertyManager& propertyManager,
                    const DataModel::AbstractSnapshot*          snapshot,
                    const DataModel::AbstractFormation*         formation,
                          FormationPropertyList&                derivedProperties ) const;

};


class PressureCalculator : public DerivedProperties::FormationPropertyCalculator {

public :

   PressureCalculator ();

   void calculate ( DerivedProperties::AbstractPropertyManager& propertyManager,
                    const DataModel::AbstractSnapshot*          snapshot,
                    const DataModel::AbstractFormation*         formation,
                          FormationPropertyList&                derivedProperties ) const;

};



//
// In this particular test the pore-pressure is discontinuous, there is a jump in the pore-pressure across the surface.
// So the surface formation test should be used and will be different across the surface.
//
TEST ( SurfacePropertyTest, Test1 )
{

   TestPropertyManager propertyManager;

   const DataModel::AbstractProperty*  overpressureProp = propertyManager.getProperty ( "OverPressure" );
   const DataModel::AbstractProperty*  pressureProp = propertyManager.getProperty ( "Pressure" );
   const DataModel::AbstractProperty*  hydroPressureProp = propertyManager.getProperty ( "HydroStaticPressure" );
   const DataModel::AbstractProperty*  depthProp = propertyManager.getProperty ( "Depth" );

   const DataModel::AbstractSnapshot*  snapshot = new MockSnapshot ( 0.0 );
   const DataModel::AbstractFormation* formation1 = new MockFormation ( "Formation1", "TopSurface", "MidSurface" );
   const DataModel::AbstractFormation* formation2 = new MockFormation ( "Formation2", "MidSurface", "BotSurface" );

   const DataModel::AbstractSurface*  topSurface = new MockSurface ( "TopSurface", 0, formation1 );
   const DataModel::AbstractSurface*  midSurface = new MockSurface ( "MidSurface", formation1, formation2 );
   const DataModel::AbstractSurface*  botSurface = new MockSurface ( "BotSurface", formation2, 0 );

   const FormationPropertyPtr depth1 = propertyManager.getFormationProperty ( depthProp, snapshot, formation1 );
   const FormationPropertyPtr depth2 = propertyManager.getFormationProperty ( depthProp, snapshot, formation2 );

   const FormationPropertyPtr pp1 = propertyManager.getFormationProperty ( pressureProp, snapshot, formation1 );
   const FormationPropertyPtr pp2 = propertyManager.getFormationProperty ( pressureProp, snapshot, formation2 );

   const SurfacePropertyPtr zs1 = propertyManager.getSurfaceProperty ( depthProp, snapshot, midSurface );

   const FormationSurfacePropertyPtr pps1 = propertyManager.getFormationSurfaceProperty ( pressureProp, snapshot, formation1, midSurface );
   const FormationSurfacePropertyPtr pps2 = propertyManager.getFormationSurfaceProperty ( pressureProp, snapshot, formation2, midSurface );

   // Depth should be the same for the surface and the bottom of the formation1 data
   for ( unsigned int i = depth1->firstI ( true ); i <= depth1->lastI ( true ); ++i ) {

      for ( unsigned int j = depth1->firstJ ( true ); j <= depth1->lastJ ( true ); ++j ) {
         EXPECT_DOUBLE_EQ ( zs1->get ( i, j ), depth1->get ( i, j, 0 ));
      }

   }

   // Depth should be the same for the surface and the top of the formation2 data
   for ( unsigned int i = depth1->firstI ( true ); i <= depth1->lastI ( true ); ++i ) {

      for ( unsigned int j = depth1->firstJ ( true ); j <= depth1->lastJ ( true ); ++j ) {
         EXPECT_DOUBLE_EQ ( zs1->get ( i, j ), depth2->get ( i, j, depth2->lastK ()));
      }

   }

   // Pore-pressure at the surface should be the same as the pore pressure at the bottom of formation1.
   for ( unsigned int i = depth1->firstI ( true ); i <= depth1->lastI ( true ); ++i ) {

      for ( unsigned int j = depth1->firstJ ( true ); j <= depth1->lastJ ( true ); ++j ) {
         EXPECT_DOUBLE_EQ ( pps1->get ( i, j ), pp1->get ( i, j, 0 ));
      }

   }

   // Pore-pressure at the surface should be the same as the pore pressure at the top of formation2.
   for ( unsigned int i = depth1->firstI ( true ); i <= depth1->lastI ( true ); ++i ) {

      for ( unsigned int j = depth1->firstJ ( true ); j <= depth1->lastJ ( true ); ++j ) {
         EXPECT_DOUBLE_EQ ( pps2->get ( i, j ), pp2->get ( i, j, pp2->lastK ()));
      }

   }

   delete formation1;
   delete formation2;
   delete topSurface;
   delete midSurface;
   delete botSurface;
   delete snapshot;
}



TestPropertyManager::TestPropertyManager () {
   // These will come from the project handle.
   m_mockProperties.push_back ( new DataModel::MockProperty ( "Depth" ) );
   m_mockProperties.push_back ( new DataModel::MockProperty ( "Thickness" ) );
   m_mockProperties.push_back ( new DataModel::MockProperty ( "Pressure" ) );
   m_mockProperties.push_back ( new DataModel::MockProperty ( "OverPressure" ) );
   m_mockProperties.push_back ( new DataModel::MockProperty ( "HydroStaticPressure" ) );

   // Add all properties to the property manager.
   for ( size_t i = 0; i < m_mockProperties.size (); ++i ) {
      addProperty ( m_mockProperties [ i ]);
   }

   // This will come from the project handle.
   m_mapGrid = new DataModel::MockGrid ( 0, 0, 0, 0, 10, 10, 10, 10 );

   addFormationPropertyCalculator ( FormationPropertyCalculatorPtr ( new DepthCalculator )); 
   addFormationPropertyCalculator ( FormationPropertyCalculatorPtr ( new HydrostaticCalculator )); 
   addFormationPropertyCalculator ( FormationPropertyCalculatorPtr ( new PressureCalculator )); 
   addFormationPropertyCalculator ( FormationPropertyCalculatorPtr ( new DerivedProperties::OverpressureFormationCalculator ));
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

   double calculatorTopDepth = topDepth;

   if ( formation->getName () == "Formation2" ) {
      calculatorTopDepth += ( numberOfNodes - 1 ) * depthDelta;
   }

   for ( unsigned int i = depthProp->firstI ( true ); i <= depthProp->lastI ( true ); ++i ) {

      for ( unsigned int j = depthProp->firstJ ( true ); j <= depthProp->lastJ ( true ); ++j ) {
         double depthValue = calculatorTopDepth;

         for ( unsigned int k = depthProp->firstK (); k <= depthProp->lastK (); ++k ) {
            unsigned int mk = depthProp->lastK () - k;
            depthProp->set ( i, j, mk, depthValue );
            depthValue += depthDelta;
         }

      }

   }

   derivedProperties.push_back ( depthProp );
}


HydrostaticCalculator::HydrostaticCalculator () {
   addPropertyName ( "HydroStaticPressure" );
}

void HydrostaticCalculator::calculate ( DerivedProperties::AbstractPropertyManager& propertyManager,
                                        const DataModel::AbstractSnapshot*          snapshot,
                                        const DataModel::AbstractFormation*         formation,
                                              FormationPropertyList&                derivedProperties ) const {

   const DataModel::AbstractProperty* depthProp = propertyManager.getProperty ( "Depth" );
   const DataModel::AbstractProperty* hydrostaticPressureProp = propertyManager.getProperty ( "HydroStaticPressure" );
   const FormationPropertyPtr depth = propertyManager.getFormationProperty ( depthProp, snapshot, formation );

   DerivedFormationPropertyPtr hydrostaticPressure = DerivedFormationPropertyPtr( new DerivedProperties::DerivedFormationProperty ( hydrostaticPressureProp,
                                                                                                                                    snapshot,
                                                                                                                                    formation,
                                                                                                                                    propertyManager.getMapGrid (),
                                                                                                                                    numberOfNodes ));

   for ( unsigned int i = depth->firstI ( true ); i <= depth->lastI ( true ); ++i ) {

      for ( unsigned int j = depth->firstJ ( true ); j <= depth->lastJ ( true ); ++j ) {

         for ( unsigned int k = depth->firstK (); k <= depth->lastK (); ++k ) {
            hydrostaticPressure->set ( i, j, k, gravity * waterDensity * depth->get ( i, j, k ));
         }

      }

   }

   derivedProperties.push_back ( hydrostaticPressure );
}


PressureCalculator::PressureCalculator () {
   addPropertyName ( "Pressure" );
}

void PressureCalculator::calculate ( DerivedProperties::AbstractPropertyManager& propertyManager,
                                     const DataModel::AbstractSnapshot*          snapshot,
                                     const DataModel::AbstractFormation*         formation,
                                           FormationPropertyList&                derivedProperties ) const {

   const DataModel::AbstractProperty* depthProp = propertyManager.getProperty ( "Depth" );
   const DataModel::AbstractProperty* pressureProp = propertyManager.getProperty ( "Pressure" );
   const FormationPropertyPtr depth = propertyManager.getFormationProperty ( depthProp, snapshot, formation );

   DerivedFormationPropertyPtr porePressure = DerivedFormationPropertyPtr( new DerivedProperties::DerivedFormationProperty ( pressureProp,
                                                                                                                             snapshot,
                                                                                                                             formation,
                                                                                                                             propertyManager.getMapGrid (),
                                                                                                                             numberOfNodes ));

   double jumpInPressure = ( formation->getName () == "Formation1" ? 0.0 : pressureGradient * 1000.0 * gravity * waterDensity );

   for ( unsigned int i = depth->firstI ( true ); i <= depth->lastI ( true ); ++i ) {

      for ( unsigned int j = depth->firstJ ( true ); j <= depth->lastJ ( true ); ++j ) {

         for ( unsigned int k = depth->firstK (); k <= depth->lastK (); ++k ) {
            porePressure->set ( i, j, k, pressureGradient * gravity * waterDensity * depth->get ( i, j, k ) + jumpInPressure );
         }

      }

   }

   derivedProperties.push_back ( porePressure );
}
