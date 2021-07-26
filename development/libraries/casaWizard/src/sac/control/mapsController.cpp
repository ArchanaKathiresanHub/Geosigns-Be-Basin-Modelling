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
#include "control/lithofractionVisualisationController.h"
#include "control/run3dCaseController.h"
#include "control/scriptRunController.h"

#include "model/functions/sortedByXWellIndices.h"
#include "model/input/cmbProjectReader.h"
#include "model/logger.h"
#include "model/output/LithoMapsInfoGenerator.h"
#include "model/sacScenario.h"
#include "model/scenarioBackup.h"
#include "model/script/Generate3DScenarioScript.h"

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
  lithofractionVisualisationController_{new LithofractionVisualisationController(mapsTab->lithofractionVisualisation(), scenario_, this)}
{
  connect(mapsTab_->buttonExportOptimized(), SIGNAL(clicked()), this, SLOT(slotExportOptimized()));
  connect(mapsTab_->buttonRunOptimized(),    SIGNAL(clicked()), this, SLOT(slotRunOptimized()));  

  connect(mapsTab_->interpolationType(), SIGNAL(currentIndexChanged(int)), this, SLOT(slotInterpolationTypeCurrentIndexChanged(int)));
  connect(mapsTab_->pValue(),            SIGNAL(valueChanged(int)),        this, SLOT(slotPvalueChanged(int)));
  connect(mapsTab_->smoothingType(),     SIGNAL(currentIndexChanged(int)), this, SLOT(slotSmoothingTypeCurrentIndexChanged(int)));
  connect(mapsTab_->smoothingRadius(),   SIGNAL(valueChanged(int)),        this, SLOT(slotSmoothingRadiusValueChanged(int)));  
  connect(mapsTab_->createGridsButton(), SIGNAL(clicked()),                this, SLOT(slotGenerateLithoMaps()));

  connect(mapsTab_->activeWellsTable(), SIGNAL(itemSelectionChanged()), this, SLOT(slotUpdateBirdView()));
  connect(mapsTab_->activeWellsTable(), SIGNAL(checkBoxSelectionChanged()), this, SLOT(slotUpdateWellSelection()));
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

void MapsController::slotUpdateTabGUI(int tabID)
{
  if (tabID != static_cast<int>(TabID::Maps))
  {
    return;
  }

  refreshGUI();  
}

void MapsController::validateWellsHaveOptimized()
{
  for (int i = 0; i < mapsTab_->numberOfActiveWells(); i++)
  {
    if (!hasOptimizedSuccessfully(i))
    {
      mapsTab_->disableWellAtIndex(i);
    }
  }
}

bool MapsController::hasOptimizedSuccessfully(const int index) const
{
  const CalibrationTargetManager& ctManager = scenario_.calibrationTargetManager();
  const QVector<int> sortedIndices = casaWizard::functions::sortedByXWellIndices(ctManager.activeWells());

  QFile successFile(scenario_.calibrationDirectory() + "/" + scenario_.runLocation() + "/" + scenario_.iterationDirName() + "/Case_" + QString::number(sortedIndices[index] + 1) + "/Stage_0.sh.success");

  return successFile.exists();
}

void MapsController::refreshGUI()
{
  mapsTab_->interpolationType()->setCurrentIndex(scenario_.interpolationMethod());
  mapsTab_->pValue()->setValue(scenario_.pIDW());
  mapsTab_->smoothingType()->setCurrentIndex(scenario_.smoothingOption());
  mapsTab_->smoothingRadius()->setValue(scenario_.radiusSmoothing());
  mapsTab_->updateSelectedWells({});

  emit signalRefreshChildWidgets();
  validateWellsHaveOptimized();
}


void MapsController::slotUpdateWellSelection()
{
  lithofractionVisualisationController_->updateBirdsView();
}

void MapsController::slotGenerateLithoMaps()
{
  Logger::log() << "Start saving optimized case" << Logger::endl();

  scenarioBackup::backup(scenario_);
  Generate3DScenarioScript saveOptimized{scenario_};
  if (!casaScriptWriter::writeCasaScript(saveOptimized) ||
      !scriptRunController_.runScript(saveOptimized))
  {
    return;
  }
  Logger::log() << "Done!" << Logger::endl();
  scenarioBackup::backup(scenario_);

  refreshGUI();
}

void MapsController::slotExportOptimized()
{
  if (scenario_.project3dPath().isEmpty())
  {
    return;
  }

  QDir sourceDir(scenario_.calibrationDirectory() + "/ThreeDFromOneD");

  CMBProjectReader projectReader;
  LithoMapsInfoGenerator lithoMapsInfoGenerator(scenario_, projectReader);

  functions::exportScenarioToZip(sourceDir, scenario_.workingDirectory(), scenario_.project3dFilename(), lithoMapsInfoGenerator);
}

void MapsController::slotRunOptimized()
{
  if (scenario_.project3dPath().isEmpty())
  {
    return;
  }

  const QString baseDirectory{scenario_.calibrationDirectory() + "/ThreeDFromOneD"};

  Run3dCaseController run3dCaseController(scenario_, scriptRunController_);
  if (run3dCaseController.run3dCase(baseDirectory, true))
  {
    scenarioBackup::backup(scenario_);
  }
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
