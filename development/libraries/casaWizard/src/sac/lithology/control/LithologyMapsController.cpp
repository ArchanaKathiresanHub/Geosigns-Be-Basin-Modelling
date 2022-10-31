//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "LithologyMapsController.h"

#include "control/assets/activeWellsController.h"
#include "control/lithofractionVisualisationController.h"

#include "model/input/cmbProjectReader.h"
#include "model/output/LithoMapsInfoGenerator.h"
#include "model/script/Generate3DScenarioScriptLithology.h"

#include "view/sharedComponents/customcheckbox.h"
#include "view/assets/activeWellsTable.h"
#include "view/assets/lithofractionVisualisation.h"
#include "view/LithologyMapsTab.h"
#include "view/assets/LithoTabIDs.h"

#include <QVector>
#include <memory>

namespace casaWizard
{

namespace sac
{

LithologyMapsController::LithologyMapsController(LithologyMapsTab* mapsTab,
                                                 LithologyScenario& scenario,
                                                 ScriptRunController& scriptRunController,
                                                 QObject* parent) :
   SacMapsController(scriptRunController,parent),
   m_mapsManager{scenario.mapsManager()},
   m_mapsTab{mapsTab},
   m_scenario{scenario},
   m_lithofractionVisualisationController{new LithofractionVisualisationController(mapsTab->mapsVisualisation(), m_scenario, this)}
{
   init();
   connect(m_mapsTab->smartGridding(), SIGNAL(stateChanged(int)), this, SLOT(slotSmartGriddingChanged(int)));
}

SacMapsTab* LithologyMapsController::mapsTab()
{
   return m_mapsTab;
}

LithologyScenario& LithologyMapsController::scenario()
{
   return m_scenario;
}

LithologyMapsManager& LithologyMapsController::mapsManager()
{
   return m_mapsManager;
}

LithofractionVisualisationController& LithologyMapsController::visualisationController()
{
   return *m_lithofractionVisualisationController;
}

void LithologyMapsController::slotUpdateTabGUI(int tabID)
{
   if (tabID != static_cast<int>(lithology::TabID::Maps))
   {
      return;
   }
   refreshGUI();
}

void LithologyMapsController::refreshGUI()
{
   SacMapsController::refreshGUI();
   m_mapsTab->smartGridding()->setCheckState(m_mapsManager.smartGridding() ? Qt::Checked : Qt::Unchecked);
   emit signalRefreshChildWidgets();
   validateWellsHaveOptimized();
}

void LithologyMapsController::slotSmartGriddingChanged(int state)
{
   mapsManager().setSmartGridding(state == Qt::Checked);
   visualisationController().hideAllTooltips();
   activeWellsController()->slotClearWellListHighlightSelection();
   visualisationController().updateBirdsView();
}

std::unique_ptr<Generate3DScenarioScript> LithologyMapsController::generate3DScenarioScript()
{
   return std::unique_ptr<Generate3DScenarioScript>(new Generate3DScenarioScriptLithology(scenario()));
}

std::unique_ptr<SacInfoGenerator> LithologyMapsController::infoGenerator()
{
   std::unique_ptr<CMBProjectReader> projectReader = std::unique_ptr<CMBProjectReader>(new CMBProjectReader);
   return std::unique_ptr<SacInfoGenerator>(new LithoMapsInfoGenerator(scenario(), std::move(projectReader)));
}

} // namespace sac

} // namespace casaWizard
