//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//


#include "predictionTargetManager.h"

#include "model/input/projectReader.h"
#include "model/logger.h"
#include "model/scenarioReader.h"
#include "model/scenarioWriter.h"
#include "model/targetParameterMapCreator.h"

#include <cassert>
#include <numeric>

namespace casaWizard
{

namespace ua
{

QVector<QString> PredictionTargetManager::s_targetVariables{"Temperature","VRe"};


PredictionTargetManager::PredictionTargetManager(const ProjectReader& projectReader, const ToDepthConverter& toDepthConverter) :
   m_predictionTargets{},
   predictionTargetsAllTimes_{},
   m_projectReader{projectReader},
   m_toDepthConverter(toDepthConverter),
   m_identifier{1}
{}

void PredictionTargetManager::addTargets(QVector<double> snapshots)
{
   if (m_targetHasTimeSeries.isEmpty())
   {
      return;
   }

   assert(m_targetHasTimeSeries.size() == m_predictionTargets.size());

   if (m_targetHasTimeSeries.size() != m_predictionTargets.size())
   {
      return;
   }

   int i = 0;
   for (auto target : m_predictionTargets)
   {
      if (m_targetHasTimeSeries[i])
      {
         for (const double age : snapshots)
         {
            PredictionTarget* newTarget = target->createCopy();
            newTarget->setAge(age);
            predictionTargetsAllTimes_.push_back(newTarget);
         }
      }
      else
      {
         PredictionTarget* newTarget = target->createCopy();
         predictionTargetsAllTimes_.push_back(newTarget);
      }
      ++i;
   }
}

PredictionTargetManager::~PredictionTargetManager()
{
   clearMemory();
}

const QVector<const PredictionTarget*> PredictionTargetManager::predictionTargets() const
{
   QVector<const PredictionTarget*> allTargets;
   for (auto target : m_predictionTargets)
   {
      allTargets.push_back(target.get());
   }
   return allTargets;
}

const QVector<const PredictionTarget*> PredictionTargetManager::predictionTargetInTimeSeries(const int row) const
{
   return predictionTargetsIncludingTimeSeries().mid(indexCumulativePredictionTarget(row), sizeOfPredictionTargetWithTimeSeries(row));
}

int PredictionTargetManager::indexCumulativePredictionTarget(const int row) const
{
   const QVector<int> sizesPredTargets = sizeOfPredictionTargetsWithTimeSeries();

   return (row > 0
           ? std::accumulate(sizesPredTargets.begin(), sizesPredTargets.begin() + row, 0)
           : 0);
}

int PredictionTargetManager::indexCumulativePredictionTargetIncludingProperties(const int row) const
{
   const QVector<int> sizesPredTargets = sizeOfPredictionTargetsWithTimeSeriesIncludingProperties();

   return (row > 0
           ? std::accumulate(sizesPredTargets.begin(), sizesPredTargets.begin() + row, 0)
           : 0);
}

void PredictionTargetManager::setTargetActiveProperty(const bool active, const int row, const QString& property)
{
   m_predictionTargets[row]->setPropertyActive(active, property);
   setPredictionTargetsAllTimes();
}

QStringList PredictionTargetManager::validSurfaceNames() const
{
   QStringList validSurfaceNames = m_projectReader.surfaceNames();
   if (!validSurfaceNames.empty())
   {
      validSurfaceNames.removeLast(); // Remove basement, since that is not supported yet
      validSurfaceNames.insert(0, ""); // Add empty entry to be able to deselect the surface in the predictionTargetTable
   }

   return validSurfaceNames;
}

const QVector<const PredictionTarget*> PredictionTargetManager::predictionTargetsIncludingTimeSeries() const
{
   return predictionTargetsAllTimes_;
}

void PredictionTargetManager::setPredictionTargetsAllTimes()
{
   for (const PredictionTarget* target : predictionTargetsAllTimes_)
   {
      delete target;
   }
   predictionTargetsAllTimes_.clear();

   QVector<double> snapshots = m_projectReader.agesFromMajorSnapshots();

   addTargets(snapshots);
}

void PredictionTargetManager::clearMemory()
{
   for (const PredictionTarget* targetTime : predictionTargetsAllTimes_)
   {
      delete targetTime;
   }
}

void PredictionTargetManager::setTarget(int row, int column, const QString& text)
{
   if (row < 0 || row >= m_predictionTargets.size())
   {
      return;
   }
   switch(column)
   {
   case 0:
   {
      break;
   }
   case 1:
   {
      m_predictionTargets[row]->setX(text.toDouble());
      break;
   }
   case 2:
   {
      m_predictionTargets[row]->setY(text.toDouble());
      break;
   }
   case 3:
   {
      PredictionTargetDepth* depthTarget = dynamic_cast<PredictionTargetDepth*>(m_predictionTargets[row].get());
      if (depthTarget != nullptr)
      {
         depthTarget->setZ(text.toDouble());
      }

      break;
   }
   case 4:
   {
      PredictionTargetSurface* surfaceTarget = dynamic_cast<PredictionTargetSurface*>(m_predictionTargets[row].get());
      if (surfaceTarget != nullptr && text != "")
      {
         surfaceTarget->setSurfaceAndLayerName(text, m_projectReader.getLayerUnderSurface(text));
      }
      else
      {
         const PredictionTarget& target = *m_predictionTargets[row];
         const double x = target.x();
         const double y = target.y();
         const double age = target.age();
         const QString locationName = target.locationName();
         QVector<QString> properties = target.properties();

         if (text == "")
         {
            m_predictionTargets[row].reset(new PredictionTargetDepth(properties, x, y, 0, age, locationName));
         }
         else
         {
            PredictionTargetSurface* surfaceTarget = new PredictionTargetSurface(properties, x, y, text, age, &m_toDepthConverter, locationName);
            surfaceTarget->setSurfaceAndLayerName(text, m_projectReader.getLayerUnderSurface(text));
            m_predictionTargets[row].reset(surfaceTarget);
         }
      }
      break;
   }
   }
   setPredictionTargetsAllTimes();
}

void PredictionTargetManager::addDepthTarget(const double x, const double y, const double z, const QVector<QString>& properties, const double age)
{
   m_predictionTargets.push_back(std::make_shared<PredictionTargetDepth>(properties, x, y, z, age, "Loc_" + QString::number(m_identifier)));
   m_targetHasTimeSeries.push_back(false);
   m_identifier++;

   setPredictionTargetsAllTimes();
}

void PredictionTargetManager::addSurfaceTarget(const double x, const double y, QString layer, const QVector<QString>& properties, const double age)
{  
   const QStringList layers = m_projectReader.surfaceNames();
   if (!layers.contains(layer))
   {
      if (layers.empty())
      {
         return;
      }
      else
      {
         layer = layers[0];
      }
   }

   m_predictionTargets.push_back(std::make_shared<PredictionTargetSurface>(properties, x, y, layer, age, &m_toDepthConverter, "Loc_" + QString::number(m_identifier)));
   m_identifier++;

   m_targetHasTimeSeries.push_back(false);
   setPredictionTargetsAllTimes();
}

void PredictionTargetManager::copyTargets(const QVector<int>& indices)
{
   for (int index : indices)
   {
      if (index<0 || index>=m_predictionTargets.size())
      {
         return;
      }

      if (dynamic_cast<PredictionTargetDepth*>(m_predictionTargets[index].get()))
      {
         m_predictionTargets.push_back(std::make_shared<PredictionTargetDepth>(*dynamic_cast<PredictionTargetDepth*>(m_predictionTargets[index].get())));
      }
      if (dynamic_cast<PredictionTargetSurface*>(m_predictionTargets[index].get()))
      {
         m_predictionTargets.push_back(std::make_shared<PredictionTargetSurface>(*dynamic_cast<PredictionTargetSurface*>(m_predictionTargets[index].get())));
      }

      m_targetHasTimeSeries.push_back(m_targetHasTimeSeries[index]);
   }

   setPredictionTargetsAllTimes();
}

void PredictionTargetManager::removeTargets(const QVector<int>& indices)
{
   for (int i = indices.size() - 1; i >= 0; i--)
   {
      const int index = indices[i];
      if (index < 0 || index>=m_predictionTargets.size())
      {
         return;
      }
      m_predictionTargets.remove(index);
      m_targetHasTimeSeries.remove(index);
   }

   setPredictionTargetsAllTimes();
}

void PredictionTargetManager::setPropertyActiveForAllTargets(const QString& propertyName)
{
   bool allTargetsSelected = true;
   for (auto target : m_predictionTargets)
   {
      if (!target->properties().contains(propertyName)) // Check if the property in this target is active
      {
         allTargetsSelected = false;
         break;
      }
   }

   // If the property is active for all targets, set property inactive for all targets.
   // Otherwise, set property active for all targets.
   for (auto target : m_predictionTargets)
   {
      target->setPropertyActive(!allTargetsSelected, propertyName);
   }
}

void PredictionTargetManager::setTimeSeriesActiveForAllTargets()
{
   bool allTargetsHaveTimeSeries = true;
   for (const bool hasTimeSeries : m_targetHasTimeSeries)
   {
      if (!hasTimeSeries) // Check if this prediction target has time series selected
      {
         allTargetsHaveTimeSeries = false;
         break;
      }
   }

   // If the property is active for all wells, set property inactive for all wells.
   // Otherwise, set property active for all wells.
   for (bool& hasTimeSeries : m_targetHasTimeSeries)
   {
      hasTimeSeries = !allTargetsHaveTimeSeries;
   }
}

int PredictionTargetManager::getIndexInPredictionTargetMatrix(const int predictionTargetRow, const int snapshotIndex, const QString& propertyName) const
{
   const QVector<QString> propertiesInTarget = m_predictionTargets[predictionTargetRow]->properties();

   return indexCumulativePredictionTargetIncludingProperties(predictionTargetRow) // Starting point in the matrix
          + snapshotIndex * propertiesInTarget.size() // Multiply snapshot index with number of properties, since the matrices of different properties are stored per snapshot
          + propertiesInTarget.indexOf(propertyName); // Add the index of the property
}


QVector<QString> PredictionTargetManager::predictionTargetOptions()
{
   return s_targetVariables;
}

void PredictionTargetManager::writeToFile(ScenarioWriter& writer) const
{
   const int version = 2;
   writer.writeValue("PredictionTargetManagerVersion", version);
   writer.writeValue("predictionTargets", predictionTargets());
   writer.writeValue("predictionTargetHasTimeSeries", m_targetHasTimeSeries);
   writer.writeValue("targetIdentifier", m_identifier);
}

void PredictionTargetManager::readFromFile(const ScenarioReader& reader)
{
   const int version = reader.readInt("PredictionTargetManagerVersion");

   if (version < 2)
   {
      QVector<PredictionTargetDepth> depthTargets = reader.readVector<PredictionTargetDepth>("depthTarget");

      if (version > 0)
      {
         QVector<bool> depthTargetHasTimeSeries = reader.readVector<bool>("depthTargetHasTimeSeries");
         QVector<PredictionTargetSurface> surfaceTargets = reader.readVector<PredictionTargetSurface>("surfaceTarget");
         QVector<bool> surfaceTargetHasTimeSeries = reader.readVector<bool>("surfaceTargetHasTimeSeries");

         for (const PredictionTargetDepth& target : depthTargets)
         {
            m_predictionTargets.append(std::make_shared<PredictionTargetDepth>(target));
         }
         for (bool hasTimeSeries : depthTargetHasTimeSeries)
         {
            m_targetHasTimeSeries.push_back(hasTimeSeries);
         }
         for (const PredictionTargetSurface& target : surfaceTargets)
         {
            m_predictionTargets.append(std::make_shared<PredictionTargetSurface>(target));
         }
         for (bool hasTimeSeries : surfaceTargetHasTimeSeries)
         {
            m_targetHasTimeSeries.push_back(hasTimeSeries);
         }
      }
   }
   else
   {
      m_predictionTargets = reader.readAndCreateVectorOfSharedPtrs<PredictionTarget>("predictionTargets");
      for (auto target : m_predictionTargets)
      {
         // Set the depth converters & set layerNames
         if (dynamic_cast<PredictionTargetSurface*>(target.get()))
         {
            PredictionTargetSurface* surfaceTarget = dynamic_cast<PredictionTargetSurface*>(target.get());
            surfaceTarget->setToDepthConverterAndCalcDepth(&m_toDepthConverter);

            const QString layerName = m_projectReader.getLayerUnderSurface(surfaceTarget->surfaceName());
            surfaceTarget->setSurfaceAndLayerName(surfaceTarget->surfaceName(), layerName);
         }
      }

      m_targetHasTimeSeries = reader.readVector<bool>("predictionTargetHasTimeSeries");
      m_identifier = reader.readInt("targetIdentifier");
   }

   setPredictionTargetsAllTimes();
}

void PredictionTargetManager::clear()
{
   clearMemory();
   predictionTargetsAllTimes_.clear();
   m_predictionTargets.clear();
   m_targetHasTimeSeries.clear();
   m_identifier = 1;
}

QVector<bool> PredictionTargetManager::targetHasTimeSeries() const
{
   return m_targetHasTimeSeries;
}

void PredictionTargetManager::setTargetHasTimeSeries(const int row, const bool isSelected)
{
   if (row < 0 || row >= m_targetHasTimeSeries.size())
   {
      return;
   }
   m_targetHasTimeSeries[row] = isSelected;
   setPredictionTargetsAllTimes();
}

int PredictionTargetManager::amountAtAge0() const
{
   return m_predictionTargets.size();
}

int PredictionTargetManager::sizeOfPredictionTargetWithTimeSeries(const int row) const
{
   QVector<int> sizes = sizeOfPredictionTargetsWithTimeSeries();
   const int n = sizes.size();
   if (n == 0 || row < 0 || row >= n )
   {
      return 0;
   }

   return sizeOfPredictionTargetsWithTimeSeries()[row];
}

int PredictionTargetManager::sizeOfPredictionTargetsWithTimeSeriesIncludingProperties(const int row) const
{
   QVector<int> sizes = sizeOfPredictionTargetsWithTimeSeries();
   const int n = sizes.size();
   if (n == 0 || row < 0 || row >= n )
   {
      return 0;
   }

   return sizeOfPredictionTargetsWithTimeSeriesIncludingProperties()[row];
}

int PredictionTargetManager::amountOfPredictionTargetWithTimeSeriesAndProperties() const
{
   int totalTargets = 0;
   for (const int numTargetsPerRow : sizeOfPredictionTargetsWithTimeSeriesIncludingProperties())
   {
      totalTargets+=numTargetsPerRow;
   }

   return totalTargets;
}

// Returning vector contains 1 by default and the number of snapshots if the prediction target has a time series
QVector<int> PredictionTargetManager::sizeOfPredictionTargetsWithTimeSeries() const
{
   const int numberSnapshots = m_projectReader.agesFromMajorSnapshots().size();

   QVector<int> sizePredTargets(m_predictionTargets.size(), 1);

   for (int i = 0; i < m_predictionTargets.size(); ++i)
   {
      if (m_targetHasTimeSeries[i])
      {
         sizePredTargets[i] = numberSnapshots;
      }
   }

   return sizePredTargets;
}

QVector<int> PredictionTargetManager::sizeOfPredictionTargetsWithTimeSeriesIncludingProperties() const
{
   const int numberSnapshots = m_projectReader.agesFromMajorSnapshots().size();

   QVector<int> sizePredTargets(m_predictionTargets.size(), 1);

   for (int i = 0; i < m_predictionTargets.size(); ++i)
   {
      if (m_targetHasTimeSeries[i])
      {
         sizePredTargets[i] = numberSnapshots * m_predictionTargets[i]->properties().size();
      }
      else
      {
         sizePredTargets[i] = m_predictionTargets[i]->properties().size();
      }
   }

   return sizePredTargets;
}

} // namespace ua

} // namespace casaWizard
