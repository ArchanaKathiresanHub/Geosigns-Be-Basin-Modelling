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
   casaScenario_{casaScenario},
   lithofractionController_{new LithofractionController(inputTab->lithofractionTable(), casaScenario_, this)}
{
}

void LithologyInputController::refreshGUI()
{
   SacInputController::refreshGUI();
   lithofractionController_->updateLithofractionTable();

   emit signalRefreshChildWidgets();
}

std::unique_ptr<SACScript> LithologyInputController::optimizationScript(const QString& baseDirectory, bool doOptimization)
{
   return std::unique_ptr<SACScript>(new SACLithologyScript(casaScenario_, baseDirectory, doOptimization));
}

void LithologyInputController::readOptimizedResults()
{
   OptimizedLithofractionExtractor lithoExtractor{casaScenario_};
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
      casaScenario_.updateT2zLastSurface();
      lithofractionController_->loadLayersFromProject();
      refreshGUI();
   }
}

SacLithologyScenario& LithologyInputController::scenario()
{
   return casaScenario_;
}

SacLithologyScenario& LithologyInputController::scenario() const
{
   return casaScenario_;
}

} // namespace lithology

} // namespace sac

} // namespace casaWizard
