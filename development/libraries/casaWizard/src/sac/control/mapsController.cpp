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
#include "control/scriptRunController.h"
#include "control/lithofractionVisualisationController.h"

#include "model/case3DTrajectoryConvertor.h"
#include "model/input/case3DTrajectoryReader.h"
#include "model/logger.h"
#include "model/sacScenario.h"
#include "model/scenarioBackup.h"
#include "model/script/cauldronScript.h"
#include "model/script/Generate3DScenarioScript.h"

#include "view/activeWellsTable.h"
#include "view/mapsTab.h"
#include "view/sacTabIDs.h"
#include "view/lithofractionVisualisation.h"

#include <QComboBox>
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
  activeWell_{0}
{
  connect(mapsTab_->interpolationType(), SIGNAL(currentIndexChanged(int)), this, SLOT(slotInterpolationTypeCurrentIndexChanged(int)));
  connect(mapsTab_->pValue(),            SIGNAL(valueChanged(int)),        this, SLOT(slotPvalueChanged(int)));
  connect(mapsTab_->smoothingType(),     SIGNAL(currentIndexChanged(int)), this, SLOT(slotSmoothingTypeCurrentIndexChanged(int)));
  connect(mapsTab_->smoothingRadius(),   SIGNAL(valueChanged(int)),        this, SLOT(slotSmoothingRadiusValueChanged(int)));
  connect(mapsTab_->threads(),           SIGNAL(valueChanged(int)),        this, SLOT(slotThreadsValueChanged(int)));
  connect(mapsTab_->createGridsButton(), SIGNAL(clicked()),                this, SLOT(slotGenerateLithoMaps()));

  connect(mapsTab_->activeWellsTable(), SIGNAL(selectedWell(const QString&)), this, SLOT(updateWell(const QString&)));
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
  const CalibrationTargetManager& ctManager = scenario_.calibrationTargetManager();
  mapsTab_->updateBirdsView(ctManager.activeAndIncludedWells());

  mapsTab_->interpolationType()->setCurrentIndex(scenario_.interpolationMethod());
  mapsTab_->pValue()->setValue(scenario_.pIDW());
  mapsTab_->smoothingType()->setCurrentIndex(scenario_.smoothingOption());
  mapsTab_->smoothingRadius()->setValue(scenario_.radiusSmoothing());
  mapsTab_->threads()->setValue(scenario_.threadsSmoothing());

  lithofractionVisualisationController_->updateAvailableLayers();  
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

void MapsController::updateWell(const QString& name)
{
  activeWell_ = -1;
  const CalibrationTargetManager& ctManager = scenario_.calibrationTargetManager();
  for (const Well* well : ctManager.activeWells())
  {
    if (well->name() == name)
    {
      activeWell_ = well->id();
      break;
    }
  }
  refreshGUI();

  Logger::log() << "Selected well " << activeWell_ << Logger::endl();
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

  lithofractionVisualisationController_->updateAvailableLayers();
}

} // namespace sac

} // namespace casaWizard
