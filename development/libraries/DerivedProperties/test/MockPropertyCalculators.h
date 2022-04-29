//
// Copyright (C) 2022-2022 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by CGI.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "../src/AbstractPropertyManager.h"
#include "../src/FormationPropertyCalculator.h"
#include "../src/DerivedFormationProperty.h"

// utilities library
#include "ConstantsMathematics.h"

using namespace DataAccess;
using namespace Interface;
using namespace DataModel;
using namespace DerivedProperties;
using namespace AbstractDerivedProperties;

// All the mock calculators goes here

class MockPropertyCalculator : public FormationPropertyCalculator
{
public:
	virtual void calculate(AbstractPropertyManager& propertyManager,
		const DataModel::AbstractSnapshot* snapshot,
		const DataModel::AbstractFormation* formation,
		FormationPropertyList& derivedProperties) const = 0;
	virtual ~MockPropertyCalculator() = default;
/// <summary>
/// one can extend this class with all sorts of add-on methods:
/// ex. setNumberOfNodes(int N) that sets the z direction discretization
/// ex. virtual void setPropertyValue(double value);
/// ex. virtusl void setCaulculationParametrs(struct& paramsSet);
/// </summary>
protected:
	int m_numberOfNodes = 10;

};

class MockDepthCalculator : public MockPropertyCalculator
{
public:

	MockDepthCalculator() {
		addPropertyName("Depth");
	}

	void calculate(AbstractPropertyManager& propertyManager,
		const DataModel::AbstractSnapshot* snapshot,
		const DataModel::AbstractFormation* formation,
		FormationPropertyList& derivedProperties) const final {

		const DataModel::AbstractProperty* depth = propertyManager.getProperty("Depth");
		DerivedFormationPropertyPtr depthProp =
			DerivedFormationPropertyPtr(
				new DerivedFormationProperty(depth, 
					snapshot, 
					formation, 
					propertyManager.getMapGrid(), 
					m_numberOfNodes)
			);

		double depthValue = m_topDepth;

		for (unsigned int i = depthProp->firstI(true); i <= depthProp->lastI(true); ++i) {

			for (unsigned int j = depthProp->firstJ(true); j <= depthProp->lastJ(true); ++j)
			{

				for (unsigned int k = depthProp->firstK(); k <= depthProp->lastK(); ++k)
				{
					depthProp->set(i, j, k, depthValue);
					depthValue += m_depthDelta;
				}
			}
		}
		derivedProperties.push_back(depthProp);
	};

private:
	double m_topDepth = 100.;
	double m_depthDelta = 10.;
};

class MockTemperatureCalculator : public MockPropertyCalculator
{
public:

	MockTemperatureCalculator()
	{
		addPropertyName("Temperature");
	}

	void calculate(AbstractPropertyManager& propertyManager,
		const DataModel::AbstractSnapshot* snapshot,
		const DataModel::AbstractFormation* formation,
		FormationPropertyList& derivedProperties) const final {

		const DataModel::AbstractProperty* depthProp = propertyManager.getProperty("Depth");
		const DataModel::AbstractProperty* tempProp = propertyManager.getProperty("Temperature");
		const FormationPropertyPtr depth = propertyManager.getFormationProperty(depthProp, snapshot, formation);

		DerivedFormationPropertyPtr temp = DerivedFormationPropertyPtr(new DerivedFormationProperty(tempProp,
			snapshot,
			formation,
			propertyManager.getMapGrid(),
			m_numberOfNodes));

		for (unsigned int i = depth->firstI(true); i <= depth->lastI(true); ++i) {

			for (unsigned int j = depth->firstJ(true); j <= depth->lastJ(true); ++j) {

				for (unsigned int k = depth->firstK(); k <= depth->lastK(); ++k) {
					temp->set(i, j, k, m_tempGradient * depth->get(i, j, k) * 0.001); //0.001 is for Km->m unit conversion
				}
			}
		}
		derivedProperties.push_back(temp);
	};

private:
	double m_tempGradient = 30; // 30oC /KM

};

class MockHydrostaticCalculator : public MockPropertyCalculator {
public:
	MockHydrostaticCalculator() {
		addPropertyName("HydroStaticPressure");
	}
	void calculate(AbstractPropertyManager& propertyManager,
		const DataModel::AbstractSnapshot* snapshot,
		const DataModel::AbstractFormation* formation,
		FormationPropertyList& derivedProperties) const final {

		const DataModel::AbstractProperty* depthProp = propertyManager.getProperty("Depth");
		const DataModel::AbstractProperty* hydrostaticPressureProp = propertyManager.getProperty("HydroStaticPressure");
		const FormationPropertyPtr depth = propertyManager.getFormationProperty(depthProp, snapshot, formation);

		DerivedFormationPropertyPtr hydrostaticPressure = DerivedFormationPropertyPtr(new DerivedFormationProperty(hydrostaticPressureProp,
			snapshot,
			formation,
			propertyManager.getMapGrid(),
			m_numberOfNodes));

		for (unsigned int i = depth->firstI(true); i <= depth->lastI(true); ++i) {

			for (unsigned int j = depth->firstJ(true); j <= depth->lastJ(true); ++j) {

				for (unsigned int k = depth->firstK(); k <= depth->lastK(); ++k) {
					hydrostaticPressure->set(i, j, k, m_gravity * m_waterDensity * depth->get(i, j, k));
				}

			}

		}
		derivedProperties.push_back(hydrostaticPressure);
	}

private:
	double m_waterDensity = 1000.0;
	double m_gravity = 10.;
};

class MockLithoPressureCalculator : public MockPropertyCalculator
{
public:
	MockLithoPressureCalculator() {
		addPropertyName("LithoStaticPressure");
	}

	void calculate(AbstractPropertyManager& propertyManager,
		const DataModel::AbstractSnapshot* snapshot,
		const DataModel::AbstractFormation* formation,
		FormationPropertyList& derivedProperties) const final {

		const DataModel::AbstractProperty* depthProp = propertyManager.getProperty("Depth");
		const DataModel::AbstractProperty* LithoPressureProp = propertyManager.getProperty("LithoStaticPressure");
		const FormationPropertyPtr depth = propertyManager.getFormationProperty(depthProp, snapshot, formation);

		DerivedFormationPropertyPtr lithoPressure = DerivedFormationPropertyPtr(new DerivedFormationProperty(LithoPressureProp,
			snapshot,
			formation,
			propertyManager.getMapGrid(),
			m_numberOfNodes));

		for (unsigned int i = depth->firstI(true); i <= depth->lastI(true); ++i) {

			for (unsigned int j = depth->firstJ(true); j <= depth->lastJ(true); ++j) {

				for (unsigned int k = depth->firstK(); k <= depth->lastK(); ++k) {
					lithoPressure->set(i, j, k, m_gravity * m_rockDensity * depth->get(i, j, k));
				}

			}

		}
		derivedProperties.push_back(lithoPressure);
	};
private:
	double m_rockDensity = 1900.0;
	double m_gravity = 10.;
};

class MockPressureCalculator : public MockPropertyCalculator
{
public:

	MockPressureCalculator() {
		addPropertyName("Pressure");
	}

	void calculate(AbstractPropertyManager& propertyManager,
		const DataModel::AbstractSnapshot* snapshot,
		const DataModel::AbstractFormation* formation,
		FormationPropertyList& derivedProperties) const final {

		const DataModel::AbstractProperty* depthProp = propertyManager.getProperty("Depth");
		const DataModel::AbstractProperty* hydPressureProp = propertyManager.getProperty("HydroStaticPressure");


		const DataModel::AbstractProperty* pressureProp = propertyManager.getProperty("Pressure");
		const FormationPropertyPtr depth = propertyManager.getFormationProperty(depthProp, snapshot, formation);
		const FormationPropertyPtr hydPres = propertyManager.getFormationProperty(hydPressureProp, snapshot, formation);

		DerivedFormationPropertyPtr porePressure = DerivedFormationPropertyPtr(new DerivedFormationProperty(pressureProp,
			snapshot,
			formation,
			propertyManager.getMapGrid(),
			m_numberOfNodes));

		for (unsigned int i = depth->firstI(true); i <= depth->lastI(true); ++i) {

			for (unsigned int j = depth->firstJ(true); j <= depth->lastJ(true); ++j) {

				for (unsigned int k = depth->firstK(); k <= depth->lastK(); ++k) {
					auto hydPressureValue = hydPres->get(i, j, k);
					porePressure->set(i, j, k, m_pressureGradient * hydPressureValue);
				}

			}

		}

		derivedProperties.push_back(porePressure);
	};

private:
	double m_pressureGradient = 1.5;
};

class MockVesCalculator : public MockPropertyCalculator
{
public:

	MockVesCalculator() {
		addPropertyName("Ves");
	}

	void calculate(AbstractPropertyManager& propertyManager,
		const DataModel::AbstractSnapshot* snapshot,
		const DataModel::AbstractFormation* formation,
		FormationPropertyList& derivedProperties) const final {

		const DataModel::AbstractProperty* depthProp = propertyManager.getProperty("Depth");
		const DataModel::AbstractProperty* LithoPressureProp = propertyManager.getProperty("LithoStaticPressure");
		const DataModel::AbstractProperty* porePressureProp = propertyManager.getProperty("Pressure");

		const FormationPropertyPtr depth = propertyManager.getFormationProperty(depthProp, snapshot, formation);
		const FormationPropertyPtr porePressure = propertyManager.getFormationProperty(porePressureProp, snapshot, formation);
		const FormationPropertyPtr lithostaticPressure = propertyManager.getFormationProperty(LithoPressureProp, snapshot, formation);

		const DataModel::AbstractProperty* vesProp = propertyManager.getProperty("Ves");
		DerivedFormationPropertyPtr ves = DerivedFormationPropertyPtr(new DerivedFormationProperty(vesProp,
			snapshot,
			formation,
			propertyManager.getMapGrid(),
			m_numberOfNodes));

		for (unsigned int i = depth->firstI(true); i <= depth->lastI(true); ++i) {

			for (unsigned int j = depth->firstJ(true); j <= depth->lastJ(true); ++j) {

				for (unsigned int k = depth->firstK(); k <= depth->lastK(); ++k) {
					ves->set(i, j, k, (lithostaticPressure->get(i, j, k) - porePressure->get(i, j, k)) * Utilities::Maths::MegaPaToPa);
				}

			}

		}

		derivedProperties.push_back(ves);
	};

};

class MockMaxVesCalculator : public MockPropertyCalculator
{
public:

	MockMaxVesCalculator() {
		addPropertyName("MaxVes");
	}

	void calculate(AbstractPropertyManager& propertyManager,
		const DataModel::AbstractSnapshot* snapshot,
		const DataModel::AbstractFormation* formation,
		FormationPropertyList& derivedProperties) const final {

		const DataModel::AbstractProperty* depthProp = propertyManager.getProperty("Depth");
		const DataModel::AbstractProperty* vesProp = propertyManager.getProperty("Ves");
		const FormationPropertyPtr depth = propertyManager.getFormationProperty(depthProp, snapshot, formation);
		const FormationPropertyPtr ves = propertyManager.getFormationProperty(vesProp, snapshot, formation);

		const DataModel::AbstractProperty* MaxVesProp = propertyManager.getProperty("MaxVes");
		DerivedFormationPropertyPtr MaxVes = DerivedFormationPropertyPtr(new DerivedFormationProperty(MaxVesProp,
			snapshot,
			formation,
			propertyManager.getMapGrid(),
			m_numberOfNodes));

		for (unsigned int i = depth->firstI(true); i <= depth->lastI(true); ++i) {

			for (unsigned int j = depth->firstJ(true); j <= depth->lastJ(true); ++j) {

				for (unsigned int k = depth->firstK(); k <= depth->lastK(); ++k) {
					MaxVes->set(i, j, k, m_factor * ves->get(i, j, k));
				}

			}

		}
		derivedProperties.push_back(MaxVes);
	};
private:
	double m_factor = 1.2;
};


class MockOverPressureCalculator : public MockPropertyCalculator
{
public:

	MockOverPressureCalculator() {
		addPropertyName("OverPressure");
	}

	void calculate(AbstractPropertyManager& propertyManager,
		const DataModel::AbstractSnapshot* snapshot,
		const DataModel::AbstractFormation* formation,
		FormationPropertyList& derivedProperties) const final {

	};

};