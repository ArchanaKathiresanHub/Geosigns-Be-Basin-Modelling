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
#include "model/functions/interpolateVector.h"
#include "model/input/case3DTrajectoryReader.h"
#include "model/input/cmbMapReader.h"
#include "model/logger.h"
#include "model/sacScenario.h"
#include "model/script/cauldronScript.h"
#include "model/script/Generate3DScenarioScript.h"
#include "model/script/track1dAllWellScript.h"
#include "view/plot/wellBirdsView.h"
#include "view/plot/wellCorrelationPlot.h"
#include "view/multiWellPlot.h"
#include "view/plotOptions.h"
#include "view/resultsTab.h"
#include "view/sacTabIDs.h"
#include "view/wellCorrelationPlotLayout.h"

#include "ConstantsMathematics.h"

#include <QFileDialog>
#include <QInputDialog>
#include <QListWidget>
#include <QPushButton>

#include <assert.h>

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
  connect(resultsTab_->plotOptions(), SIGNAL(slotActiveChanged()), this, SLOT(slotActiveChanged()));
  connect(resultsTab_->plotOptions(), SIGNAL(plotTypeChange(int)), this, SLOT(slotTogglePlotType(int)));
  connect(resultsTab_->wellCorrelationPlotLayout(), SIGNAL(propertyChanged(QString)), this, SLOT(slotUpdateProperty(QString)));

  connect(resultsTab_->wellsList(), SIGNAL(itemSelectionChanged()), this, SLOT(slotUpdateWell()));
  connect(resultsTab_->wellBirdsView(), SIGNAL(pointSelectEvent(int,int)), this, SLOT(slotUpdateWellFromBirdView(int, int)));
  connect(resultsTab_->wellCorrelationPlot(), SIGNAL(selectedWell(int)), this, SLOT(slotSelectedWellFromCorrelation(int)));
  connect(resultsTab_->multiWellPlot(), SIGNAL(isExpandedChanged(int, int)), this, SLOT(slotUpdateIsExpanded(int, int)));
  connect(resultsTab_->multiWellPlot(), SIGNAL(showSurfaceLinesChanged(bool)), this, SLOT(slotUpdateSurfaceLines(bool)));
  connect(resultsTab_->multiWellPlot(), SIGNAL(fitRangeToDataChanged(bool)), this, SLOT(slotUpdateFitRangeToData(bool)));
}

void ResultsController::refreshGUI()
{
  setActiveWells();
  setActivePropertiesToWells();

  setGuiOptionsInPlots();
  setDomainBirdsView();  
}

void ResultsController::setActiveWells()
{
  const CalibrationTargetManager& ctManager = scenario_.calibrationTargetManager();
  resultsTab_->updateWellList(ctManager.activeWells());
  resultsTab_->updateBirdsView(ctManager.activeWells());
}

void ResultsController::setActivePropertiesToWells()
{
  const CalibrationTargetManager& ctManager = scenario_.calibrationTargetManager();
  resultsTab_->updateActivePropertyUserNames(ctManager.activePropertyUserNames());
}

void ResultsController::setGuiOptionsInPlots()
{
  resultsTab_->plotOptions()->setActivePlots(scenario_.activePlots());
  resultsTab_->multiWellPlot()->setShowSurfaceLines(scenario_.showSurfaceLines());
  resultsTab_->multiWellPlot()->setFitRangeToData(scenario_.fitRangeToData());
}

void ResultsController::setDomainBirdsView()
{  
  double xMin = 0; double xMax = 1; double yMin = 0; double yMax = 1;
  scenario_.projectReader().domainRange(xMin, xMax, yMin, yMax);
  resultsTab_->setRangeBirdsView(xMin * Utilities::Maths::MeterToKilometer,
                                 xMax * Utilities::Maths::MeterToKilometer,
                                 yMin * Utilities::Maths::MeterToKilometer,
                                 yMax * Utilities::Maths::MeterToKilometer);

  CMBMapReader mapReader;
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
    slotUpdateWell(); // Manual update, as the itemSelectionChanged() signal is not sent when the list is empty
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

void ResultsController::slotUpdateWell()
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

void ResultsController::slotActiveChanged()
{
  const QVector<bool> activePlots = resultsTab_->plotOptions()->activePlots();
  scenario_.setActivePlots(activePlots);

  refreshPlot();
}

void ResultsController::slotUpdateSurfaceLines(const bool showSurfaceLines)
{
  scenario_.setShowSurfaceLines(showSurfaceLines);

  refreshPlot();
}

void ResultsController::slotUpdateFitRangeToData(const bool fitRangeToData)
{
  scenario_.setFitRangeToData(fitRangeToData);

  refreshPlot();
}

void ResultsController::slotUpdateIsExpanded(int state, int plotID)
{
  resultsTab_->multiWellPlot()->setExpanded(state == Qt::CheckState::Checked, plotID);

  refreshPlot();
}


void ResultsController::refreshPlot()
{
  updateOptimizedTable();
  updateWellPlot();
  updateCorrelationPlot();
}

void ResultsController::slotTogglePlotType(const int currentIndex)
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

void ResultsController::slotUpdateProperty(QString property)
{
  activeProperty_ = property;
  updateCorrelationPlot();
}

void ResultsController::slotUpdateWellFromBirdView(const int lineIndex, const int pointIndex)
{  
  if (lineIndex != 1)
  {
    return;
  }

  QListWidgetItem* item = resultsTab_->wellsList()->item(pointIndex);
  item->setSelected(!item->isSelected());
}

void ResultsController::slotSelectedWellFromCorrelation(const int wellIndex)
{
  const CalibrationTargetManager& mgr = scenario_.calibrationTargetManager();
  const QVector<const Well*> wells = mgr.wells();
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
      resultsTab_->updateWellPlot({},{},{},{},{}, false);
    }
    return;
  }

  QStringList propertyUserNames;
  const CalibrationTargetManager& calibrationManager = scenario_.calibrationTargetManager();
  const QVector<QVector<const CalibrationTarget*>> targetsInWell = calibrationManager.extractWellTargets(propertyUserNames, selectedWellsIDs_);

  const WellTrajectoryManager& wellTrajectoryManager = scenario_.wellTrajectoryManager();
  const QVector<QVector<WellTrajectory>> allTrajectories = wellTrajectoryManager.trajectoriesInWell(selectedWellsIDs_, propertyUserNames);  

  resultsTab_->updateWellPlot(targetsInWell,
                              getUnitsForProperties(allTrajectories),
                              allTrajectories,
                              scenario_.activePlots(),
                              getSurfaceValues(),
                              scenario_.fitRangeToData());
}

QMap<QString, double> ResultsController::getSurfaceValues()
{
  QMap<QString, double> surfaceValues;
  if (scenario_.showSurfaceLines())
  {
    const CalibrationTargetManager& calibrationManager = scenario_.calibrationTargetManager();
    const casaWizard::Well& well = calibrationManager.well(selectedWellsIDs_[0]);
    CMBMapReader mapReader;
    mapReader.load(scenario_.project3dPath().toStdString());
    QStringList surfaceNames = scenario_.projectReader().surfaceNames();
    for (int i = 0; i < surfaceNames.size(); i++)
    {
      QString depthGridName = scenario_.projectReader().getDepthGridName(i);
      const double depth = -1.0*mapReader.getValue(well.x(), well.y(), depthGridName.toStdString());
      surfaceValues.insert(surfaceNames[i], depth);
    }
  }

  return surfaceValues;
}

QStringList ResultsController::getUnitsForProperties(const QVector<QVector<WellTrajectory>>& allTrajectories)
{
  const CalibrationTargetManager& calibrationManager = scenario_.calibrationTargetManager();
  QStringList units;
  for (int i = 0; i < allTrajectories[0].size(); i++)
  {
    QString cauldronName = calibrationManager.getCauldronPropertyName(allTrajectories[0][i].propertyUserName());
    units.push_back(scenario_.projectReader().getUnit(cauldronName));
  }

  return units;
}

void ResultsController::updateCorrelationPlot()
{
  QSignalBlocker blocker(resultsTab_->wellCorrelationPlotLayout());

  QStringList propertyUserNames;
  const CalibrationTargetManager& calibrationManager = scenario_.calibrationTargetManager();
  QVector<QVector<const CalibrationTarget*>> targetsInWell = calibrationManager.extractWellTargets(propertyUserNames, selectedWellsIDs_);

  if (propertyUserNames.empty())
  {
    return;
  }

  int iProperty = 0;
  for (const QString& property: propertyUserNames)
  {
    if (iProperty != 0 && calibrationManager.getCauldronPropertyName(property) == "TwoWayTime")
    {
      propertyUserNames.move(iProperty, 0);
    }
    iProperty++;
  }

  const WellTrajectoryManager& wellTrajectoryManager = scenario_.wellTrajectoryManager();
  QVector<QVector<WellTrajectory>> allTrajectories = wellTrajectoryManager.trajectoriesInWell(selectedWellsIDs_, propertyUserNames);

  assert(targetsInWell.size() ==  allTrajectories[0].size());
  int activePropertyIndex = propertyUserNames.indexOf(activeProperty_);
  if (activePropertyIndex == -1)
  {
    activePropertyIndex = 0;
    activeProperty_ = propertyUserNames[activePropertyIndex];
  }
  resultsTab_->wellCorrelationPlotLayout()->setProperties(propertyUserNames, activePropertyIndex);

  assert(allTrajectories.size() == 4);

  const int nTrajectories = targetsInWell.size();

  double minValue = 0.0;
  double maxValue = 1.0;
  bool first = true;

  QVector<int> wellIndices;
  QVector<QVector<double>> measuredValueTrajectories;
  QVector<QVector<double>> simulatedValueTrajectories;
  QVector<bool> activePlots{};

  for (int j = 0; j < allTrajectories.size(); ++j)
  {
    const bool wellIndicesSet = wellIndices.size()>0;
    if (!scenario_.activePlots()[j])
    {
      continue;
    }

    QVector<double> allValuesMeasured;
    QVector<double> allValuesSimulated;
    for (int i = 0; i < nTrajectories; ++i)
    {
      if (allTrajectories[j][i].propertyUserName() != activeProperty_ ||
          allTrajectories[j][i].depth().empty())
      {
        continue;
      }

      QVector<double> depthMeasured;
      QVector<double> valueMeasured;
      for (const CalibrationTarget* target : targetsInWell[i])
      {
        depthMeasured.push_back(target->z());
        valueMeasured.push_back(target->value());
      }

      QVector<double> valueSimulated = functions::interpolateVector(allTrajectories[j][i].depth(), allTrajectories[j][i].value(), depthMeasured);

      for (int k = 0; k < valueSimulated.size(); k++)
      {
        allValuesMeasured.append(valueMeasured[k]);
        allValuesSimulated.append(valueSimulated[k]);
        if (!wellIndicesSet)
        {
          wellIndices.append(allTrajectories[j][i].wellIndex());
        }

        if (first)
        {
            minValue = valueMeasured[k];
            maxValue = valueMeasured[k];
            first = false;
        }

        if (minValue > valueMeasured[k])  minValue = valueMeasured[k];
        if (minValue > valueSimulated[k]) minValue = valueSimulated[k];
        if (maxValue < valueMeasured[k])  maxValue = valueMeasured[k];
        if (maxValue < valueSimulated[k]) maxValue = valueSimulated[k];
      }
    }
    measuredValueTrajectories.append(allValuesMeasured);
    simulatedValueTrajectories.append(allValuesSimulated);
  }

  resultsTab_->updateCorrelationPlot(measuredValueTrajectories,
                                     simulatedValueTrajectories,
                                     activeProperty_,
                                     scenario_.activePlots(),
                                     minValue,
                                     maxValue,
                                     wellIndices);
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
