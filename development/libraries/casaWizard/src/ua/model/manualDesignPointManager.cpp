//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "manualDesignPointManager.h"

#include "model/scenarioReader.h"
#include "model/scenarioWriter.h"

#include "model/logger.h"
#include "model/input/dataFileParser.h"

#include <cmath>
#include <assert.h>

namespace
{
double s_minManualPointDistance = 0.01;
}

namespace casaWizard
{

namespace ua
{

ManualDesignPointManager::ManualDesignPointManager() :
   m_numberOfParameters{0}
{
}

void ManualDesignPointManager::addDesignPoint()
{
   addDesignPoint(QVector<double>(m_numberOfParameters));
   updateVisualIndexToPointIndexMap();
}

bool ManualDesignPointManager::addDesignPoint(const QVector<double> newPoint)
{
   if (newPoint.size() != m_numberOfParameters)
   {
      return false;
   }

   if (pointTooCloseToExistingPoints(newPoint))
   {
      return false;
   }

   m_manualDesignPoints.push_back(ManualDesignPoint(newPoint));
   updateVisualIndexToPointIndexMap();
   return true;
}

bool ManualDesignPointManager::pointTooCloseToExistingPoints(const QVector<double> point) const
{   
   for (const auto & existingPoint : m_manualDesignPoints)
   {
      if (existingPoint.hidden())
      {
         continue;
      }

      const QVector<double>& influentialParameterValues = existingPoint.influentialParameterValues();
      if (influentialParameterValues.size() != point.size())
      {
         return true;
      }

      if (point.size() == 0)
      {
         return false;
      }

      double distance(0);
      for (int i = 0; i < influentialParameterValues.size(); i++)
      {
         double d = influentialParameterValues[i]-point[i];
         distance += d*d;
      }
      distance = std::sqrt(distance);

      if (distance < s_minManualPointDistance)
      {
         return true;
      }
   }
   return false;
}

void ManualDesignPointManager::removeDesignPoint(const int visiblePointIndex)
{
   int manualPointIndex = m_visualIndexToPointIndexMap.find(visiblePointIndex).value();

   if ( manualPointIndex >= m_manualDesignPoints.size() || manualPointIndex < 0)
   {
      return;
   }

   ManualDesignPoint::SimulationState state = m_manualDesignPoints[manualPointIndex].simulationState();

   switch (state)
   {
   case ManualDesignPoint::SimulationState::Failed:
      //Failed simulations remain present in casa. Removing them would result in a mismatch between the wizard and casa state.
      m_manualDesignPoints[manualPointIndex].hide();
      break;
   case ManualDesignPoint::SimulationState::NotRun:
      m_manualDesignPoints.remove(manualPointIndex);
      break;
   case ManualDesignPoint::SimulationState::Completed:
      break;
   }
   updateVisualIndexToPointIndexMap();
}

void ManualDesignPointManager::updateVisualIndexToPointIndexMap()
{
   m_visualIndexToPointIndexMap.clear();
   int visualIndex(0);
   for (int i = 0; i < m_manualDesignPoints.size(); i++)
   {
      const auto& point = m_manualDesignPoints[i];
      if (!point.hidden())
      {
         m_visualIndexToPointIndexMap[visualIndex] = i;
         visualIndex++;
      }
   }
}

void ManualDesignPointManager::addInfluentialParameter(const int amount)
{
   m_numberOfParameters += amount;
   for (int i = 0; i<amount; ++i)
   {
      for (auto& designPoint : m_manualDesignPoints)
      {
         designPoint.addInfluentialParameter(0);
      }
   }
}

void ManualDesignPointManager::removeInfluentialParameter(const int index)
{
   if (index >= m_numberOfParameters || index < 0)
   {
      return;
   }

   for (auto& designPoint : m_manualDesignPoints)
   {
      designPoint.removeParameter(index);
   }
   m_numberOfParameters--;
}

int ManualDesignPointManager::numberOfVisiblePoints() const
{
   int nVisiblePoints(0);
   for (int i = 0; i < m_manualDesignPoints.size(); i++)
   {
      if (!m_manualDesignPoints[i].hidden())
      {
         nVisiblePoints++;
      }
   }
   return nVisiblePoints;
}

int ManualDesignPointManager::numberOfParameters() const
{
   return m_numberOfParameters;
}

void ManualDesignPointManager::readAndSetCompletionStates(const QString& stateFileName, const QString& doeIndicesFileName, const QStringList& namesDoeOptionsSelected)
{
   int userDefinedIndex = namesDoeOptionsSelected.indexOf("UserDefined");
   if (userDefinedIndex < 0)
   {
      return; //No user defined points were run.
   }

   const QVector<int> simStates = DataFileParser<int>::colDominantMatrix(stateFileName).at(0);
   const QVector<QVector<int>> doeIndices = DataFileParser<int>::readFile(doeIndicesFileName);

   if (doeIndices.size() != namesDoeOptionsSelected.size())
   {
      Logger::log() << "Warning: Mismatch between doeIndices returned from casa and wizard state. Completion states were not read." << Logger::endl();
      return;
   }

   const QVector<int>& indicesUserDefined = doeIndices.at(userDefinedIndex);

   if (indicesUserDefined.size() != m_manualDesignPoints.size())
   {
      Logger::log() << "Warning: Mismatch between doeIndices returned from casa and wizard state. Completion states were not read."<< Logger::endl();
      return;
   }

   for (int i = 0; i <indicesUserDefined.size(); i++)
   {
      if (simStates.size() <= indicesUserDefined[i] || indicesUserDefined[i] < 0)
      {
         Logger::log() << "Warning: Mismatch between doeIndices and simulation states. Completion states of manual design points might not be set correctly."<< Logger::endl();
         continue;
      }

      if (simStates.at(indicesUserDefined[i]) == 1)
      {
         setCompleted(i);
      }
      else if (!m_manualDesignPoints[i].hidden())
      {
         setCompleted(i,false);
         m_manualDesignPoints[i].hide();
         ManualDesignPoint pointCopy(m_manualDesignPoints[i].influentialParameterValues());
         m_manualDesignPoints.push_back(pointCopy);
      }
   }
   updateVisualIndexToPointIndexMap();
}

void ManualDesignPointManager::setParameterValue(const int visiblePointIndex, const int parameterIndex, const double value)
{
   int manualPointIndex = m_visualIndexToPointIndexMap.find(visiblePointIndex).value();

   if ( manualPointIndex >= m_manualDesignPoints.size() || manualPointIndex < 0)
   {
      return;
   }
   m_manualDesignPoints[manualPointIndex].setParameterValue(parameterIndex,value);
}

int ManualDesignPointManager::numberOfCasesToRun() const
{
   int result  = 0;
   for (const auto& point : m_manualDesignPoints)
   {
      if (point.simulationState() == ManualDesignPoint::SimulationState::NotRun)
      {
         result ++;
      }
   }
   return result;
}

QVector<QVector<double>> ManualDesignPointManager::pointsToRun() const
{
   QVector<QVector<double>> pointsToRun;
   for (const auto& point : m_manualDesignPoints)
   {
      if (point.simulationState() == ManualDesignPoint::SimulationState::NotRun)
      {
         pointsToRun.push_back(point.influentialParameterValues());
      }
   }
   return pointsToRun;
}

QVector<QVector<double>> ManualDesignPointManager::parameters() const
{
   QVector<QVector<double>> parameters;
   for (int i = 0; i < m_manualDesignPoints.size(); i++)
   {
      const auto& point = m_manualDesignPoints[i];
      if (!point.hidden())
      {
         parameters.push_back(point.influentialParameterValues());
      }
   }
   return parameters;
}

QVector<bool> ManualDesignPointManager::completed() const
{
   QVector<bool> completed;
   for (const auto& point : m_manualDesignPoints)
   {
      if (!point.hidden())
      {
         completed.push_back(point.simulationState() == ManualDesignPoint::SimulationState::Completed);
      }
   }
   return completed;
}

void ManualDesignPointManager::setCompleted(int idx, bool completionState)
{
   if (idx < m_manualDesignPoints.size())
   {
      m_manualDesignPoints[idx].setSimulationState(completionState);
   }
   else
   {
      Logger::log() << "Error when changing completion state of manual design point: index " << QString::number(idx) << " not found" << Logger::endl();
   }
}

bool ManualDesignPointManager::isCompleted(int visiblePointIndex) const
{
   int manualPointIndex = m_visualIndexToPointIndexMap.find(visiblePointIndex).value();

   if ( manualPointIndex >= m_manualDesignPoints.size() || manualPointIndex < 0)
   {
      return false;
   }
   return m_manualDesignPoints.at(manualPointIndex).simulationState() == ManualDesignPoint::SimulationState::Completed;
}

void ManualDesignPointManager::setAllIncomplete()
{
   for (auto& point : m_manualDesignPoints)
   {
      point.setSimulationState(ManualDesignPoint::SimulationState::NotRun);
   }
}

void ManualDesignPointManager::removeHiddenPoints()
{
   QMutableVectorIterator<ManualDesignPoint> i(m_manualDesignPoints);
   while (i.hasNext())
   {
      i.next();
      if (i.value().hidden())
      {
         i.remove();
      }
   }
   updateVisualIndexToPointIndexMap();
}

void ManualDesignPointManager::writeToFile(ScenarioWriter& writer) const
{
   writer.writeValue("ManualDesignPointManagerVersion", 1);
   writer.writeValue("manualDesignPoints", m_manualDesignPoints);
   writer.writeValue("manualDesignPointParameters", m_numberOfParameters);
}

void ManualDesignPointManager::readFromFile(const ScenarioReader& reader)
{
   int version = reader.readInt("ManualDesignPointManagerVersion");
   if (version == 0)
   {
      QVector<QVector<double>> influentialParameterValues = reader.readVector<QVector<double>>("manualDesignPoints");
      QVector<bool>  completed = reader.readVector<bool>("manualDesignPointCompleted");

      for (int i = 0; i < influentialParameterValues.size(); i++)
      {
         m_manualDesignPoints.push_back(ManualDesignPoint(influentialParameterValues[i]));

         if (completed[i])
         {
            m_manualDesignPoints.back().setSimulationState(ManualDesignPoint::SimulationState::Completed);
         }
         else
         {
            m_manualDesignPoints.back().setSimulationState(ManualDesignPoint::SimulationState::NotRun);
         }
      }
   }
   else
   {
      m_manualDesignPoints = reader.readVector<ManualDesignPoint>("manualDesignPoints");
   }

   m_numberOfParameters = reader.readInt("manualDesignPointParameters");
   updateVisualIndexToPointIndexMap();
}

void ManualDesignPointManager::clear()
{
   m_manualDesignPoints.clear();
   m_numberOfParameters = 0;
}

} // namespace ua

} // namespace casaWizard
