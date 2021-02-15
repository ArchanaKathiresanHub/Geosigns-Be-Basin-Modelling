//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "resultsController.h"

#include "control/casaScriptWriter.h"
#include "control/scriptRunController.h"
#include "model/case3DTrajectoryConvertor.h"
#include "model/input/case3DTrajectoryReader.h"
#include "model/logger.h"
#include "model/sacScenario.h"
#include "model/scenarioBackup.h"
#include "model/script/cauldronScript.h"
#include "model/script/Generate3DScenarioScript.h"
#include "model/script/track1dAllWellScript.h"
#include "view/plot/wellBirdsView.h"
#include "view/plot/wellScatterPlot.h"
#include "view/plotOptions.h"
#include "view/resultsTab.h"
#include "view/sacTabIDs.h"

#include "ConstantsMathematics.h"

#include <QFileDialog>
#include <QInputDialog>
#include <QListWidget>
#include <QPushButton>

namespace casaWizard
{

namespace sac
{

ResultsController::ResultsController(ResultsTab* resultsTab,
                                     SACScenario& scenario,
                                     ScriptRunController& scriptRunController,
                                     QObject* parent) :
  QObject(parent),
  resultsTab_{resultsTab},
  scenario_{scenario},
  scriptRunController_{scriptRunController},
  activeWell_{0},
  activeProperty_{""}
{
  connect(resultsTab_->wellsList(), SIGNAL(currentTextChanged(const QString&)), this, SLOT(updateWell(const QString&)));
  connect(resultsTab_->wellsList(), SIGNAL(itemSelectionChanged()), this, SLOT(refreshPlot()));

  connect(resultsTab_->plotOptions(), SIGNAL(activeChanged()), this, SLOT(refreshPlot()));
  connect(resultsTab_->plotOptions(), SIGNAL(plotTypeChange(int)), this, SLOT(togglePlotType(int)));
  connect(resultsTab_->plotOptions(), SIGNAL(propertyChanged(QString)), this, SLOT(updateProperty(QString)));

  connect(resultsTab_->wellBirdsView(), SIGNAL(pointSelectEvent(int,int)), this, SLOT(updateWellFromBirdView(int, int)));
  connect(resultsTab_->wellScatterPlot(), SIGNAL(selectedWell(int)), this, SLOT(selectedWellFromScatter(int)));
}

void ResultsController::refreshGUI()
{
  setActiveWells();
  setActivePlots();
  setDomainBirdsView();
}

void ResultsController::setActiveWells()
{
  const CalibrationTargetManager& ctManager = scenario_.calibrationTargetManager();
  resultsTab_->updateWellList(ctManager.activeWells());
  resultsTab_->updateBirdsView(ctManager.activeWells());
}

void ResultsController::setActivePlots()
{
  resultsTab_->plotOptions()->setActivePlots(scenario_.activePlots());
}

void ResultsController::setDomainBirdsView()
{
  double xMin, xMax, yMin, yMax;
  scenario_.projectReader().domainRange(xMin, xMax, yMin, yMax);
  resultsTab_->setRangeBirdsView(xMin * Utilities::Maths::MeterToKilometer,
                                 xMax * Utilities::Maths::MeterToKilometer,
                                 yMin * Utilities::Maths::MeterToKilometer,
                                 yMax * Utilities::Maths::MeterToKilometer);
}

void ResultsController::slotUpdateTabGUI(int tabID)
{
  if (tabID != static_cast<int>(TabID::Results))
  {
    return;
  }

  refreshGUI();
  setDefaultWellSelection();
}

void ResultsController::setDefaultWellSelection()
{
  if (noValidWellSelected())
  {
    resultsTab_->wellsList()->setCurrentRow(0);
  }
}

bool ResultsController::noValidWellSelected()
{
  const CalibrationTargetManager& ctManager = scenario_.calibrationTargetManager();
  const int nrOfActiveWells = ctManager.activeWells().size();
  return ( nrOfActiveWells > 0 && ( resultsTab_->wellsList()->currentRow() < 0 ||
                                    resultsTab_->wellsList()->currentRow() >= nrOfActiveWells) );
}

void ResultsController::updateWell(const QString& name)
{
  activeWell_ = -1;
  const CalibrationTargetManager& ctManager = scenario_.calibrationTargetManager();
  for (const Well* well : ctManager.activeWells())
  {
    if (well->name() == name)
    {
      activeWell_ = well->id();
      break;
    }
  }
  refreshPlot();
  Logger::log() << "Selected well " << activeWell_ << Logger::endl();
}

void ResultsController::refreshPlot()
{
  QVector<bool> activePlots = resultsTab_->plotOptions()->activePlots();
  scenario_.setActivePlots(activePlots);
  updateOptimizedTable();
  updateWellPlot();
  updateScatterPlot();
  updateBirdView();
}

void ResultsController::togglePlotType(const int currentIndex)
{
  resultsTab_->setPlotType(currentIndex);
  refreshPlot();
}

void ResultsController::updateOptimizedTable()
{
  const LithofractionManager& manager{scenario_.lithofractionManager()};
  double xLoc = 0.0;
  double yLoc = 0.0;
  if (activeWell_>-1)
  {
    const CalibrationTargetManager& ctManager = scenario_.calibrationTargetManager();
    const Well& well = ctManager.well(activeWell_);
    xLoc = well.x();
    yLoc = well.y();
  }

  QVector<OptimizedLithofraction> optimizedLithofractions = manager.optimizedInWell(activeWell_);
  const QVector<Lithofraction>& lithofractions = manager.lithofractions();
  const QStringList layerNames = scenario_.projectReader().layerNames();

  QStringList layerNameList;
  QVector<QStringList> lithoNamesVector;
  QVector<QVector<double>> originalValuesVector;
  QVector<QVector<double>> optimizedValuesVector;
  for (const OptimizedLithofraction& optimizedLithofraction : optimizedLithofractions)
  {
    const Lithofraction& litho = lithofractions[optimizedLithofraction.lithofractionId()];
    layerNameList.push_back(litho.layerName());

    const int layerIndex = layerNames.indexOf(litho.layerName());
    const QStringList lithoNames = scenario_.projectReader().lithologyTypesForLayer(layerIndex);
    QVector<double> originalValues = scenario_.projectReader().lithologyValuesForLayerAtLocation(layerIndex, xLoc, yLoc);

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

  resultsTab_->updateOptimizedLithoTable(layerNameList, lithoNamesVector, originalValuesVector, optimizedValuesVector);
}

void ResultsController::updateProperty(const QString property)
{
  activeProperty_ = property;
  updateScatterPlot();
}

void ResultsController::updateWellFromBirdView(const int lineIndex, const int pointIndex)
{
  // Disable when multiple wells are selected
  if (selectedWells().size() > 1)
  {
    return;
  }
  QSignalBlocker blocker(resultsTab_->wellsList());
  if (lineIndex != 1)
  {
    return;
  }

  if (pointIndex<activeWell_)
  {
    activeWell_ = pointIndex;
  }
  else
  {
    activeWell_ = pointIndex + 1;
  }
  resultsTab_->wellsList()->setCurrentRow(activeWell_);
  refreshPlot();
}

void ResultsController::selectedWellFromScatter(const int wellIndex)
{
  const CalibrationTargetManager& mgr = scenario_.calibrationTargetManager();
  const QVector<const Well*> wells = mgr.activeWells();
  if (wellIndex>=wells.size())
  {
    return;
  }

  Logger::log() << "Selected point is in well " << wells[wellIndex]->name() << Logger::endl();

}

void ResultsController::updateWellPlot()
{
  QStringList properties;
  const CalibrationTargetManager& calibrationManager = scenario_.calibrationTargetManager();
  QVector<QVector<CalibrationTarget>> targetsInWell = calibrationManager.extractWellTargets(properties, activeWell_);

  const WellTrajectoryManager& wellTrajectoryManager = scenario_.wellTrajectoryManager();
  const QVector<QVector<WellTrajectory>> allTrajectories = wellTrajectoryManager.trajectoriesInWell({activeWell_}, properties);
  QVector<bool> activePlots{scenario_.activePlots()};



  resultsTab_->updateWellPlot(targetsInWell,
                              properties,
                              allTrajectories,
                              activePlots);
}

void ResultsController::updateScatterPlot()
{
  QSignalBlocker blocker(resultsTab_->plotOptions());
  QStringList properties;
  QVector<int> wellIndices = selectedWells();
  const CalibrationTargetManager& calibrationManager = scenario_.calibrationTargetManager();
  QVector<QVector<CalibrationTarget>> targetsInWell = calibrationManager.extractWellTargets(properties, wellIndices);

  const WellTrajectoryManager& wellTrajectoryManager = scenario_.wellTrajectoryManager();
  QVector<QVector<WellTrajectory>> allTrajectories = wellTrajectoryManager.trajectoriesInWell(wellIndices, properties);
  QVector<bool> activePlots{scenario_.activePlots()};

  resultsTab_->updateScatterPlot(targetsInWell,
                                 properties,
                                 allTrajectories,
                                 activePlots,
                                 activeProperty_);
}

void ResultsController::updateBirdView()
{
  resultsTab_->updateActiveWells(selectedWells());
}

QVector<int> ResultsController::selectedWells()
{
  QModelIndexList indices = resultsTab_->wellsList()->selectionModel()->selectedIndexes();

  QVector<int> wellIndices;
  for(const QModelIndex& index : indices)
  {
    wellIndices.push_back(index.row());
  }
  std::sort(wellIndices.begin(), wellIndices.end());
  return wellIndices;
}

} // namespace sac

} // namespace casaWizard
