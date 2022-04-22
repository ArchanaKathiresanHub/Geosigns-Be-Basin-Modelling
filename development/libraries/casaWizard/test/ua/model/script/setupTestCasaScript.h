// Setting up the casa scenario for the writing scripts
// Setup functions are creating the statefiles.
// The existence of these file is checked before writing the script.
#pragma once

#include "model/doeOption.h"
#include "model/influentialParameterManager.h"
#include "model/uaScenario.h"
#include "stubProjectReader.h"

#include <QObject>
#include <gtest/gtest.h>
#include <fstream>

class SetupTestCasaScript : public ::testing::Test
{
public:
  casaWizard::ua::UAScenario scenario{new casaWizard::StubProjectReader()};
  std::string stateFile_ = "stateFileTestCasaScriptDoE.txt";
  void SetUp() override
  {
    scenario.setWorkingDirectory(".");
    scenario.setProject3dFileNameAndLoadFile("projStub.project3d");
    scenario.setStateFileNameDoE(QString(stateFile_.c_str()));

    casaWizard::ua::InfluentialParameterManager& ipManager = scenario.influentialParameterManager();
    ipManager.add();

    casaWizard::ua::PredictionTargetManager& ptManager = scenario.predictionTargetManager();
    ptManager.addDepthTarget(1000.0, 1500.0, 100.0, {"T"}); // With time series
    ptManager.setTargetHasTimeSeries(0, true);
    ptManager.addDepthTarget(1000.0, 1500.0, 400.0, {"V"}); // Without time series (present day only)

    // Tornado doe
    scenario.setIsDoeOptionSelected(0, true);

    // SpaceFilling doe
    scenario.setIsDoeOptionSelected(5, true);
    scenario.updateDoeArbitraryNDesignPoints(5, 50);

    // Set doe options for QC (also applied to UA).
    scenario.setIsQcDoeOptionSelected(0, true);
    scenario.setProxyOrder(2);
    scenario.setProxyKrigingMethod("Global");

    casaWizard::ua::ManualDesignPointManager& mdpManager = scenario.manualDesignPointManager();
    mdpManager.addInfluentialParameter();
    mdpManager.addDesignPoint({1.2});

    std::ofstream createFile(stateFile_, std::ios::binary);
    if (createFile.is_open())
    {
      createFile << "Dummy content";
    }
    createFile.close();
  }

  void TearDown() override
  {
    std::remove(stateFile_.c_str());
  }
};
