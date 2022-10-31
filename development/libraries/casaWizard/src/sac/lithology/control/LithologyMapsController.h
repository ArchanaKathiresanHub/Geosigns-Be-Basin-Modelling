//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include "lithofractionVisualisationController.h"

#include "control/SacMapsController.h"
#include "model/LithologyMapsManager.h"
#include "model/LithologyScenario.h"

#include <memory>

namespace casaWizard
{

class ScriptRunController;

namespace sac
{

class ActiveWellsController;
class LithologyMapsTab;
class Generate3DScenarioScript;
class SacInfoGenerator;

class LithologyMapsController : public SacMapsController
{
   Q_OBJECT

public:
   explicit LithologyMapsController(LithologyMapsTab* resultsTab,
                                    LithologyScenario& scenario,
                                    ScriptRunController& scriptRunController,
                                    QObject* parent);
private slots:

   void slotUpdateTabGUI(int tabID);
   void slotSmartGriddingChanged(int state);

private:  
   SacMapsTab* mapsTab() override;
   LithologyScenario& scenario() override;
   LithologyMapsManager& mapsManager() override;
   LithofractionVisualisationController& visualisationController() override;

   std::unique_ptr<Generate3DScenarioScript> generate3DScenarioScript() override;
   std::unique_ptr<SacInfoGenerator> infoGenerator() override;

   void refreshGUI() override;

   LithologyMapsManager& m_mapsManager;
   LithologyMapsTab* m_mapsTab;
   LithologyScenario& m_scenario;
   LithofractionVisualisationController* m_lithofractionVisualisationController;
};

} // namespace sac

} // namespace casaWizard
