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
#include "control/lithofractionVisualisationController.h"
#include "control/run3dCaseController.h"
#include "control/scriptRunController.h"

#include "model/functions/folderOperations.h"
#include "model/input/cmbProjectReader.h"
#include "model/logger.h"
#include "model/output/LithoMapsInfoGenerator.h"
#include "model/sacScenario.h"
#include "model/scenarioBackup.h"
#include "model/script/Generate3DScenarioScript.h"

#include "view/components/customcheckbox.h"
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
  mapsManager_{scenario.mapsManager()},
  mapsTab_{mapsTab},
  scenario_{scenario},
  scriptRunController_{scriptRunController},
  activeWellsController_{new ActiveWellsController(mapsTab->activeWellsTable(), scenario_, this)},
  lithofractionVisualisationController_{new LithofractionVisualisationController(mapsTab->lithofractionVisualisation(), scenario_, this)}
{
  connect(mapsTab_->buttonExportOptimized(), SIGNAL(clicked()), this, SLOT(slotExportOptimized()));
  connect(mapsTab_->buttonRunOptimized(),    SIGNAL(clicked()), this, SLOT(slotRunOptimized()));
  connect(mapsTab_->buttonExportOptimizedToZycor(), SIGNAL(clicked()), this, SLOT(slotExportOptimizedToZycor()));

  connect(mapsTab_->interpolationType(), SIGNAL(currentIndexChanged(int)), this, SLOT(slotInterpolationTypeCurrentIndexChanged(int)));
  connect(mapsTab_->pValue(),            SIGNAL(valueChanged(int)),        this, SLOT(slotPvalueChanged(int)));
  connect(mapsTab_->smoothingType(),     SIGNAL(currentIndexChanged(int)), this, SLOT(slotSmoothingTypeCurrentIndexChanged(int)));
  connect(mapsTab_->smoothingRadius(),   SIGNAL(valueChanged(int)),        this, SLOT(slotSmoothingRadiusValueChanged(int)));  
  connect(mapsTab_->smartGridding(),     SIGNAL(stateChanged(int)),        this, SLOT(slotSmartGriddingChanged(int)));
  connect(mapsTab_->createGridsButton(), SIGNAL(clicked()),                this, SLOT(slotGenerateLithoMaps()));

  connect(mapsTab_->activeWellsTable(), SIGNAL(itemSelectionChanged()), this, SLOT(slotUpdateBirdView()));
  connect(mapsTab_->activeWellsTable(), SIGNAL(checkBoxSelectionChanged()), this, SLOT(slotUpdateWellSelection()));

  connect(lithofractionVisualisationController_, SIGNAL(wellClicked(const QString&)), this, SLOT(slotWellClicked(const QString&)));
  connect(lithofractionVisualisationController_, SIGNAL(clearWellListHighlightSelection()), activeWellsController_, SLOT(slotClearWellListHighlightSelection()));
}

void MapsController::slotInterpolationTypeCurrentIndexChanged(int interpolationType)
{
  mapsManager_.setInterpolationMethod(interpolationType);
}

void MapsController::slotPvalueChanged(int pIDW)
{
  mapsManager_.setPIDW(pIDW);
}

void MapsController::slotSmoothingTypeCurrentIndexChanged(int smoothingType)
{
  mapsManager_.setSmoothingOption(smoothingType);
}

void MapsController::slotSmoothingRadiusValueChanged(int smoothingRadius)
{
  mapsManager_.setRadiusSmoothing(smoothingRadius);
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
    if (!scenario_.hasOptimizedSuccessfully(i))
    {
      mapsTab_->disableWellAtIndex(i);
    }
  }
}

void MapsController::refreshGUI()
{
  mapsTab_->interpolationType()->setCurrentIndex(mapsManager_.interpolationMethod());
  mapsTab_->pValue()->setValue(mapsManager_.pIDW());
  mapsTab_->smoothingType()->setCurrentIndex(mapsManager_.smoothingOption());
  mapsTab_->smoothingRadius()->setValue(mapsManager_.radiusSmoothing());
  lithofractionVisualisationController_->updateSelectedWells({});
  mapsTab_->smartGridding()->setCheckState(mapsManager_.smartGridding() ? Qt::Checked : Qt::Unchecked);

  emit signalRefreshChildWidgets();
  validateWellsHaveOptimized();
}

void MapsController::slotUpdateWellSelection()
{
  lithofractionVisualisationController_->updateBirdsView();
}

void MapsController::slotSmartGriddingChanged(int state)
{
  mapsManager_.setSmartGridding(state == Qt::Checked);
  lithofractionVisualisationController_->hideAllTooltips();
  activeWellsController_->slotClearWellListHighlightSelection();
  lithofractionVisualisationController_->updateBirdsView();
}

void MapsController::slotWellClicked(const QString& wellName)
{
  mapsTab_->highlightWell(wellName);
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
  scenarioBackup::backup(scenario_);

  Logger::log() << "Done!" << Logger::endl();
  refreshGUI();
}

void MapsController::slotExportOptimized()
{
  if (scenario_.project3dPath().isEmpty())
  {
    return;
  }

  QDir sourceDir(scenario_.optimizedProjectDirectory());

  CMBProjectReader projectReader;
  LithoMapsInfoGenerator lithoMapsInfoGenerator(scenario_, projectReader);
  functions::exportScenarioToZip(sourceDir, scenario_.workingDirectory(), scenario_.project3dFilename(), lithoMapsInfoGenerator);
}

void MapsController::slotExportOptimizedToZycor()
{
  QDir sourceDir(scenario_.optimizedProjectDirectory());
  if (!sourceDir.exists())
  {
    Logger::log() << "No optimized project is available for export" << Logger::endl();
    return;
  }

  QString targetPath = QFileDialog::getExistingDirectory(nullptr, "Choose a location for the exported zycor maps", QDir::currentPath(),
                                                           QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

  QDir targetDir(targetPath);
  if (!targetDir.exists() || targetPath == "")
  {
    Logger::log() << "Target directory is not set" << Logger::endl();
    return;
  }

  scenario_.exportOptimizedLithofractionMapsToZycor(targetPath);

  Logger::log() << "Finished exporting optimized lithofraction maps to Zycor" << Logger::endl();
}

void MapsController::slotRunOptimized()
{
  if (scenario_.project3dPath().isEmpty())
  {
    return;
  }

  const QString baseDirectory{scenario_.optimizedProjectDirectory()};
  Run3dCaseController run3dCaseController(scenario_, scriptRunController_);
  if (run3dCaseController.run3dCase(baseDirectory, true))
  {
    scenarioBackup::backup(scenario_);
  }
}

void MapsController::slotUpdateBirdView()
{
  QVector<int> selectedWells = scenario_.getIncludedWellIndicesFromSelectedWells(getSelectedWellIndices());
  lithofractionVisualisationController_->updateSelectedWells(selectedWells);
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

} // namespace sac

} // namespace casaWizard
