//
// Copyright (C) 2012-2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "SacResultsController.h"

#include "control/ScriptRunController.h"

#include "model/functions/cauldronPropertyUnits.h"
#include "model/functions/interpolateVector.h"

#include "model/input/cmbMapReader.h"
#include "model/logger.h"
#include "model/SacScenario.h"

#include "view/plots/wellBirdsView.h"
#include "view/plots/wellCorrelationPlot.h"
#include "view/assets/multiWellPlot.h"
#include "view/assets/PlotOptions.h"
#include "view/SacResultsTab.h"
#include "view/assets/wellCorrelationPlotLayout.h"

#include "ConstantsMathematics.h"

#include <QListWidget>

#include <assert.h>

namespace casaWizard
{

namespace sac
{

SacResultsController::SacResultsController(SacResultsTab* resultsTab,
                                           ScriptRunController& scriptRunController,
                                           QObject* parent) :
   QObject(parent),
   m_scriptRunController{scriptRunController},
   m_activeProperty{""},
   m_wellPrepWells{false},
   m_selectedWellsIDs{}
{
   connect(resultsTab->plotOptions(), SIGNAL(activeChanged()), this, SLOT(slotActiveChanged()));
   connect(resultsTab->plotOptions(), SIGNAL(plotTypeChange(int)), this, SLOT(slotTogglePlotType(int)));
   connect(resultsTab->wellCorrelationPlotLayout(), SIGNAL(propertyChanged(QString)), this, SLOT(slotUpdateProperty(QString)));

   connect(resultsTab->wellsList(), SIGNAL(itemSelectionChanged()), this, SLOT(slotUpdateWell()));
   connect(resultsTab->wellBirdsView(), SIGNAL(pointSelectEvent(int,int)), this, SLOT(slotUpdateWellFromBirdView(int, int)));
   connect(resultsTab->wellCorrelationPlot(), SIGNAL(selectedWell(int)), this, SLOT(slotSelectedWellFromCorrelation(int)));
   connect(resultsTab->multiWellPlot(), SIGNAL(isExpandedChanged(int, int)), this, SLOT(slotUpdateIsExpanded(int, int)));
   connect(resultsTab->multiWellPlot(), SIGNAL(showSurfaceLinesChanged(bool)), this, SLOT(slotUpdateSurfaceLines(bool)));
   connect(resultsTab->multiWellPlot(), SIGNAL(fitRangeToDataChanged(bool)), this, SLOT(slotUpdateFitRangeToData(bool)));
}

const CalibrationTargetManager& SacResultsController::calibrationTargetManager() const
{
   return m_wellPrepWells ? scenario().calibrationTargetManagerWellPrep() : scenario().calibrationTargetManager();
}

void SacResultsController::refreshGUI()
{
   setActiveWells();
   setActivePropertiesToWells();

   setGuiOptionsInPlots();
   setDomainBirdsView();

   initializeWellSelection();
}

void SacResultsController::setActiveWells()
{
   resultsTab()->updateWellList(calibrationTargetManager().activeWells());
   resultsTab()->updateBirdsView(calibrationTargetManager().activeWells());
}

void SacResultsController::setActivePropertiesToWells()
{
   resultsTab()->updateActivePropertyUserNames(calibrationTargetManager().activePropertyUserNames());
}

void SacResultsController::setGuiOptionsInPlots()
{
   resultsTab()->plotOptions()->setActivePlots(scenario().activePlots());
   resultsTab()->multiWellPlot()->setShowSurfaceLines(scenario().showSurfaceLines());
   resultsTab()->multiWellPlot()->setFitRangeToData(scenario().fitRangeToData());
}

void SacResultsController::setDomainBirdsView()
{
   double xMin = 0; double xMax = 1; double yMin = 0; double yMax = 1;
   scenario().projectReader().domainRange(xMin, xMax, yMin, yMax);
   resultsTab()->setRangeBirdsView(xMin * Utilities::Maths::MeterToKilometer,
                                   xMax * Utilities::Maths::MeterToKilometer,
                                   yMin * Utilities::Maths::MeterToKilometer,
                                   yMax * Utilities::Maths::MeterToKilometer);

   CMBMapReader mapReader;
   mapReader.load(scenario().project3dPath().toStdString());
   VectorVectorMap depthMap = mapReader.getMapData(scenario().projectReader().getDepthGridName(0).toStdString());
   depthMap.setDefinedValuesToOne();
   resultsTab()->wellBirdsView()->setFixedValueRange({0,1});
   resultsTab()->wellBirdsView()->updatePlots(depthMap.getData());
}

void SacResultsController::initializeWellSelection()
{
   QListWidget* wellsList = resultsTab()->wellsList();
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

bool SacResultsController::wellPrepWells() const
{
   return m_wellPrepWells;
}

void SacResultsController::setWellPrepWells(bool state)
{
   m_wellPrepWells = state;
}

void SacResultsController::slotUpdateWell()
{
   m_selectedWellsIDs = RefreshSelectedWellsIDs();
   if (m_selectedWellsIDs.size() == 1)
   {
      Logger::log() << "Selected well " << calibrationTargetManager().well(m_selectedWellsIDs.front()).name() << Logger::endl();
   }

   updateBirdView();
   refreshPlot();
}

void SacResultsController::slotActiveChanged()
{
   const QVector<bool> activePlots = resultsTab()->plotOptions()->activePlots();
   scenario().setActivePlots(activePlots);

   refreshPlot();
}

void SacResultsController::slotUpdateSurfaceLines(const bool showSurfaceLines)
{
   scenario().setShowSurfaceLines(showSurfaceLines);

   refreshPlot();
}

void SacResultsController::slotUpdateFitRangeToData(const bool fitRangeToData)
{
   scenario().setFitRangeToData(fitRangeToData);

   refreshPlot();
}

void SacResultsController::slotUpdateIsExpanded(int state, int plotID)
{
   resultsTab()->multiWellPlot()->setExpanded(state == Qt::CheckState::Checked, plotID);

   refreshPlot();
}


void SacResultsController::refreshPlot()
{
   updateOptimizedTable();
   updateWellPlot();
   updateCorrelationPlot();
}

void SacResultsController::slotTogglePlotType(const int currentIndex)
{
   resultsTab()->setPlotType(currentIndex);
   initializeWellSelection();
}

void SacResultsController::slotUpdateProperty(QString property)
{
   m_activeProperty = property;
   updateCorrelationPlot();
}

void SacResultsController::slotUpdateWellFromBirdView(const int lineIndex, const int pointIndex)
{
   if (lineIndex != 1)
   {
      return;
   }

   QListWidgetItem* item = resultsTab()->wellsList()->item(pointIndex);
   item->setSelected(!item->isSelected());
}

void SacResultsController::slotSelectedWellFromCorrelation(const int wellIndex)
{
   const QVector<const Well*> wells = calibrationTargetManager().wells();
   if (wellIndex>=wells.size())
   {
      return;
   }

   Logger::log() << "Selected point is in well " << wells[wellIndex]->name() << Logger::endl();
}

QVector<QVector<WellTrajectory>> SacResultsController::getAllTrajectories(const QStringList& propertyUserNames) const
{
   WellTrajectoryManager mgrWellPrep;
   if (m_wellPrepWells) mgrWellPrep.updateWellTrajectories(calibrationTargetManager());
   const WellTrajectoryManager& wellTrajectoryManager = m_wellPrepWells ? mgrWellPrep : scenario().wellTrajectoryManager();
   return wellTrajectoryManager.trajectoriesInWell(m_selectedWellsIDs, propertyUserNames);
}

void SacResultsController::updateWellPlot()
{
   if (m_selectedWellsIDs.size() != 1)
   {
      if (m_selectedWellsIDs.empty())
      {
         resultsTab()->updateWellPlot({},{},{},{},{}, false);
      }
      return;
   }

   QStringList propertyUserNames;
   const QVector<QVector<const CalibrationTarget*>> targetsInWell = calibrationTargetManager().extractWellTargets(propertyUserNames, m_selectedWellsIDs);

   resultsTab()->updateWellPlot(targetsInWell,
                                getUnitsForProperties(targetsInWell),
                                getAllTrajectories(propertyUserNames),
                                scenario().activePlots(),
                                getSurfaceValues(),
                                scenario().fitRangeToData());
}

QMap<QString, double> SacResultsController::getSurfaceValues()
{
   QMap<QString, double> surfaceValues;
   if (scenario().showSurfaceLines())
   {
      const casaWizard::Well& well = calibrationTargetManager().well(m_selectedWellsIDs[0]);
      CMBMapReader mapReader;
      mapReader.load(scenario().project3dPath().toStdString());
      QStringList surfaceNames = scenario().projectReader().surfaceNames();
      for (int i = 0; i < surfaceNames.size(); i++)
      {
         QString depthGridName = scenario().projectReader().getDepthGridName(i);
         const double depth = -1.0*mapReader.getValue(well.x(), well.y(), depthGridName.toStdString());
         surfaceValues.insert(surfaceNames[i], depth);
      }
   }

   return surfaceValues;
}

QStringList SacResultsController::getUnitsForProperties(const QVector<QVector<const CalibrationTarget*>> targets)
{
   QStringList units;
   for (QVector<const CalibrationTarget*> target : targets)
   {
      const QString cauldronName = calibrationTargetManager().getCauldronPropertyName(target.at(0)->propertyUserName());
      units.push_back(QString::fromStdString(functions::getUnit(cauldronName.toStdString())));
   }
   return units;
}

void SacResultsController::updateCorrelationPlot()
{
   QSignalBlocker blocker(resultsTab()->wellCorrelationPlotLayout());

   QStringList propertyUserNames;
   QVector<QVector<const CalibrationTarget*>> targetsInWell = calibrationTargetManager().extractWellTargets(propertyUserNames, m_selectedWellsIDs);

   if (propertyUserNames.empty())
   {
      resultsTab()->clearWellCorrelationPlot();
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
   resultsTab()->wellCorrelationPlotLayout()->setProperties(propertyUserNames, activePropertyIndex);

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
      if (!scenario().activePlots()[j])
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

   resultsTab()->updateCorrelationPlot(measuredValueTrajectories,
                                       simulatedValueTrajectories,
                                       m_activeProperty,
                                       scenario().activePlots(),
                                       minValue,
                                       maxValue,
                                       wellIndices);
}

void SacResultsController::updateBirdView()
{
   resultsTab()->updateSelectedWells();
}

QVector<int> SacResultsController::RefreshSelectedWellsIDs()
{
   const QVector<const Well*> activeWells = calibrationTargetManager().activeWells();

   QModelIndexList indices = resultsTab()->wellsList()->selectionModel()->selectedIndexes();

   QVector<int> wellIndices;
   if (indices.size() != 0 && activeWells.size() != 0 )
   {  
      for(const QModelIndex& index : indices)
      {
         wellIndices.push_back(activeWells[index.row()]->id());
      }
      std::sort(wellIndices.begin(), wellIndices.end());
   }
   return wellIndices;
}

QVector<int>& SacResultsController::selectedWellsIDs()
{
   return m_selectedWellsIDs;
}

} // namespace sac

} // namespace casaWizard
