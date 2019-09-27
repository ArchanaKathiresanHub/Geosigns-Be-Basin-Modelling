#include "model/influentialParameterManager.h"

#include "model/scenarioReader.h"
#include "model/scenarioWriter.h"

#include "model/influentialParameter/crustThinning.h"
#include "model/influentialParameter/crustThinningOneEvent.h"
#include "model/influentialParameter/equilibriumOceanicLithosphereThickness.h"
#include "model/influentialParameter/initialLithosphericMantleThickness.h"
#include "model/influentialParameter/surfaceTemperature.h"
#include "model/influentialParameter/thermalConductivity.h"
#include "model/influentialParameter/topCrustHeatProduction.h"
#include "model/influentialParameter/topCrustHeatProductionGrid.h"
#include "model/influentialParameter/topCrustHeatProductionGridScaling.h"

#include "stubProjectReader.h"

#include "expectFileEq.h"

#include <gtest/gtest.h>

TEST( InfluentialParameterManagerTest, testWriteToFile )
{
  casaWizard::StubProjectReader projectReader;
  casaWizard::ua::InfluentialParameterManager manager{projectReader};

  manager.add(0);
  manager.setArguments(0, {10, 11}, {});

  manager.add(1);
  manager.add(2);
  manager.add(3);
  manager.add(4);
  manager.add(5);
  manager.add(6);
  manager.add(7);
  manager.add(8);

  casaWizard::ScenarioWriter writer{"influentialParameterManagerActual.dat"};
  manager.writeToFile(writer);
  writer.close();

  expectFileEq("influentialParameterManagerActual.dat", "influentialParameterManager.dat");
}

TEST( InfluentialParameterManagerTest, testReadFromFile )
{
  casaWizard::StubProjectReader projectReader;
  casaWizard::ua::InfluentialParameterManager manager{projectReader};

  casaWizard::ScenarioReader reader{"influentialParameterManager.dat"};
  manager.readFromFile(reader);

  QVector<casaWizard::ua::InfluentialParameter*> params = manager.influentialParameters();

  ASSERT_EQ(params.size(), 9);

  // Check if the influential parameters are of the expected type
  ASSERT_TRUE( dynamic_cast<casaWizard::ua::TopCrustHeatProduction*>(params[0]));
  EXPECT_DOUBLE_EQ(params[0]->arguments().getDoubleArgument(0), 10);
  EXPECT_DOUBLE_EQ(params[0]->arguments().getDoubleArgument(1), 11);

  EXPECT_TRUE( dynamic_cast<casaWizard::ua::TopCrustHeatProductionGrid*>(params[1]));
  EXPECT_TRUE( dynamic_cast<casaWizard::ua::TopCrustHeatProductionGridScaling*>(params[2]));
  EXPECT_TRUE( dynamic_cast<casaWizard::ua::CrustThinning*>(params[3]));
  EXPECT_TRUE( dynamic_cast<casaWizard::ua::CrustThinningOneEvent*>(params[4]));
  EXPECT_TRUE( dynamic_cast<casaWizard::ua::ThermalConductivity*>(params[5]));
  EXPECT_TRUE( dynamic_cast<casaWizard::ua::EquilibriumOceanicLithosphereThickness*>(params[6]));
  EXPECT_TRUE( dynamic_cast<casaWizard::ua::InitialLithosphericMantleThickness*>(params[7]));
  EXPECT_TRUE( dynamic_cast<casaWizard::ua::SurfaceTemperature*>(params[8]));

}

