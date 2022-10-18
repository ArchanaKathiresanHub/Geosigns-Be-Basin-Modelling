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
#include "model/MapsManagerLithology.h"
#include "model/sacLithologyScenario.h"

#include <memory>

namespace casaWizard
{

class ScriptRunController;

namespace sac
{

class ActiveWellsController;
class MapsTabLithology;
class Generate3DScenarioScript;
class SacInfoGenerator;

class MapsControllerLithology : public SacMapsController
{
   Q_OBJECT

public:
   explicit MapsControllerLithology(MapsTabLithology* resultsTab,
                                    SacLithologyScenario& scenario,
                                    ScriptRunController& scriptRunController,
                                    QObject* parent);
private slots:

   void slotUpdateTabGUI(int tabID);
   void slotSmartGriddingChanged(int state);

private:  
   SacMapsTab* mapsTab() override;
   SacLithologyScenario& scenario() override;
   MapsManagerLithology& mapsManager() override;
   LithofractionVisualisationController& visualisationController() override;

   std::unique_ptr<Generate3DScenarioScript> generate3DScenarioScript() override;
   std::unique_ptr<SacInfoGenerator> infoGenerator() override;

   void refreshGUI() override;

   MapsManagerLithology& m_mapsManager;
   MapsTabLithology* m_mapsTab;
   SacLithologyScenario& m_scenario;
   LithofractionVisualisationController* m_lithofractionVisualisationController;
};

} // namespace sac

} // namespace casaWizard
