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
#include "model/input/mapreader.h"
#include "model/logger.h"
#include "model/sacScenario.h"
#include "model/script/cauldronScript.h"
#include "model/script/Generate3DScenarioScript.h"
#include "model/script/track1dAllWellScript.h"
#include "view/plot/wellBirdsView.h"
#include "view/plot/wellCorrelationPlot.h"
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
  selectedWellsIDs_{},
  activeProperty_{""}
{
  connect(resultsTab_->plotOptions(), SIGNAL(activeChanged()), this, SLOT(activeChanged()));
  connect(resultsTab_->plotOptions(), SIGNAL(plotTypeChange(int)), this, SLOT(togglePlotType(int)));
  connect(resultsTab_->plotOptions(), SIGNAL(propertyChanged(QString)), this, SLOT(updateProperty(QString)));

  connect(resultsTab_->wellsList(), SIGNAL(itemSelectionChanged()), this, SLOT(updateWell()));
  connect(resultsTab_->wellBirdsView(), SIGNAL(pointSelectEvent(int,int)), this, SLOT(updateWellFromBirdView(int, int)));

  connect(resultsTab_->wellCorrelationPlot(), SIGNAL(selectedWell(int)), this, SLOT(selectedWellFromCorrelation(int)));  
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
  double xMin = 0; double xMax = 1; double yMin = 0; double yMax = 1;
  scenario_.projectReader().domainRange(xMin, xMax, yMin, yMax);
  resultsTab_->setRangeBirdsView(xMin * Utilities::Maths::MeterToKilometer,
                                 xMax * Utilities::Maths::MeterToKilometer,
                                 yMin * Utilities::Maths::MeterToKilometer,
                                 yMax * Utilities::Maths::MeterToKilometer);

  MapReader mapReader;
  mapReader.load(scenario_.project3dPath().toStdString());
  VectorVectorMap depthMap = mapReader.getMapData(scenario_.projectReader().getDepthGridName(0).toStdString());
  std::vector<std::vector<double>> values = depthMap.getData();
  for ( auto& row : values )
  {
    for ( double& value : row )
    {
      value = 1.0;
    }
  }
  resultsTab_->wellBirdsView()->setFixedValueRange({0,1});
  resultsTab_->wellBirdsView()->updatePlots( values, depthMap.getData());
}

void ResultsController::slotUpdateTabGUI(int tabID)
{
  if (tabID != static_cast<int>(TabID::Results))
  {
    return;
  }

  refreshGUI();

  initializeWellSelection();   
}

void ResultsController::initializeWellSelection()
{
  QListWidget* wellsList = resultsTab_->wellsList();
  if (wellsList->count() == 0)
  {
    wellsList->setCurrentRow(-1);
    updateWell(); // Manual update, as the itemSelectionChanged() signal is not sent when the list is empty
  }
  else if (selectedWellsIDs_.empty())
  {
    wellsList->setCurrentRow(0);
  }
  else
  {
    QSignalBlocker blocker(wellsList);

    const CalibrationTargetManager& calibrationManager = scenario_.calibrationTargetManager();
    const QVector<const Well*> activeWells = calibrationManager.activeWells();
    for( int i = 0; i < activeWells.size(); ++i)
    {
      int wellID = activeWells[i]->id();
      for ( const int selectedWellID : selectedWellsIDs_)
      {
        if ( wellID == selectedWellID)
        {
          wellsList->setItemSelected(wellsList->item(i), true);
          break;
        }
      }
    }
    updateBirdView();
    refreshPlot();
  }
}

void ResultsController::updateWell()
{
  selectedWellsIDs_ = selectedWellsIDs();
  if (selectedWellsIDs_.size() == 1)
  {    
    const CalibrationTargetManager& ctManager = scenario_.calibrationTargetManager();
    Logger::log() << "Selected well " << ctManager.well(selectedWellsIDs_.front()).name() << Logger::endl();
  }

  updateBirdView();
  refreshPlot();
}

void ResultsController::activeChanged()
{
  const QVector<bool> activePlots = resultsTab_->plotOptions()->activePlots();
  scenario_.setActivePlots(activePlots);

  refreshPlot();
}

void ResultsController::refreshPlot()
{
  updateOptimizedTable();
  updateWellPlot();
  updateCorrelationPlot();
}

void ResultsController::togglePlotType(const int currentIndex)
{
  if (currentIndex!=1 && selectedWellsIDs_.size()>1)
  {
    selectedWellsIDs_.clear();
  }

  resultsTab_->setPlotType(currentIndex);
  initializeWellSelection();
}

void ResultsController::updateOptimizedTable()
{
  if (selectedWellsIDs_.size() != 1)
  {
    if (selectedWellsIDs_.empty())
    {
      resultsTab_->updateOptimizedLithoTable({},{},{},{});
    }
    return;
  }

  const LithofractionManager& manager{scenario_.lithofractionManager()};
  double xLoc = 0.0;
  double yLoc = 0.0;

  const CalibrationTargetManager& ctManager = scenario_.calibrationTargetManager();
  const Well& well = ctManager.well(selectedWellsIDs_.front());
  xLoc = well.x();
  yLoc = well.y();

  QVector<OptimizedLithofraction> optimizedLithofractions = manager.optimizedInWell(selectedWellsIDs_.front());
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
  updateCorrelationPlot();
}

void ResultsController::updateWellFromBirdView(const int lineIndex, const int pointIndex)
{  
  if (lineIndex != 1)
  {
    return;
  }

  QListWidgetItem* item = resultsTab_->wellsList()->item(pointIndex);
  item->setSelected(!item->isSelected());
}

void ResultsController::selectedWellFromCorrelation(const int wellIndex)
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
  if (selectedWellsIDs_.size() != 1)
  {
    if (selectedWellsIDs_.empty())
    {
      resultsTab_->updateWellPlot({},{},{},{});
    }
    return;
  }
  QStringList properties;
  const CalibrationTargetManager& calibrationManager = scenario_.calibrationTargetManager();
  QVector<QVector<CalibrationTarget>> targetsInWell = calibrationManager.extractWellTargets(properties, selectedWellsIDs_);

  const WellTrajectoryManager& wellTrajectoryManager = scenario_.wellTrajectoryManager();
  const QVector<QVector<WellTrajectory>> allTrajectories = wellTrajectoryManager.trajectoriesInWell(selectedWellsIDs_, properties);
  QVector<bool> activePlots{scenario_.activePlots()};

  resultsTab_->updateWellPlot(targetsInWell,
                              properties,
                              allTrajectories,
                              activePlots);
}

void ResultsController::updateCorrelationPlot()
{
  QSignalBlocker blocker(resultsTab_->plotOptions());
  QStringList properties;  
  const CalibrationTargetManager& calibrationManager = scenario_.calibrationTargetManager();
  QVector<QVector<CalibrationTarget>> targetsInWell = calibrationManager.extractWellTargets(properties, selectedWellsIDs_);

  const WellTrajectoryManager& wellTrajectoryManager = scenario_.wellTrajectoryManager();
  QVector<QVector<WellTrajectory>> allTrajectories = wellTrajectoryManager.trajectoriesInWell(selectedWellsIDs_, properties);
  QVector<bool> activePlots{scenario_.activePlots()};

  resultsTab_->updateCorrelationPlot(targetsInWell,
                                     properties,
                                     allTrajectories,
                                     activePlots,
                                     activeProperty_);
}

void ResultsController::updateBirdView()
{
  resultsTab_->updateSelectedWells();
}

QVector<int> ResultsController::selectedWellsIDs()
{
  const CalibrationTargetManager& calibrationManager = scenario_.calibrationTargetManager();
  const QVector<const Well*> activeWells = calibrationManager.activeWells();

  QModelIndexList indices = resultsTab_->wellsList()->selectionModel()->selectedIndexes();

  QVector<int> wellIndices;
  for(const QModelIndex& index : indices)
  {
    wellIndices.push_back(activeWells[index.row()]->id());
  }
  std::sort(wellIndices.begin(), wellIndices.end());
  return wellIndices;
}

} // namespace sac

} // namespace casaWizard
