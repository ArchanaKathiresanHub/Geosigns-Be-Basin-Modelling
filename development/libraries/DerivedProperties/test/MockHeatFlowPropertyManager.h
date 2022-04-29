//
// Copyright (C) 2022-2022 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by CGI.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// std headers
#include <vector>
#include <string>

#include "../src/AbstractPropertyManager.h"
#include "../src/FormationPropertyCalculator.h"
#include "../src/DerivedFormationProperty.h"

#include "MockPropertyCalculators.h"

#include "MockFormation.h"
#include "MockSnapshot.h"
#include "MockProperty.h"
#include "MockGrid.h"

//GeoPhysics
#include "GeoPhysicsObjectFactory.h"

using namespace DataAccess;
using namespace Interface;
using namespace DataModel;
using namespace AbstractDerivedProperties;

class MockHeatFlowPropertyManager : public AbstractPropertyManager
{
public :

   explicit MockHeatFlowPropertyManager (GeoPhysics::ProjectHandle* projectHandle);

   ~MockHeatFlowPropertyManager() = default;

   const DataModel::AbstractProperty* getProperty ( const std::string& name ) const final;

   const DataModel::AbstractGrid* getMocMapGrid () const ;

   /// \brief Return whether or not the node is defined.
   bool getNodeIsValid ( const unsigned int i, const unsigned int j ) const final;

   const DataModel::AbstractGrid* getMapGrid() const final;

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

   /// \brief Map grid of nodes, do we need to mock?
   const std::shared_ptr<DataModel::AbstractGrid> m_mapMocGrid = std::make_shared<DataModel::MockGrid>(0, 0, 0, 0, 10, 10, 10, 10);

   /// <summary>
   /// map from the scenario, no mocking
   /// </summary>
   const DataModel::AbstractGrid* m_mapGrid = nullptr;

   /// <summary>
   /// All the property thats needs mock up calculations goes here
   /// [0] -> Depth,
   /// [1] -> Temperature,
   /// [2] -> Pressure,
   /// [3] -> Ves,
   /// [4] -> MaxVes,
   /// [5] -> OverPressure,
   /// [6] -> LithoPressure
   /// [7] -> hydroStaticPressure
   /// </summary>
   const std::vector<std::shared_ptr<MockPropertyCalculator>> m_propertyCalculators = {
	std::make_shared<MockDepthCalculator>(),
	std::make_shared<MockTemperatureCalculator>(),
	std::make_shared<MockPressureCalculator>(),
	std::make_shared<MockVesCalculator>(),
	std::make_shared<MockMaxVesCalculator>(),
	std::make_shared<MockOverPressureCalculator>(),
	std::make_shared<MockLithoPressureCalculator>(),
	std::make_shared<MockHydrostaticCalculator>()
   };

   /// \brief The geophysics project handle
      GeoPhysics::ProjectHandle& m_projectHandle;
};