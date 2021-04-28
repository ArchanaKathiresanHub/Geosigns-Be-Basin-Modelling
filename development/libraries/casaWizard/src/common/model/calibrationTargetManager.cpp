#include "calibrationTargetManager.h"

#include "logger.h"
#include "model/input/cmbMapReader.h"
#include "scenarioReader.h"
#include "scenarioWriter.h"
#include "targetParameterMapCreator.h"
#include "wellValidator.h"

#include <QSet>

#include <algorithm>
#include <assert.h>

namespace casaWizard
{

CalibrationTargetManager::CalibrationTargetManager() :
  wells_{}
{
}

QVector<const CalibrationTarget*> CalibrationTargetManager::calibrationTargets() const
{
  QVector<const CalibrationTarget*> targets;
  for (const Well& well : wells_)
  {
    for (const CalibrationTarget* target : well.calibrationTargets())
    {
      targets.push_back(target);
    }
  }
  return targets;
}

void CalibrationTargetManager::addCalibrationTarget(const QString& name, const QString& property, const int wellIndex,
                                                    const double z, const double value)
{
  if (wellIndex >= wells_.size())
  {
    return;
  }

  wells_[wellIndex].addCalibrationTarget(name, property, z, value);
}

int CalibrationTargetManager::addWell(const QString& wellName, double x, double y)
{
  const int newId = wells_.size();
  Well newWell(newId, wellName, x, y);
  wells_.append(newWell);
  return newId;
}

const QVector<const Well*> CalibrationTargetManager::wells() const
{
  QVector<const Well*> wells;
  for (const Well& well : wells_)
  {
     wells.push_back(&well);
  }
  return wells;
}

const QVector<const Well*> CalibrationTargetManager::activeWells() const
{
  QVector<const Well*> activeWells;
  for (const Well& well : wells_)
  {
    if (well.isActive())
    {
      activeWells.push_back(&well);
    }
  }
  return activeWells;
}

const QVector<const Well*> CalibrationTargetManager::activeAndIncludedWells() const
{
  QVector<const Well*> activeAndIncludedWells;
  for (const Well* well : activeWells())
  {
    if (!well->isExcluded())
    {
      activeAndIncludedWells.push_back(well);
    }
  }
  return activeAndIncludedWells;
}

const Well& CalibrationTargetManager::well(const int wellIndex) const
{
  assert(wellIndex>=0 && wellIndex<wells_.size());
  return wells_[wellIndex];
}

void CalibrationTargetManager::setWellIsActive(bool active, int wellIndex)
{
  assert(wellIndex>=0 && wellIndex<wells_.size());
  wells_[wellIndex].setIsActive(active);
}

void CalibrationTargetManager::setWellIsExcluded(bool excluded, int wellIndex)
{
  assert(wellIndex>=0 && wellIndex<wells_.size());
  wells_[wellIndex].setIsExcluded(excluded);
}

int CalibrationTargetManager::amountOfActiveCalibrationTargets() const
{
  return activeCalibrationTargets().size();
}

void CalibrationTargetManager::clear()
{
  objectiveFunctionManager_.clear();
  wells_.clear();
}

void CalibrationTargetManager::writeToFile(ScenarioWriter& writer) const
{
  writer.writeValue("CalibrationTargetManagerVersion", 0);
  objectiveFunctionManager_.writeToFile(writer);
  QVector<int> wellIds;
  for (const Well& well : wells_)
  {
    well.writeToFile(writer);
    wellIds.push_back(well.id());
  }
  writer.writeValue("wellIndices", wellIds);
}

void CalibrationTargetManager::readFromFile(const ScenarioReader& reader)
{
  const int version = reader.readInt("CalibrationTargetManagerVersion");

  objectiveFunctionManager_.readFromFile(reader);
  wells_.clear();

  QVector<int> wellIds = reader.readVector<int>("wellIndices");
  for (const int wellIndex : wellIds)
  {
    Well newWell;
    newWell.readFromFile(reader, wellIndex);
    wells_.push_back(newWell);
  }  
}

QVector<const CalibrationTarget*> CalibrationTargetManager::activeCalibrationTargets() const
{
  QVector<const CalibrationTarget*> targets;
  for (const Well* well : activeWells())
  {
    for (const CalibrationTarget* target : well->calibrationTargets())
    {
      targets.push_back(target);
    }
  }
  return targets;
}

QStringList CalibrationTargetManager::activeProperties() const
{
  QStringList properties;
  for (const CalibrationTarget* target : activeCalibrationTargets())
  {
    if (!properties.contains(target->property()))
    {
      properties.append(target->property());
    }
  }
  return properties;
}

void CalibrationTargetManager::disableInvalidWells(const std::string& projectFileName, const std::string& depthGridName)
{
  CMBMapReader mapReader;
  mapReader.load(projectFileName);
  WellValidator validator(mapReader);
  for (Well& well : wells_)
  {
    if (!validator.isValid(well, depthGridName))
    {
      well.setIsActive(false);
      well.setIsOutOfBasin(true);
      Logger::log() << "Well " << well.name() << " is outside of the basin model and is therefore disabled." << Logger::endl();
    }
  }
}

QVector<QVector<CalibrationTarget>> CalibrationTargetManager::extractWellTargets(QStringList& properties, const int wellIndex) const
{
  if (wellIndex >= wells_.size() || wellIndex < 0)
  {
    return {};
  }

  properties.clear();
  QVector<QVector<CalibrationTarget>> targetsInWell;

  for (const CalibrationTarget* target : wells_[wellIndex].calibrationTargets())
  {
    const int propertyIndex = properties.indexOf(target->property());
    if (propertyIndex == -1)
    {
      properties.append(target->property());
      QVector<CalibrationTarget> newVector{*target};
      targetsInWell.append(newVector);
    }
    else
    {
      targetsInWell[propertyIndex].append(*target);
    }
  }
  return targetsInWell;
}

QVector<QVector<CalibrationTarget> > CalibrationTargetManager::extractWellTargets(QStringList& properties, const QVector<int> wellIndices) const
{
  QVector<QVector<CalibrationTarget>> targetsInWells;
  if (wellIndices.size()==0)
  {
    return {};
  }

  for (const int wellIndex : wellIndices)
  {
    QStringList wellProperties;
    for (const QVector<CalibrationTarget>& targets : extractWellTargets(wellProperties, wellIndex))
    {
      targetsInWells.append(targets);
    }
    properties.append(wellProperties);
  }

  // Remove duplicate properties
  QSet<QString> setProperties = QSet<QString>::fromList(properties);
  properties = QStringList::fromSet(setProperties);
  std::sort(properties.begin(), properties.end());
  return targetsInWells;
}

const ObjectiveFunctionManager& CalibrationTargetManager::objectiveFunctionManager() const
{
  return objectiveFunctionManager_;
}

void CalibrationTargetManager::updateObjectiveFunctionFromTargets()
{
  QSet<QString> targetVariables;
  for (const CalibrationTarget *const target : calibrationTargets())
  {
    targetVariables.insert(target->property());
  }
  objectiveFunctionManager_.setVariables(targetVariables.toList());
  applyObjectiveFunctionOnCalibrationTargets();
}

void CalibrationTargetManager::applyObjectiveFunctionOnCalibrationTargets()
{
  for(Well& well : wells_)
  {
    well.applyObjectiveFunction(objectiveFunctionManager_);
  }
}

void CalibrationTargetManager::setObjectiveFunction(int row, int col, double value)
{
  objectiveFunctionManager_.setValue(row, col, value);
  applyObjectiveFunctionOnCalibrationTargets();
}

void CalibrationTargetManager::setObjectiveFunctionVariables(const QStringList& variables)
{
  objectiveFunctionManager_.setVariables(variables);
}

} // namespace casaWizard
