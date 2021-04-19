//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "mapsController.h"

#include "control/activeWellsController.h"
#include "control/casaScriptWriter.h"
#include "control/functions/folderOperations.h"
#include "control/scriptRunController.h"
#include "control/lithofractionVisualisationController.h"

#include "model/case3DTrajectoryConvertor.h"
#include "model/input/case3DTrajectoryReader.h"
#include "model/input/cmbProjectReader.h"
#include "model/logger.h"
#include "model/output/LithoMapsInfoGenerator.h"
#include "model/sacScenario.h"
#include "model/scenarioBackup.h"
#include "model/script/cauldronScript.h"
#include "model/script/Generate3DScenarioScript.h"
#include "model/script/track1dAllWellScript.h"

#include "view/activeWellsTable.h"
#include "view/lithofractionVisualisation.h"
#include "view/mapsTab.h"
#include "view/sacTabIDs.h"

#include <QComboBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QListWidget>
#include <QPushButton>
#include <QSpinBox>
#include <QVector>

namespace casaWizard
{

namespace sac
{

MapsController::MapsController(MapsTab* mapsTab,
                               SACScenario& scenario,
                               ScriptRunController& scriptRunController,
                               QObject* parent) :
  QObject(parent),
  mapsTab_{mapsTab},
  scenario_{scenario},
  scriptRunController_{scriptRunController},
  activeWellsController_{new ActiveWellsController(mapsTab->activeWellsTable(), scenario_, this)},
  lithofractionVisualisationController_{new LithofractionVisualisationController(mapsTab->lithofractionVisualisation(), scenario_, this)},
  selectedWell_{0}
{
  connect(mapsTab_->buttonExportOptimized(), SIGNAL(clicked()), this, SLOT(slotExportOptimized()));
  connect(mapsTab_->buttonRunOptimized(),    SIGNAL(clicked()), this, SLOT(slotRunOptimized()));
  connect(mapsTab_->buttonRunOriginal(),     SIGNAL(clicked()), this, SLOT(slotRunOriginal()));

  connect(mapsTab_->interpolationType(), SIGNAL(currentIndexChanged(int)), this, SLOT(slotInterpolationTypeCurrentIndexChanged(int)));
  connect(mapsTab_->pValue(),            SIGNAL(valueChanged(int)),        this, SLOT(slotPvalueChanged(int)));
  connect(mapsTab_->smoothingType(),     SIGNAL(currentIndexChanged(int)), this, SLOT(slotSmoothingTypeCurrentIndexChanged(int)));
  connect(mapsTab_->smoothingRadius(),   SIGNAL(valueChanged(int)),        this, SLOT(slotSmoothingRadiusValueChanged(int)));
  connect(mapsTab_->threads(),           SIGNAL(valueChanged(int)),        this, SLOT(slotThreadsValueChanged(int)));
  connect(mapsTab_->createGridsButton(), SIGNAL(clicked()),                this, SLOT(slotGenerateLithoMaps()));

  connect(mapsTab_->activeWellsTable(), SIGNAL(itemSelectionChanged()), this, SLOT(slotUpdateBirdView()));
  connect(mapsTab_->activeWellsTable(), SIGNAL(selectedWell(const QString&)), this, SLOT(slotUpdateWell(const QString&)));
}

void MapsController::slotInterpolationTypeCurrentIndexChanged(int interpolationType)
{
  scenario_.setInterpolationMethod(interpolationType);
}

void MapsController::slotPvalueChanged(int pIDW)
{
  scenario_.setPIDW(pIDW);
}

void MapsController::slotSmoothingTypeCurrentIndexChanged(int smoothingType)
{
  scenario_.setSmoothingOption(smoothingType);
}

void MapsController::slotSmoothingRadiusValueChanged(int smoothingRadius)
{
  scenario_.setRadiusSmoothing(smoothingRadius);
}

void MapsController::slotThreadsValueChanged(int threads)
{
  scenario_.setThreadsSmoothing(threads);
}

void MapsController::refreshGUI()
{
  mapsTab_->interpolationType()->setCurrentIndex(scenario_.interpolationMethod());
  mapsTab_->pValue()->setValue(scenario_.pIDW());
  mapsTab_->smoothingType()->setCurrentIndex(scenario_.smoothingOption());
  mapsTab_->smoothingRadius()->setValue(scenario_.radiusSmoothing());
  mapsTab_->threads()->setValue(scenario_.threadsSmoothing());
  mapsTab_->updateSelectedWells({});

  emit signalRefreshChildWidgets();
}

void MapsController::slotUpdateTabGUI(int tabID)
{
  if (tabID != static_cast<int>(TabID::Maps))
  {
    return;
  }

  refreshGUI();  
}

void MapsController::slotUpdateWell(const QString& name)
{
  selectedWell_ = -1;
  const CalibrationTargetManager& ctManager = scenario_.calibrationTargetManager();
  for (const Well* well : ctManager.activeWells())
  {
    if (well->name() == name)
    {
      selectedWell_ = well->id();
      break;
    }
  }

  refreshGUI();  
}

void MapsController::slotGenerateLithoMaps()
{
  Logger::log() << "Start saving optimized case" << Logger::endl();

  Generate3DScenarioScript saveOptimized{scenario_};
  if (!casaScriptWriter::writeCasaScript(saveOptimized) ||
      !scriptRunController_.runScript(saveOptimized))
  {
    return;
  }

  refreshGUI();
}

void MapsController::slotExportOptimized()
{
  QDir sourceDir(scenario_.calibrationDirectory() + "/ThreeDFromOneD");

  CMBProjectReader projectReader;
  LithoMapsInfoGenerator lithoMapsInfoGenerator(scenario_, projectReader);

  functions::exportScenarioToZip(sourceDir, scenario_.workingDirectory(), scenario_.project3dFilename(), lithoMapsInfoGenerator);
}

void MapsController::slotRunOptimized()
{
  scenarioBackup::backup(scenario_);

  const QString baseDirectory{scenario_.calibrationDirectory() + "/ThreeDFromOneD"};

  if (run3dCase(baseDirectory) && import3dWellData(baseDirectory, true))
  {
    scenarioBackup::backup(scenario_);
  }
}

void MapsController::slotRunOriginal()
{
  scenarioBackup::backup(scenario_);

  const QString runDirectory{scenario_.calibrationDirectory() + "/ThreeDBase"};

  const QDir sourceDir(scenario_.workingDirectory());
  const QDir targetDir(runDirectory);
  if (!sourceDir.exists())
  {
    Logger::log() << "Source directory " + sourceDir.absolutePath() + " not found" << Logger::endl();
    return;
  }

  const bool filesCopied = functions::copyCaseFolder(sourceDir, targetDir);
  if (!filesCopied)
  {
    Logger::log() << "Failed to create the 3D original case"
                  << "\nThe original is not run" << Logger::endl();
    return;
  }

  if (run3dCase(runDirectory) && import3dWellData(runDirectory, false) )
  {
    scenarioBackup::backup(scenario_);
  }
}

bool MapsController::run3dCase(const QString directory)
{
  bool ok = true;
  const int cores = QInputDialog::getInt(nullptr, "Number of cores", "Cores", scenario_.numberCPUs(), 1, 48, 1, &ok);
  if (!ok)
  {
    return false;
  }

  scenario_.setNumberCPUs(cores);  
  CauldronScript cauldron{scenario_, directory};
  if (!casaScriptWriter::writeCasaScript(cauldron) ||
      !scriptRunController_.runScript(cauldron))
  {
    return false;
  }

  return true;
}

bool MapsController::import3dWellData(const QString baseDirectory, const bool isOptimized)
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
  QStringList properties = scenario_.calibrationTargetManager().activeProperties();

  Track1DAllWellScript import{baseDirectory,
                              xCoordinates,
                              yCoordinates,
                              properties,
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
    Logger::log() << "Failed to read file" << e.what() << Logger::endl();
    return false;
  }

  case3DTrajectoryConvertor::convertToScenario(reader, scenario_, isOptimized);

  Logger::log() << "Finished extracting data from the 3D case" << Logger::endl();
  return true;
}

void MapsController::slotUpdateBirdView()
{
  mapsTab_->updateSelectedWells(selectedWells());
}

QVector<int> MapsController::selectedWells()
{
  const QVector<int> wellIndices = getSelectedWellIndices();
  const QVector<int> excludedWells = getExcludedWells();

  return transformToActiveAndIncluded(wellIndices, excludedWells);
}

QVector<int> MapsController::getSelectedWellIndices()
{
  QModelIndexList indices = mapsTab_->activeWellsTable()->selectionModel()->selectedIndexes();

  QVector<int> wellIndices;
  for(const QModelIndex& index : indices)
  {
    wellIndices.push_back(index.row());
  }

  return wellIndices;
}

QVector<int> MapsController::getExcludedWells()
{
  QVector<int> excludedWells;
  int counter = 0;
  for (const Well* well : scenario_.calibrationTargetManager().activeWells())
  {
    if (well->isExcluded())
    {
      excludedWells.push_back(counter);
    }
    counter++;
  }

  return excludedWells;
}

QVector<int> MapsController::transformToActiveAndIncluded(const QVector<int>& wellIndices, const QVector<int>& excludedWells)
{
  QVector<int> wellIndicesActiveIncluded;
  for (int wellIndex : wellIndices)
  {
    int exclusionShift = 0;
    bool excluded = false;
    for (int excludedWellIndex : excludedWells)
    {
      if(wellIndex == excludedWellIndex) excluded = true;
      if(wellIndex >  excludedWellIndex) exclusionShift++;
    }

    if (!excluded)
    {
      wellIndicesActiveIncluded.push_back(wellIndex - exclusionShift);
    }
  }

  return wellIndicesActiveIncluded;
}

} // namespace sac

} // namespace casaWizard
