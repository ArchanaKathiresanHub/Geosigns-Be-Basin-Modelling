//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include <QObject>
#include <QVector>

#include "model/ThermalScenario.h"
#include "control/SacResultsController.h"
#include "view/ThermalResultsTab.h"

class QString;

namespace casaWizard
{

namespace sac
{

class SacScenario;

namespace thermal
{


class ThermalResultsController : public SacResultsController
{
   Q_OBJECT

public:
   explicit ThermalResultsController(ThermalResultsTab* ThermalResultsTab,
                                     ThermalScenario& scenario,
                                     ScriptRunController& scriptRunController,
                                     QObject* parent);

private slots:
   void slotUpdateTabGUI(int tabID) final;
   void slotTogglePlotType(const int currentIndex) final;

private:
   ThermalScenario& scenario() final;
   ThermalScenario& scenario() const final;

   ThermalResultsTab* resultsTab() final;
   const ThermalResultsTab* resultsTab() const final;

   void updateOptimizedTable() final;

   ThermalScenario& m_scenario;
   ThermalResultsTab* m_resultsTab;
};

} // namespace thermal

} // namespace sac

} // namespace casaWizard
