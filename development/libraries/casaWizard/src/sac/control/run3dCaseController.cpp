//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "run3dCaseController.h"

#include "control/casaScriptWriter.h"
#include "control/scriptRunController.h"

#include "model/case3DTrajectoryConvertor.h"
#include "model/input/case3DTrajectoryReader.h"
#include "model/logger.h"
#include "model/sacScenario.h"
#include "model/scenarioBackup.h"
#include "model/script/cauldronScript.h"
#include "model/script/Generate3DScenarioScript.h"
#include "model/script/track1dAllWellScript.h"
#include "model/output/cmbProjectWriter.h"

#include <QFileDialog>
#include <QInputDialog>
#include <QStringList>

namespace casaWizard
{

namespace sac
{

Run3dCaseController::Run3dCaseController(SACScenario& scenario,
                                         ScriptRunController& scriptRunController) :
  scenario_{scenario},
  scriptRunController_{scriptRunController}
{
}

bool Run3dCaseController::run3dCase(const QString& directory, const bool isOptimized)
{
  bool getCores = false;
  const int cores = QInputDialog::getInt(nullptr, "Number of cores", "Cores", scenario_.numberCPUs(), 1, 1024, 1, &getCores);

  bool getSubSampling = false;
  const int subSampling = QInputDialog::getInt(nullptr, "Subsampling", "", 1, 1, 1024, 1, &getSubSampling);

  if (!getCores || !getSubSampling)
  {
    return false;
  }

  const QString projectFilename{QDir::separator() + QFileInfo(scenario_.project3dPath()).fileName()};
  CMBProjectWriter writer(directory + projectFilename);
  writer.setScaling(subSampling, subSampling);

  scenario_.setNumberCPUs(cores);  
  CauldronScript cauldron{scenario_, directory};
  if (!casaScriptWriter::writeCasaScript(cauldron) ||
      !scriptRunController_.runScript(cauldron))
  {
    QDir(directory).removeRecursively();
    return false;
  }

  return import3dWellData(directory, isOptimized);
}

bool Run3dCaseController::import3dWellData(const QString& baseDirectory, const bool isOptimized)
{
  Logger::log() << "Extracting data from the 3D case using track1d" << Logger::endl();

  QVector<double> xCoordinates;
  QVector<double> yCoordinates;
  const CalibrationTargetManager& calibrationTargetManager = scenario_.calibrationTargetManager();
  const QVector<const Well*> wells = calibrationTargetManager.activeWells();

  for (const Well* well : wells)
  {
    xCoordinates.push_back(well->x());
    yCoordinates.push_back(well->y());
  }
  QStringList propertyUserNames = calibrationTargetManager.activePropertyUserNames();
  QStringList propertyCauldronNames;
  for (const QString& propertyUserName : propertyUserNames)
  {
    propertyCauldronNames.push_back(calibrationTargetManager.getCauldronPropertyName(propertyUserName));
  }

  Track1DAllWellScript import{baseDirectory,
                              xCoordinates,
                              yCoordinates,
                              propertyCauldronNames,
                              scenario_.project3dFilename()};
  if (!scriptRunController_.runScript(import))
  {
    Logger::log() << "Failed to run well import" << Logger::endl();
    return false;
  }

  Case3DTrajectoryReader reader{baseDirectory + "/welldata.csv"};
  try
  {
    reader.read();
  }
  catch(const std::exception& e)
  {
    Logger::log() << e.what() << Logger::endl();
    return false;
  }

  case3DTrajectoryConvertor::convertToScenario(reader, scenario_, isOptimized);

  Logger::log() << "Finished extracting data from the 3D case" << Logger::endl();
  Logger::log() << "Done!" << Logger::endl();
  return true;
}

} // namespace sac

} // namespace casaWizard
