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
#include "../src/HeatFlowFormationCalculator.h"
#include "GeoPhysicsFormation.h"
#include "Snapshot.h"

#include "MockHeatFlowPropertyManager.h"


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

/// For heatflowCalculator we need
/// vesVector,
/// maxVesVector,
/// overpressureVector,
/// porePressureVector,
/// lithoPressureVector,
/// temperatureVector,
/// chemCompactionVector,

class HeatFlowCalculatorTest : public ::testing::Test
{
public:
	HeatFlowCalculatorTest();
	~HeatFlowCalculatorTest() override {
		if (m_projectHandle)
		{
			m_projectHandle->finishActivity(false);
		}
		m_projectHandle.reset();
		m_factory.reset();
		PetscFinalize();
	};

	/// \brief Project Handle - stores the data from the project3d file
	std::unique_ptr<GeoPhysics::ProjectHandle> m_projectHandle;

	/// \brief Geophysics object factory
	std::shared_ptr<GeoPhysics::ObjectFactory> m_factory = std::make_shared<GeoPhysics::ObjectFactory>();

};

HeatFlowCalculatorTest::HeatFlowCalculatorTest()
{
	// Initialize PETSc
	PetscInitialize(nullptr, nullptr, nullptr, PETSC_IGNORE);
	int rank;
	MPI_Comm_rank(PETSC_COMM_WORLD, &rank);
	// Setting up the project handle cause HeatFlowFormationCalculator contains pHandle in ctor
	m_projectHandle.reset(dynamic_cast<GeoPhysics::ProjectHandle*>(OpenCauldronProject("01_simple_50km_HF_test.project3d", m_factory.get())));
	const Interface::Grid* grid = m_projectHandle->getLowResolutionOutputGrid();

	// Initialize the project handle
	auto started = m_projectHandle->startActivity("Test_HeatFlow_Calculator", grid, false, false);

}

TEST_F(HeatFlowCalculatorTest, HFOutsideAoI)
{
	MockHeatFlowPropertyManager propertyManager(m_projectHandle.get());

	// Since we have the projectHandle here we do not need to moc the formation and snapshots now, we just retrieve it from p3files
	const DataModel::AbstractProperty* HFPropX = propertyManager.getProperty("HeatFlowX");
	assert(HFPropX);

	FormationList* formations = m_projectHandle->getFormations();
	assert(formations);
	assert(formations->size() > 0);

	Interface::SnapshotList* snapshots = m_projectHandle->getSnapshots(Interface::MAJOR);
	assert(snapshots);
	assert(snapshots->size() > 0);

	unsigned int l = 0;
	// can do for all formation in the scenario, like, for ( l = 0; l < formations->size(); ++l)
#ifdef needs_debugging
	std::cout << (*formations)[l]->getName() << '\n';
#endif

	auto nonConstformation = const_cast<DataAccess::Interface::Formation*>((*formations)[0]);
	const unsigned int numberOfSegments = dynamic_cast<GeoPhysics::GeoPhysicsFormation*>(nonConstformation)->setMaximumNumberOfElements(false);


	// can do for all snapshots in the scenario like, for (const auto& Asnapshot : *snapshots)

	// present day map
	const FormationPropertyPtr hf_x = propertyManager.getFormationProperty(HFPropX, *(snapshots->begin()), (*formations)[l]);

	// only interior, hence last-1
	for (unsigned int i = hf_x->firstI(true); i <= hf_x->lastI(true) - 1; ++i)
	{
		for (unsigned int j = hf_x->firstJ(true); j <= hf_x->lastJ(true) - 1; ++j)
		{
#ifdef needs_debugging
			for (unsigned int k = hf_x->firstK(); k <= hf_x->lastK(); ++k)
			{
				std::cout << i << ' ' << j << ' ' << hf_x->get(i, j, 0) << std::endl;

			}
#endif
			// since we have not initialized the Element structure all values inside the domain
			// will be Undefined values
			EXPECT_DOUBLE_EQ(hf_x->get(i, j, 0), DataAccess::Interface::DefaultUndefinedMapValue);

		}
	}
	delete formations;
	delete snapshots;
}
