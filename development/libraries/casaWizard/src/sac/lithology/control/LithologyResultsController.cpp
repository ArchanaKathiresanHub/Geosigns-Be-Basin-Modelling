//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "LithologyResultsController.h"

#include "control/casaScriptWriter.h"
#include "control/ScriptRunController.h"
#include "model/case3DTrajectoryConvertor.h"
#include "model/functions/cauldronPropertyUnits.h"
#include "model/functions/interpolateVector.h"
#include "model/input/case3DTrajectoryReader.h"
#include "model/input/cmbMapReader.h"
#include "model/logger.h"
#include "model/sacLithologyScenario.h"
#include "model/script/cauldronScript.h"
#include "model/script/Generate3DScenarioScript.h"
#include "model/script/track1dAllWellScript.h"
#include "view/plot/wellBirdsView.h"
#include "view/plot/wellCorrelationPlot.h"
#include "view/assets/multiWellPlot.h"
#include "view/assets/PlotOptions.h"
#include "view/LithologyResultsTab.h"
#include "view/LithoTabIDs.h"
#include "view/assets/wellCorrelationPlotLayout.h"

#include "ConstantsMathematics.h"

#include <QButtonGroup>
#include <QFileDialog>
#include <QInputDialog>
#include <QListWidget>
#include <QPushButton>

#include <assert.h>

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
   QObject(parent),
   m_resultsTab{resultsTab},
   m_scenario{scenario},
   m_scriptRunController{scriptRunController},
   m_selectedWellsIDs{},
   m_activeProperty{""},
   m_wellPrepWells{false}
{
   connect(m_resultsTab->wellPrepOrSAC(), SIGNAL(buttonClicked(int)), this, SLOT(slotWellPrepOrSAC(int)));

   connect(m_resultsTab->plotOptions(), SIGNAL(activeChanged()), this, SLOT(slotActiveChanged()));
   connect(m_resultsTab->plotOptions(), SIGNAL(plotTypeChange(int)), this, SLOT(slotTogglePlotType(int)));
   connect(m_resultsTab->wellCorrelationPlotLayout(), SIGNAL(propertyChanged(QString)), this, SLOT(slotUpdateProperty(QString)));

   connect(m_resultsTab->wellsList(), SIGNAL(itemSelectionChanged()), this, SLOT(slotUpdateWell()));
   connect(m_resultsTab->wellBirdsView(), SIGNAL(pointSelectEvent(int,int)), this, SLOT(slotUpdateWellFromBirdView(int, int)));
   connect(m_resultsTab->wellCorrelationPlot(), SIGNAL(selectedWell(int)), this, SLOT(slotSelectedWellFromCorrelation(int)));
   connect(m_resultsTab->multiWellPlot(), SIGNAL(isExpandedChanged(int, int)), this, SLOT(slotUpdateIsExpanded(int, int)));
   connect(m_resultsTab->multiWellPlot(), SIGNAL(showSurfaceLinesChanged(bool)), this, SLOT(slotUpdateSurfaceLines(bool)));
   connect(m_resultsTab->multiWellPlot(), SIGNAL(fitRangeToDataChanged(bool)), this, SLOT(slotUpdateFitRangeToData(bool)));
}

const CalibrationTargetManager& LithologyResultsController::calibrationTargetManager() const
{
   return m_wellPrepWells ? m_scenario.calibrationTargetManagerWellPrep() : m_scenario.calibrationTargetManager();
}

void LithologyResultsController::refreshGUI()
{
   setActiveWells();
   setActivePropertiesToWells();

   setGuiOptionsInPlots();
   setDomainBirdsView();

   initializeWellSelection();
}

void LithologyResultsController::setActiveWells()
{
   m_resultsTab->updateWellList(calibrationTargetManager().activeWells());
   m_resultsTab->updateBirdsView(calibrationTargetManager().activeWells());
}

void LithologyResultsController::setActivePropertiesToWells()
{  
   m_resultsTab->updateActivePropertyUserNames(calibrationTargetManager().activePropertyUserNames());
}

void LithologyResultsController::setGuiOptionsInPlots()
{
   m_resultsTab->plotOptions()->setActivePlots(m_scenario.activePlots());
   m_resultsTab->multiWellPlot()->setShowSurfaceLines(m_scenario.showSurfaceLines());
   m_resultsTab->multiWellPlot()->setFitRangeToData(m_scenario.fitRangeToData());
}

void LithologyResultsController::setDomainBirdsView()
{  
   double xMin = 0; double xMax = 1; double yMin = 0; double yMax = 1;
   m_scenario.projectReader().domainRange(xMin, xMax, yMin, yMax);
   m_resultsTab->setRangeBirdsView(xMin * Utilities::Maths::MeterToKilometer,
                                  xMax * Utilities::Maths::MeterToKilometer,
                                  yMin * Utilities::Maths::MeterToKilometer,
                                  yMax * Utilities::Maths::MeterToKilometer);

   CMBMapReader mapReader;
   mapReader.load(m_scenario.project3dPath().toStdString());
   VectorVectorMap depthMap = mapReader.getMapData(m_scenario.projectReader().getDepthGridName(0).toStdString());
   depthMap.setDefinedValuesToOne();
   m_resultsTab->wellBirdsView()->setFixedValueRange({0,1});
   m_resultsTab->wellBirdsView()->updatePlots(depthMap.getData());
}

void LithologyResultsController::slotUpdateTabGUI(int tabID)
{
   if (tabID != static_cast<int>(lithology::TabID::Results))
   {
      return;
   }

   refreshGUI();
}

void LithologyResultsController::slotWellPrepOrSAC(int buttonId)
{
   m_wellPrepWells = (buttonId == 1);
   m_selectedWellsIDs.clear();

   refreshGUI();
}

void LithologyResultsController::initializeWellSelection()
{
   QListWidget* wellsList = m_resultsTab->wellsList();
   if (wellsList->count() == 0)
   {
      wellsList->setCurrentRow(-1);
      slotUpdateWell(); // Manual update, as the itemSelectionChanged() signal is not sent when the list is empty
   }
   else if (m_selectedWellsIDs.empty())
   {
      wellsList->setCurrentRow(0);
   }
   else
   {
      QSignalBlocker blocker(wellsList);

      const QVector<const Well*> activeWells = calibrationTargetManager().activeWells();
      for( int i = 0; i < activeWells.size(); ++i)
      {
         int wellID = activeWells[i]->id();
         for ( const int selectedWellID : m_selectedWellsIDs)
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

void LithologyResultsController::slotUpdateWell()
{
   m_selectedWellsIDs = selectedWellsIDs();
   if (m_selectedWellsIDs.size() == 1)
   {
      Logger::log() << "Selected well " << calibrationTargetManager().well(m_selectedWellsIDs.front()).name() << Logger::endl();
   }

   updateBirdView();
   refreshPlot();
}

void LithologyResultsController::slotActiveChanged()
{
   const QVector<bool> activePlots = m_resultsTab->plotOptions()->activePlots();
   m_scenario.setActivePlots(activePlots);

   refreshPlot();
}

void LithologyResultsController::slotUpdateSurfaceLines(const bool showSurfaceLines)
{
   m_scenario.setShowSurfaceLines(showSurfaceLines);

   refreshPlot();
}

void LithologyResultsController::slotUpdateFitRangeToData(const bool fitRangeToData)
{
   m_scenario.setFitRangeToData(fitRangeToData);

   refreshPlot();
}

void LithologyResultsController::slotUpdateIsExpanded(int state, int plotID)
{
   m_resultsTab->multiWellPlot()->setExpanded(state == Qt::CheckState::Checked, plotID);

   refreshPlot();
}


void LithologyResultsController::refreshPlot()
{
   updateOptimizedTable();
   updateWellPlot();
   updateCorrelationPlot();
}

void LithologyResultsController::slotTogglePlotType(const int currentIndex)
{
   if (currentIndex!=1 && m_selectedWellsIDs.size()>1)
   {
      m_selectedWellsIDs.clear();
   }

   m_resultsTab->setPlotType(currentIndex);
   initializeWellSelection();
}

void LithologyResultsController::updateOptimizedTable()
{
   if (m_selectedWellsIDs.size() != 1)
   {
      if (m_selectedWellsIDs.empty())
      {
         m_resultsTab->updateOptimizedLithoTable({},{},{},{});
      }
      return;
   }

   const LithofractionManager& manager{m_scenario.lithofractionManager()};
   double xLoc = 0.0;
   double yLoc = 0.0;

   const Well& well = calibrationTargetManager().well(m_selectedWellsIDs.front());
   xLoc = well.x();
   yLoc = well.y();

   QVector<OptimizedLithofraction> optimizedLithofractions = manager.optimizedInWell(m_selectedWellsIDs.front());
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

void LithologyResultsController::slotUpdateProperty(QString property)
{
   m_activeProperty = property;
   updateCorrelationPlot();
}

void LithologyResultsController::slotUpdateWellFromBirdView(const int lineIndex, const int pointIndex)
{  
   if (lineIndex != 1)
   {
      return;
   }

   QListWidgetItem* item = m_resultsTab->wellsList()->item(pointIndex);
   item->setSelected(!item->isSelected());
}

void LithologyResultsController::slotSelectedWellFromCorrelation(const int wellIndex)
{  
   const QVector<const Well*> wells = calibrationTargetManager().wells();
   if (wellIndex>=wells.size())
   {
      return;
   }

   Logger::log() << "Selected point is in well " << wells[wellIndex]->name() << Logger::endl();
}

QVector<QVector<WellTrajectory>> LithologyResultsController::getAllTrajectories(const QStringList& propertyUserNames) const
{
   WellTrajectoryManager mgrWellPrep;
   if (m_wellPrepWells) mgrWellPrep.updateWellTrajectories(calibrationTargetManager());
   const WellTrajectoryManager& wellTrajectoryManager = m_wellPrepWells ? mgrWellPrep : m_scenario.wellTrajectoryManager();
   return wellTrajectoryManager.trajectoriesInWell(m_selectedWellsIDs, propertyUserNames);
}

void LithologyResultsController::updateWellPlot()
{
   if (m_selectedWellsIDs.size() != 1)
   {
      if (m_selectedWellsIDs.empty())
      {
         m_resultsTab->updateWellPlot({},{},{},{},{}, false);
      }
      return;
   }

   QStringList propertyUserNames;
   const QVector<QVector<const CalibrationTarget*>> targetsInWell = calibrationTargetManager().extractWellTargets(propertyUserNames, m_selectedWellsIDs);

   m_resultsTab->updateWellPlot(targetsInWell,
                               getUnitsForProperties(targetsInWell),
                               getAllTrajectories(propertyUserNames),
                               m_scenario.activePlots(),
                               getSurfaceValues(),
                               m_scenario.fitRangeToData());
}

QMap<QString, double> LithologyResultsController::getSurfaceValues()
{
   QMap<QString, double> surfaceValues;
   if (m_scenario.showSurfaceLines())
   {
      const casaWizard::Well& well = calibrationTargetManager().well(m_selectedWellsIDs[0]);
      CMBMapReader mapReader;
      mapReader.load(m_scenario.project3dPath().toStdString());
      QStringList surfaceNames = m_scenario.projectReader().surfaceNames();
      for (int i = 0; i < surfaceNames.size(); i++)
      {
         QString depthGridName = m_scenario.projectReader().getDepthGridName(i);
         const double depth = -1.0*mapReader.getValue(well.x(), well.y(), depthGridName.toStdString());
         surfaceValues.insert(surfaceNames[i], depth);
      }
   }

   return surfaceValues;
}

QStringList LithologyResultsController::getUnitsForProperties(const QVector<QVector<const CalibrationTarget*>> targets)
{  
   QStringList units;
   for (QVector<const CalibrationTarget*> target : targets)
   {
      const QString cauldronName = calibrationTargetManager().getCauldronPropertyName(target.at(0)->propertyUserName());
      units.push_back(QString::fromStdString(functions::getUnit(cauldronName.toStdString())));
   }
   return units;
}

void LithologyResultsController::updateCorrelationPlot()
{
   QSignalBlocker blocker(m_resultsTab->wellCorrelationPlotLayout());

   QStringList propertyUserNames;
   QVector<QVector<const CalibrationTarget*>> targetsInWell = calibrationTargetManager().extractWellTargets(propertyUserNames, m_selectedWellsIDs);

   if (propertyUserNames.empty())
   {
      m_resultsTab->clearWellCorrelationPlot();
      return;
   }

   int iProperty = 0;
   for (const QString& property: propertyUserNames)
   {
      if (iProperty != 0 && calibrationTargetManager().getCauldronPropertyName(property) == "TwoWayTime")
      {
         propertyUserNames.move(iProperty, 0);
      }
      iProperty++;
   }

   const QVector<QVector<WellTrajectory>> allTrajectories = getAllTrajectories(propertyUserNames);

   assert(targetsInWell.size() ==  allTrajectories[0].size());
   int activePropertyIndex = propertyUserNames.indexOf(m_activeProperty);
   if (activePropertyIndex == -1)
   {
      activePropertyIndex = 0;
      m_activeProperty = propertyUserNames[activePropertyIndex];
   }
   m_resultsTab->wellCorrelationPlotLayout()->setProperties(propertyUserNames, activePropertyIndex);

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
      if (!m_scenario.activePlots()[j])
      {
         continue;
      }

      QVector<double> allValuesMeasured;
      QVector<double> allValuesSimulated;
      for (int i = 0; i < nTrajectories; ++i)
      {
         if (allTrajectories[j][i].propertyUserName() != m_activeProperty ||
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

   m_resultsTab->updateCorrelationPlot(measuredValueTrajectories,
                                      simulatedValueTrajectories,
                                      m_activeProperty,
                                      m_scenario.activePlots(),
                                      minValue,
                                      maxValue,
                                      wellIndices);
}

void LithologyResultsController::updateBirdView()
{
   m_resultsTab->updateSelectedWells();
}

QVector<int> LithologyResultsController::selectedWellsIDs()
{  
   const QVector<const Well*> activeWells = calibrationTargetManager().activeWells();

   QModelIndexList indices = m_resultsTab->wellsList()->selectionModel()->selectedIndexes();

   QVector<int> wellIndices;
   for(const QModelIndex& index : indices)
   {
      wellIndices.push_back(activeWells[index.row()]->id());
   }
   std::sort(wellIndices.begin(), wellIndices.end());
   return wellIndices;
}

} // namespace lithology

} // namespace sac

} // namespace casaWizard
