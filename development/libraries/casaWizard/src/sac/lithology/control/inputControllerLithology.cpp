//
// Copyright (C) 2020 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "inputControllerLithology.h"

#include "control/assets/DataExtractionController.h"
#include "control/lithofractionController.h"
#include "control/ScriptRunController.h"
#include "model/script/SacLithologyScript.h"
#include "model/sacLithologyScenario.h"
#include "view/inputTab.h"
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

InputControllerLithology::InputControllerLithology(InputTab* inputTabP,
                                                   SacLithologyScenario& casaScenario,
                                                   ScriptRunController& scriptRunController,
                                                   QObject* parent) :
   SacInputController(inputTabP,
                      casaScenario,
                      scriptRunController,
                      parent),
   casaScenario_{casaScenario},
   lithofractionController_{new LithofractionController(inputTabP->lithofractionTable(), casaScenario_, this)}
{}

void InputControllerLithology::refreshGUI()
{
   SacInputController::refreshGUI();
   lithofractionController_->updateLithofractionTable();
}

void InputControllerLithology::slotUpdateTabGUI(int tabID)
{
   if (tabID != static_cast<int>(TabID::Input))
   {
      return;
   }
   refreshGUI();
}

std::unique_ptr<SACScript> InputControllerLithology::optimizationScript(const QString& baseDirectory, bool doOptimization)
{
   return std::unique_ptr<SACScript>(new SACLithologyScript(casaScenario_, baseDirectory, doOptimization));
}

void InputControllerLithology::readOptimizedResults()
{
   OptimizedLithofractionExtractor lithoExtractor{casaScenario_};
   dataExtractionController()->readOptimizedResults(lithoExtractor);
}

SacLithologyScenario& InputControllerLithology::scenario()
{
   return casaScenario_;
}

SacLithologyScenario& InputControllerLithology::scenario() const
{
   return casaScenario_;
}

bool InputControllerLithology::slotPushButtonSelectProject3dClicked()
{
   if (!SacInputController::slotPushButtonSelectProject3dClicked())
   {
      refreshGUI();
      return false;
   }

   casaScenario_.updateT2zLastSurface();
   lithofractionController_->loadLayersFromProject();
   refreshGUI();
   return true;
}

} // namespace lithology

} // namespace sac

} // namespace casaWizard
