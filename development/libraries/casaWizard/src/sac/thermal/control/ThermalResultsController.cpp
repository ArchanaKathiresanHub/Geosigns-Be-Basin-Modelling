//
// Copyright (C) 2012-2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "ThermalResultsController.h"

#include "model/SacScenario.h"
#include "control/ScriptRunController.h"
#include "model/extractor/OptimizedTCHPExtractor.h"
#include "view/ThermalResultsTab.h"
#include "view/assets/ThermalTabIDs.h"

namespace casaWizard
{

namespace sac
{

namespace thermal
{

ThermalResultsController::ThermalResultsController(ThermalResultsTab* resultsTab,
                                                   ThermalScenario& scenario,
                                                   ScriptRunController& scriptRunController,
                                                   QObject* parent) :
   SacResultsController(resultsTab, scriptRunController, parent),
   m_scenario{scenario},
   m_resultsTab{resultsTab}
{
}

void ThermalResultsController::slotUpdateTabGUI(int tabID)
{
   if (tabID != static_cast<int>(thermal::TabID::Results))
   {
      return;
   }
   refreshGUI();
}

void ThermalResultsController::slotTogglePlotType(const int currentIndex)
{
   if (currentIndex == 0 && selectedWellsIDs().size() > 1)
   {
      selectedWellsIDs() = {selectedWellsIDs().first()};
   }

   SacResultsController::slotTogglePlotType(currentIndex);
}

void ThermalResultsController::updateOptimizedTable()
{
   if (selectedWellsIDs().empty())
   {
      m_resultsTab->updateOptimizedHeatProductionTable({},{},{});
      return;
   }

   //get data
   QStringList wellNames;
   QVector<double> originalValues;
   QVector<double> optimalValues;
   for (int i = 0; i < selectedWellsIDs().size(); ++i)
   {
      wellNames.push_back(calibrationTargetManager().well(selectedWellsIDs()[i]).name());
      originalValues.push_back(m_scenario.projectReader().topCrustHeatProdAtLocation(calibrationTargetManager().well(selectedWellsIDs()[i]).x(),
                                                                                     calibrationTargetManager().well(selectedWellsIDs()[i]).y()));
      optimalValues.push_back( m_scenario.TCHPmanager().optimizedInWell( calibrationTargetManager().well(selectedWellsIDs()[i]).id() ).optimizedHeatProduction() );
   }

   m_resultsTab->updateOptimizedHeatProductionTable(wellNames, originalValues, optimalValues);
}

ThermalScenario& ThermalResultsController::scenario()
{
   return m_scenario;
}

ThermalScenario& ThermalResultsController::scenario() const
{
   return m_scenario;
}

ThermalResultsTab*ThermalResultsController::resultsTab()
{
   return m_resultsTab;
}

const ThermalResultsTab*ThermalResultsController::resultsTab() const
{
   return m_resultsTab;
}

} // namespace thermal

} // namespace sac

} // namespace casaWizard
