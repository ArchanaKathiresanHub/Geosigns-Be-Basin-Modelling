//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "LithologyResultsController.h"

#include "view/LithoTabIDs.h"

#include <QButtonGroup>

namespace casaWizard
{

namespace sac
{

namespace lithology
{

LithologyResultsController::LithologyResultsController(LithologyResultsTab* resultsTab,
                                                       SacLithologyScenario& scenario,
                                                       ScriptRunController& scriptRunController,
                                                       QObject* parent) :
   SacResultsController(resultsTab, scriptRunController, parent),
   m_resultsTab{resultsTab},
   m_scenario{scenario}
{
   connect(m_resultsTab->wellPrepOrSAC(), SIGNAL(buttonClicked(int)), this, SLOT(slotWellPrepOrSAC(int)));
}


void LithologyResultsController::slotUpdateTabGUI(int tabID)
{
   if (tabID != static_cast<int>(lithology::TabID::Results))
   {
      return;
   }

   refreshGUI();
}

void LithologyResultsController::slotTogglePlotType(const int currentIndex)
{
   if (currentIndex != 1 && selectedWellsIDs().size() > 1)
   {
      selectedWellsIDs() = {selectedWellsIDs().first()};
   }

   SacResultsController::slotTogglePlotType(currentIndex);
}

void LithologyResultsController::slotWellPrepOrSAC(int buttonId)
{
   setWellPrepWells(buttonId == 1);
   selectedWellsIDs().clear();

   refreshGUI();
}


void LithologyResultsController::updateOptimizedTable()
{
   if (selectedWellsIDs().size() != 1)
   {
      if (selectedWellsIDs().empty())
      {
         m_resultsTab->updateOptimizedLithoTable({},{},{},{});
      }
      return;
   }

   const LithofractionManager& manager{m_scenario.lithofractionManager()};
   double xLoc = 0.0;
   double yLoc = 0.0;

   const Well& well = calibrationTargetManager().well(selectedWellsIDs().front());
   xLoc = well.x();
   yLoc = well.y();

   QVector<OptimizedLithofraction> optimizedLithofractions = manager.optimizedInWell(selectedWellsIDs().front());
   const QVector<Lithofraction>& lithofractions = manager.lithofractions();
   const QStringList layerNames = m_scenario.projectReader().layerNames();

   QStringList layerNameList;
   QVector<QStringList> lithoNamesVector;
   QVector<QVector<double>> originalValuesVector;
   QVector<QVector<double>> optimizedValuesVector;
   for (const OptimizedLithofraction& optimizedLithofraction : optimizedLithofractions)
   {
      const Lithofraction& litho = lithofractions[optimizedLithofraction.lithofractionId()];
      layerNameList.push_back(litho.layerName());

      const int layerIndex = layerNames.indexOf(litho.layerName());
      const QStringList lithoNames = m_scenario.projectReader().lithologyTypesForLayer(layerIndex);
      QVector<double> originalValues = m_scenario.projectReader().lithologyValuesForLayerAtLocation(layerIndex, xLoc, yLoc);

      QStringList names = {lithoNames[litho.firstComponent()], lithoNames[litho.secondComponent()]};
      QVector<double> original = {originalValues[litho.firstComponent()], originalValues[litho.secondComponent()]};
      QVector<double> optimized = {optimizedLithofraction.optimizedPercentageFirstComponent(), optimizedLithofraction.optimizedPercentageSecondComponent()};
      if (!lithoNames[litho.thirdComponent()].isEmpty())
      {
         names.push_back(lithoNames[litho.thirdComponent()]);
         original.push_back(originalValues[litho.thirdComponent()]);
         optimized.push_back(optimizedLithofraction.optimizedPercentageThirdComponent());
      }

      lithoNamesVector.push_back(names);
      originalValuesVector.push_back(original);
      optimizedValuesVector.push_back(optimized);
   }

   m_resultsTab->updateOptimizedLithoTable(layerNameList, lithoNamesVector, originalValuesVector, optimizedValuesVector);
}


SacLithologyScenario& LithologyResultsController::scenario()
{
   return m_scenario;
}

SacLithologyScenario& LithologyResultsController::scenario() const
{
   return m_scenario;
}

LithologyResultsTab* LithologyResultsController::resultsTab()
{
   return m_resultsTab;
}

const LithologyResultsTab* LithologyResultsController::resultsTab() const
{
   return m_resultsTab;
}

} // namespace lithology

} // namespace sac

} // namespace casaWizard
