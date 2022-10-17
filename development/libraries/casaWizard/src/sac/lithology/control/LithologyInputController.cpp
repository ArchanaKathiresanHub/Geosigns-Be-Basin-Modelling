//
// Copyright (C) 2020 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "LithologyInputController.h"

#include "control/assets/DataExtractionController.h"
#include "control/lithofractionController.h"
#include "control/ScriptRunController.h"
#include "model/script/SacLithologyScript.h"
#include "model/sacLithologyScenario.h"
#include "view/LithologyInputTab.h"
#include "view/LithoTabIDs.h"

#include "model/extractor/optimizedLithofractionExtractor.h"

#include <QObject>
#include <QString>

namespace casaWizard
{

namespace sac
{

namespace lithology
{

LithologyInputController::LithologyInputController(LithologyInputTab* inputTab,
                                                   SacLithologyScenario& casaScenario,
                                                   ScriptRunController& scriptRunController,
                                                   QObject* parent) :
   SacInputController(inputTab,
                      casaScenario,
                      scriptRunController,
                      parent),
   m_scenario{casaScenario},
   m_inputTab{inputTab},
   m_lithofractionController{new LithofractionController(inputTab->lithofractionTable(), m_scenario, this)}
{
}

void LithologyInputController::refreshGUI()
{
   SacInputController::refreshGUI();
   m_lithofractionController->updateLithofractionTable();

   emit signalRefreshChildWidgets();
}

std::unique_ptr<SACScript> LithologyInputController::optimizationScript(const QString& baseDirectory, bool doOptimization)
{
   return std::unique_ptr<SACScript>(new SACLithologyScript(m_scenario, baseDirectory, doOptimization));
}

void LithologyInputController::readOptimizedResults()
{
   OptimizedLithofractionExtractor lithoExtractor{m_scenario};
   dataExtractionController()->readOptimizedResults(lithoExtractor);
}

void LithologyInputController::slotUpdateTabGUI(int tabID)
{
   if (tabID != static_cast<int>(TabID::Input))
   {
      return;
   }
   refreshGUI();
}

void LithologyInputController::slotPushButtonSelectProject3dClicked()
{
   if (selectWorkspace())
   {
      m_scenario.updateT2zLastSurface();
      m_lithofractionController->loadLayersFromProject();
      refreshGUI();
   }
}

SacLithologyScenario& LithologyInputController::scenario()
{
   return m_scenario;
}

SacLithologyScenario& LithologyInputController::scenario() const
{
   return m_scenario;
}

LithologyInputTab*LithologyInputController::inputTab()
{
   return m_inputTab;
}

const LithologyInputTab*LithologyInputController::inputTab() const
{
   return m_inputTab;
}

} // namespace lithology

} // namespace sac

} // namespace casaWizard
