//
// Copyright (C) 2012-2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include "assets/TCHPVisualisationController.h"

#include "control/SacMapsController.h"
#include "view/ThermalMapTab.h"
#include "model/ThermalMapManager.h"
#include "model/ThermalScenario.h"

#include <memory>

namespace casaWizard
{

class ScriptRunController;

namespace sac
{

class ActiveWellsController;
class Generate3DScenarioScript;
class SacInfoGenerator;

namespace thermal
{


class ThermalMapTab;

class ThermalMapController : public SacMapsController
{
   Q_OBJECT

public:
   explicit ThermalMapController(ThermalMapTab* resultsTab,
                                 ThermalScenario& scenario,
                                 ScriptRunController& scriptRunController,
                                 QObject* parent);
private slots:
   void slotUpdateTabGUI(int tabID);

private:
   ThermalMapTab* mapsTab() override;
   ThermalScenario& scenario() override;
   ThermalMapManager& mapsManager() override;
   TCHPVisualisationController& visualisationController() override;

   std::unique_ptr<Generate3DScenarioScript> generate3DScenarioScript() override;
   std::unique_ptr<SacInfoGenerator> infoGenerator() override;

   void refreshGUI() override;

   ThermalMapManager& m_mapsManager;
   ThermalMapTab* m_mapsTab;
   ThermalScenario& m_scenario;
   TCHPVisualisationController m_tchpVisualisationController;
};

} // namespace thermal

} // namespace sac

} // namespace casaWizard
