//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "model/influentialParameterManager.h"

#include "model/scenarioReader.h"
#include "model/scenarioWriter.h"

#include "model/influentialParameter/equilibriumOceanicLithosphereThickness.h"
#include "model/influentialParameter/initialLithosphericMantleThickness.h"
#include "model/influentialParameter/surfaceTemperature.h"
#include "model/influentialParameter/thermalConductivity.h"
#include "model/influentialParameter/topCrustHeatProduction.h"
#include "model/influentialParameter/topCrustHeatProductionGrid.h"
#include "model/influentialParameter/topCrustHeatProductionGridScaling.h"

#include "ConstantsNumerical.h"

#include "stubProjectReader.h"

#include "expectFileEq.h"

#include <gtest/gtest.h>

TEST( InfluentialParameterManagerTest, testReadWrite)
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

   {
      casaWizard::ScenarioWriter writer{"influentialParameterManager.dat"};
      manager.writeToFile(writer);
      writer.close();
   }

   casaWizard::ScenarioReader reader{"influentialParameterManager.dat"};
   casaWizard::ua::InfluentialParameterManager managerRead{projectReader};
   managerRead.readFromFile(reader);

   {
      casaWizard::ScenarioWriter writer{"influentialParameterManagerRead.dat"};
      managerRead.writeToFile(writer);
      writer.close();
   }

   expectFileEq("influentialParameterManager.dat", "influentialParameterManagerRead.dat");
}

TEST( InfluentialParameterManagerTest, testReadFromFileV0NoCrustThinning )
{
   casaWizard::StubProjectReader projectReader;
   casaWizard::ua::InfluentialParameterManager manager{projectReader};

   casaWizard::ScenarioReader reader{"influentialParameterManagerV0NoCrustThinning.dat"};
   manager.readFromFile(reader);

   QVector<casaWizard::ua::InfluentialParameter*> params = manager.influentialParameters();
   ASSERT_EQ(params.size(), 7);

   // Check if the influential parameters are of the expected type
   ASSERT_TRUE( dynamic_cast<casaWizard::ua::TopCrustHeatProduction*>(params[0]));
   EXPECT_TRUE( dynamic_cast<casaWizard::ua::TopCrustHeatProductionGrid*>(params[1]));
   EXPECT_TRUE( dynamic_cast<casaWizard::ua::TopCrustHeatProductionGridScaling*>(params[2]));
   EXPECT_TRUE( dynamic_cast<casaWizard::ua::ThermalConductivity*>(params[3]));
   EXPECT_TRUE( dynamic_cast<casaWizard::ua::EquilibriumOceanicLithosphereThickness*>(params[4]));
   EXPECT_TRUE( dynamic_cast<casaWizard::ua::InitialLithosphericMantleThickness*>(params[5]));
   EXPECT_TRUE( dynamic_cast<casaWizard::ua::SurfaceTemperature*>(params[6]));
}

TEST( InfluentialParameterManagerTest, testReadFromFileV0CrustThinning )
{
   casaWizard::StubProjectReader projectReader;
   casaWizard::ua::InfluentialParameterManager manager{projectReader};

   casaWizard::ScenarioReader reader{"influentialParameterManagerV0.dat"};
   EXPECT_THROW(manager.readFromFile(reader), std::runtime_error);
   QVector<casaWizard::ua::InfluentialParameter*> params = manager.influentialParameters();
   ASSERT_EQ(params.size(), 0);
}

TEST( InfluentialParameterManagerTest, testStoreMcmcOutputInParameters)
{
   using namespace casaWizard::ua;

   casaWizard::StubProjectReader projectReader;
   casaWizard::ua::InfluentialParameterManager manager{projectReader};
   manager.add(0);
   manager.add(1);
   manager.add(2);

   { //No data stored
      QVector<InfluentialParameter*> parameters = manager.influentialParameters();
      QVector<double> optimalValuesOut = manager.optimalParameterValues();
      for (int i = 0; i < 2; i++)
      {
         InfluentialParameter* p = parameters[i];
         EXPECT_DOUBLE_EQ(p->optimalValue(),Utilities::Numerical::IbsNoDataValue);
         EXPECT_DOUBLE_EQ(optimalValuesOut[i],Utilities::Numerical::IbsNoDataValue);
      }
   }

   { //Data stored
      QVector<double> optimalValues{1.0,3.0,Utilities::Numerical::IbsNoDataValue};

      QVector<QVector<double>> mcmcInfluentialParameterMatrix{{optimalValues[0],2.0},{optimalValues[1]},{}};
      manager.storeMCMCOutputInParameters(mcmcInfluentialParameterMatrix);

      QVector<InfluentialParameter*> parameters = manager.influentialParameters();
      QVector<double> optimalValuesOut = manager.optimalParameterValues();
      for (int i = 0; i < 2; i++)
      {
         InfluentialParameter* p = parameters[i];
         EXPECT_DOUBLE_EQ(p->optimalValue(),optimalValues[i]);
         EXPECT_DOUBLE_EQ(optimalValuesOut[i],optimalValues[i]);
      }
   }
}

TEST( InfluentialParameterManagerTest, nameLists)
{
   using namespace casaWizard::ua;
   casaWizard::StubProjectReader projectReader;
   InfluentialParameterManager manager{projectReader};

   QStringList expectedLabelNames;
   QStringList expectedNames;
   for (int i = 0; i<7; i++)
   {
      manager.add(i);
      InfluentialParameter* p = InfluentialParameter::createFromIndex(i);
      p->fillArguments(projectReader);
      expectedLabelNames.append(p->labelName());
      expectedNames.append(p->nameArgumentBased());
      delete p;
   }

   QStringList labelsList = manager.labelNameList();
   EXPECT_EQ(expectedLabelNames,labelsList);

   QStringList namesList = manager.nameList();
   EXPECT_EQ(expectedNames,namesList);
}


