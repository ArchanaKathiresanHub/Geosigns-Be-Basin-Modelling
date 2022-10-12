//
// Copyright (C) 2022-2022 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by CGI.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "MockHeatFlowPropertyManager.h"
#include "../src/HeatFlowFormationCalculator.h"
#include "../src/OverpressureFormationCalculator.h"

using namespace DataAccess;
using namespace Interface;
using namespace DataModel;
using namespace DerivedProperties;
using namespace AbstractDerivedProperties;

// Constructor
MockHeatFlowPropertyManager::MockHeatFlowPropertyManager(GeoPhysics::ProjectHandle* projectHandle)
	:m_projectHandle(*projectHandle)
{
	// These will come from the project handle.
	m_mockProperties.push_back(std::make_shared < DataModel::MockProperty>("Heat", DataModel::DISCONTINUOUS_3D_PROPERTY));
	m_mockProperties.push_back(std::make_shared < DataModel::MockProperty>("HeatFlowX", DataModel::DISCONTINUOUS_3D_PROPERTY));
	m_mockProperties.push_back(std::make_shared < DataModel::MockProperty>("HeatFlowY", DataModel::DISCONTINUOUS_3D_PROPERTY));
	m_mockProperties.push_back(std::make_shared < DataModel::MockProperty>("HeatFlowZ", DataModel::DISCONTINUOUS_3D_PROPERTY));

	m_mockProperties.push_back(std::make_shared < DataModel::MockProperty>("Depth", DataModel::CONTINUOUS_3D_PROPERTY));
	m_mockProperties.push_back(std::make_shared < DataModel::MockProperty>("Temperature", DataModel::CONTINUOUS_3D_PROPERTY));
	m_mockProperties.push_back(std::make_shared < DataModel::MockProperty>("Pressure", DataModel::CONTINUOUS_3D_PROPERTY));
	m_mockProperties.push_back(std::make_shared < DataModel::MockProperty>("OverPressure", DataModel::CONTINUOUS_3D_PROPERTY));
	m_mockProperties.push_back(std::make_shared < DataModel::MockProperty>("Ves", DataModel::CONTINUOUS_3D_PROPERTY));
	m_mockProperties.push_back(std::make_shared < DataModel::MockProperty>("MaxVes", DataModel::CONTINUOUS_3D_PROPERTY));

	// req. only for basement formations
	m_mockProperties.push_back(std::make_shared < DataModel::MockProperty>("LithoStaticPressure", DataModel::CONTINUOUS_3D_PROPERTY));
	
	m_mockProperties.push_back(std::make_shared < DataModel::MockProperty>("HydroStaticPressure", DataModel::CONTINUOUS_3D_PROPERTY));
	m_mockProperties.push_back(std::make_shared < DataModel::MockProperty>("ChemicalCompaction", DataModel::DISCONTINUOUS_3D_PROPERTY));

	// Add all properties to the property manager.
	// so all mocProperties also become m_property??
	for (const auto & aProp : m_mockProperties)
	{
		addProperty(aProp.get());
	}

	// the grid from the model. no need to mock?
	m_mapGrid = projectHandle->getLowResolutionOutputGrid();
	
	//add the calculators here
	auto depthCalc = m_propertyCalculators[0];
	addFormationPropertyCalculator(FormationPropertyCalculatorPtr(depthCalc));
	auto TempCalc = m_propertyCalculators[1];
	addFormationPropertyCalculator(FormationPropertyCalculatorPtr(TempCalc));
	auto PressureCalc = m_propertyCalculators[2];
	addFormationPropertyCalculator(FormationPropertyCalculatorPtr(PressureCalc));
	auto VesCalc = m_propertyCalculators[3];
	addFormationPropertyCalculator(FormationPropertyCalculatorPtr(VesCalc));
	auto MaxVesCalc = m_propertyCalculators[4];
	addFormationPropertyCalculator(FormationPropertyCalculatorPtr(MaxVesCalc));

	/// there is an Overpressure calculator already, no need for the mock

	addFormationPropertyCalculator(FormationPropertyCalculatorPtr(std::make_shared <OverpressureFormationCalculator>()));
	auto LithoPressure = m_propertyCalculators[6];
	addFormationPropertyCalculator(FormationPropertyCalculatorPtr(LithoPressure));
	auto HydPressure = m_propertyCalculators[7];
	addFormationPropertyCalculator(FormationPropertyCalculatorPtr(HydPressure));

	addFormationPropertyCalculator(FormationPropertyCalculatorPtr(std::make_shared< HeatFlowFormationCalculator>(*projectHandle)), false);
}

const DataModel::AbstractProperty* MockHeatFlowPropertyManager::getProperty(const std::string& name) const
{
	for (const auto & aProp : m_properties)
	{
		if (!aProp->getName().compare(name))
		{
			return aProp;
		}
	}
	return nullptr;
}

const DataModel::AbstractGrid* MockHeatFlowPropertyManager::getMocMapGrid() const
{
	return m_mapMocGrid.get();
}

bool MockHeatFlowPropertyManager::getNodeIsValid(const unsigned int i, const unsigned int j) const
{
	// clever way to use some undefined nodes, actually, one should use m_projectHandle.getNodeIsValid(i, j);
	if ((i + j) % 2 == 0) {
		return true;
	}
	else {
		return false;
	}

}

const DataModel::AbstractGrid* MockHeatFlowPropertyManager::getMapGrid() const
{
	return m_mapGrid;
}

void MockHeatFlowPropertyManager::addProperty(const DataModel::AbstractProperty* property)
{
	if (std::find(m_properties.begin(), m_properties.end(), property) == m_properties.end())
	{
		m_properties.push_back(property);
	}
}