//
// Copyright (C) 2022-2022 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by CGI.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include <iostream>
#include <vector>
#include <string>
#include "../src/AbstractPropertyManager.h"
#include "../src/FormationPropertyCalculator.h"
#include "../src/DerivedFormationProperty.h"
#include "../src/DepthBelowMudlineFormationCalculator.h"

#include "MockFormation.h"
#include "MockSnapshot.h"
#include "MockProperty.h"
#include "MockGrid.h"

#include <gtest/gtest.h>

// PetSc
#include <petsc.h>

//GeoPhysics
#include "GeoPhysicsObjectFactory.h"

using namespace DataAccess;
using namespace Interface;
using namespace DataModel;
using namespace DerivedProperties;
using namespace AbstractDerivedProperties;

// Parameters to construct a layer with top surface depth as 'topDepth',
// total number of nodes in the layer in depth direction 'numberOfNodes',
// difference between any two nodes in depth direction as 'depthDelta'
static const double topDepth = 1000.0;
static const int numberOfNodes = 11;
static const double depthDelta = 100.0;

class TestPropertyManager : public AbstractPropertyManager 
{
public :

   TestPropertyManager ();

   ~TestPropertyManager() final = default;

   const DataModel::AbstractProperty* getProperty ( const std::string& name ) const final;

   const DataModel::AbstractGrid* getMapGrid () const final;

   /// \brief Return whether or not the node is defined.
   bool getNodeIsValid ( const unsigned int i, const unsigned int j ) const final;

private :

   /// \brief List of all available properties.
   using PropertyList = std::vector <const DataModel::AbstractProperty*>;

   /// \brief Add an abstract property to the list of available properties.
   ///
   /// If a property has been added already then it will not be added a second time.
   void addProperty ( const DataModel::AbstractProperty* property );

   /// \brief Contains list of all known properties.
   PropertyList                 m_properties;
   
   /// \brief list of pointer to properties to be computed
   std::vector<std::shared_ptr<MockProperty>> m_mockProperties;

   /// \brief Map grid of nodes
   const std::shared_ptr<DataModel::AbstractGrid> m_mapGrid;

   /// \brief Geophysics object factory
   std::shared_ptr<GeoPhysics::ObjectFactory> m_factory;

   /// \brief Project Handle - stores the data from the project3d file
   std::unique_ptr<GeoPhysics::ProjectHandle> m_projectHandle;


};

class DepthCalculator : public FormationPropertyCalculator
{
public :

   DepthCalculator ();

   void calculate ( AbstractPropertyManager& propertyManager,
                    const DataModel::AbstractSnapshot*          snapshot,
                    const DataModel::AbstractFormation*         formation,
                          FormationPropertyList&                derivedProperties ) const final;

};

// Constant depth => constant thickness test.
TEST ( DepthBelowMudlineCalculatorTest, Test )
{
    // Initialize PETSc
    PetscInitialize ((int*) 0, (char***) 0, (char *) 0, PETSC_IGNORE);
    int rank;
    MPI_Comm_rank ( PETSC_COMM_WORLD, &rank );
    // Scope the stack variables to clear them before PetscFinalize
    {
        TestPropertyManager propertyManager;

        const DataModel::AbstractProperty* depthBelowMudlineProperty = propertyManager.getProperty("DepthBelowMudline");

        const std::shared_ptr<DataModel::AbstractSnapshot> snapshot = std::make_shared<MockSnapshot>(0.0);
        const std::shared_ptr<DataModel::AbstractFormation> formation = std::make_shared<MockFormation>("Layer");

        const FormationPropertyPtr dbm = propertyManager.getFormationProperty(depthBelowMudlineProperty, snapshot.get(), formation.get());

        // DepthBelowMudline of the top surface of "Layer"
        double expectedDBM = 0;
        for (unsigned int i = dbm->firstI(true); i <= dbm->lastI(true); ++i)
        {
            for (unsigned int j = dbm->firstJ(true); j <= dbm->lastJ(true); ++j)
            {
                EXPECT_DOUBLE_EQ(dbm->get(i, j, 0), expectedDBM);
            }
        }

        // DepthBelowMudline of the mid surface in "Layer"
        expectedDBM = 500;
        for (unsigned int i = dbm->firstI(true); i <= dbm->lastI(true); ++i)
        {
            for (unsigned int j = dbm->firstJ(true); j <= dbm->lastJ(true); ++j)
            {
                EXPECT_DOUBLE_EQ(dbm->get(i, j, 5), expectedDBM);
            }
        }

        // DepthBelowMudline of the bottom surface in "Layer"
        expectedDBM = 1000;
        for (unsigned int i = dbm->firstI(true); i <= dbm->lastI(true); ++i)
        {
            for (unsigned int j = dbm->firstJ(true); j <= dbm->lastJ(true); ++j)
            {
                EXPECT_DOUBLE_EQ(dbm->get(i, j, 10), expectedDBM);
            }
        }
    }
   PetscFinalize ();
}

// Constructor
TestPropertyManager::TestPropertyManager ():
    m_mapGrid(std::make_shared<DataModel::MockGrid>(0, 0, 0, 0, 10, 10, 10, 10)),
    m_factory(std::make_shared<GeoPhysics::ObjectFactory>())
{
   // These will come from the project handle.
   m_mockProperties.push_back(std::make_shared< DataModel::MockProperty>("Depth", DataModel::CONTINUOUS_3D_PROPERTY));
   m_mockProperties.push_back(std::make_shared< DataModel::MockProperty>("DepthBelowMudline", DataModel::CONTINUOUS_3D_PROPERTY));

   // Add all properties to the property manager.
   for ( size_t i = 0; i < m_mockProperties.size (); ++i ) 
   {
      addProperty ( m_mockProperties [ i ].get());
   }

   // Setting up the project handle
   m_projectHandle.reset(dynamic_cast<GeoPhysics::ProjectHandle*>(OpenCauldronProject("DBMProject.project3d", m_factory.get())));
   const Interface::Grid* grid = m_projectHandle->getLowResolutionOutputGrid();

   // Initialize the project handle
   m_projectHandle->startActivity("Test_DBM_Calculator", grid, false, false, false);
   m_projectHandle->initialise();

   addFormationPropertyCalculator ( FormationPropertyCalculatorPtr ( new DepthCalculator )); 
   addFormationPropertyCalculator(FormationPropertyCalculatorPtr(new DepthBelowMudlineFormationCalculator(*m_projectHandle)), false);
}

const DataModel::AbstractProperty* TestPropertyManager::getProperty ( const std::string& name ) const 
{
   for ( size_t i = 0; i < m_properties.size (); ++i ) {

      if ( m_properties [ i ]->getName () == name ) 
      {
         return m_properties [ i ];
      }
   }
   return nullptr;
}

const DataModel::AbstractGrid* TestPropertyManager::getMapGrid () const 
{
   return m_mapGrid.get();
}

bool TestPropertyManager::getNodeIsValid ( const unsigned int i, const unsigned int j ) const 
{ 
   (void) i;
   (void) j;
   return true; 
}

void TestPropertyManager::addProperty ( const DataModel::AbstractProperty* property ) 
{
   if ( std::find ( m_properties.begin (), m_properties.end (), property ) == m_properties.end ()) 
   {
      m_properties.push_back ( property );
   }
}

DepthCalculator::DepthCalculator () 
{
   addPropertyName ( "Depth" );
}

void DepthCalculator::calculate ( AbstractPropertyManager& propertyManager,
                                  const DataModel::AbstractSnapshot*          snapshot,
                                  const DataModel::AbstractFormation*         formation,
                                        FormationPropertyList&                derivedProperties ) const 
{
    const DataModel::AbstractProperty* depth = propertyManager.getProperty ( "Depth" );
    DerivedFormationPropertyPtr depthProp = 
        DerivedFormationPropertyPtr( new DerivedFormationProperty ( depth, snapshot, formation, propertyManager.getMapGrid(), numberOfNodes));
                                                                                                                                                                                                                            
   if ( formation->getName () == "Layer" ) 
   {
      for ( unsigned int i = depthProp->firstI ( true ); i <= depthProp->lastI ( true ); ++i ) 
      {
         for ( unsigned int j = depthProp->firstJ ( true ); j <= depthProp->lastJ ( true ); ++j ) 
         {
            double depthValue = topDepth;
            for ( unsigned int k = depthProp->firstK (); k <= depthProp->lastK (); ++k ) 
            {
                depthProp->set(i, j, k, depthValue);
                depthValue += depthDelta;
            }
         }
      }
   } 
   derivedProperties.push_back ( depthProp );
}
