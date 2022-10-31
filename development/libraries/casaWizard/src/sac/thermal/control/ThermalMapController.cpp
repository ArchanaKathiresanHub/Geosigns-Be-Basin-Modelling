//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "ThermalMapController.h"

#include "control/assets/activeWellsController.h"
#include "assets/TCHPVisualisationController.h"

#include "model/input/cmbProjectReader.h"
#include "model/output/ThermalInfoGenerator.h"
#include "model/script/Generate3DThermalScenarioScript.h"
#include "model/ThermalScenario.h"

#include "view/sharedComponents/customcheckbox.h"
#include "view/assets/activeWellsTable.h"
#include "view/assets/TCHPVisualisation.h"
#include "view/ThermalMapTab.h"
#include "view/assets/ThermalTabIDs.h"

#include <QVector>
#include <memory>

namespace casaWizard
{

namespace sac
{

namespace thermal
{

ThermalMapController::ThermalMapController(ThermalMapTab* mapsTab,
                                           ThermalScenario& scenario,
                                           ScriptRunController& scriptRunController,
                                           QObject* parent) :
   SacMapsController(scriptRunController,parent),
   m_mapsManager{scenario.mapsManager()},
   m_mapsTab{mapsTab},
   m_scenario{scenario},
   m_tchpVisualisationController{mapsTab->mapsVisualisation(), scenario, this}
{
   init();
}

ThermalMapTab* ThermalMapController::mapsTab()
{
   return m_mapsTab;
}

ThermalScenario& ThermalMapController::scenario()
{
   return m_scenario;
}

ThermalMapManager& ThermalMapController::mapsManager()
{
   return m_mapsManager;
}

TCHPVisualisationController& ThermalMapController::visualisationController()
{
   return m_tchpVisualisationController;
}

void ThermalMapController::slotUpdateTabGUI(int tabID)
{
   if (tabID != static_cast<int>(thermal::TabID::Maps))
   {
      return;
   }
   refreshGUI();
}

void ThermalMapController::refreshGUI()
{
   SacMapsController::refreshGUI();
   emit signalRefreshChildWidgets();
   validateWellsHaveOptimized();
}

std::unique_ptr<Generate3DScenarioScript> ThermalMapController::generate3DScenarioScript()
{
   return std::unique_ptr<Generate3DScenarioScript>(new Generate3DThermalScenarioScript(m_scenario));
}

std::unique_ptr<SacInfoGenerator> ThermalMapController::infoGenerator()
{
   std::unique_ptr<CMBProjectReader> projectReader = std::unique_ptr<CMBProjectReader>(new CMBProjectReader);
   return std::unique_ptr<SacInfoGenerator>(new ThermalInfoGenerator(m_scenario, std::move(projectReader)));
}

} // namespace thermal

} // namespace sac

} // namespace casaWizard
