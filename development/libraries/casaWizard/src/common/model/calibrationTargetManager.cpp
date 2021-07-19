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
  objectiveFunctionManager_{userNameToCauldronNameMapping_},
  userNameToCauldronNameMapping_{},
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

void CalibrationTargetManager::addCalibrationTarget(const QString& name, const QString& propertyUserName,
                                                    const int wellIndex, const double z, const double value)
{
  if (wellIndex >= wells_.size())
  {
    return;
  }

  wells_[wellIndex].addCalibrationTarget(name, propertyUserName, z, value);
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
  userNameToCauldronNameMapping_.clear();
  wells_.clear();
}

void CalibrationTargetManager::writeToFile(ScenarioWriter& writer) const
{
  writer.writeValue("CalibrationTargetManagerVersion", 1);
  objectiveFunctionManager_.writeToFile(writer);
  QVector<int> wellIds;
  for (const Well& well : wells_)
  {
    well.writeToFile(writer);
    wellIds.push_back(well.id());
  }
  writer.writeValue("wellIndices", wellIds);

  int counter = 0;
  for (const QString& key: userNameToCauldronNameMapping_.keys())
  {
    writer.writeValue("MappingEntry_" + QString::number(counter), key + "," + userNameToCauldronNameMapping_.value(key));
    counter++;
  }
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

  if (version > 0)
  {
    int i = 0;
    while (reader.readString("MappingEntry_" + QString::number(i)) != "")
    {
      QStringList entries = reader.readString("MappingEntry_" + QString::number(i)).split(",");
      addToMapping(entries[0], entries[1]);
      i++;
    }
  }

  for (const auto& val : objectiveFunctionManager_.values())
  {
    if (userNameToCauldronNameMapping_.value(val.variableUserName(), "Unknown") == "Unknown")
    {
      addToMapping(val.variableUserName(), val.variableUserName());
    }
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

QStringList CalibrationTargetManager::activePropertyUserNames() const
{
  QStringList properties;
  for (const CalibrationTarget* target : activeCalibrationTargets())
  {
    if (!properties.contains(target->propertyUserName()))
    {
      properties.append(target->propertyUserName());
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

QVector<QVector<const CalibrationTarget*>> CalibrationTargetManager::extractWellTargets(QStringList& propertyUserNames, const int wellIndex) const
{

  if (wellIndex >= wells_.size() || wellIndex < 0)
  {
    return {};
  }
  QVector<QVector<const CalibrationTarget*>> targetsInWell;
  propertyUserNames.clear();

  for (const CalibrationTarget* target : wells_[wellIndex].calibrationTargets())
  {
    const int propertyIndex = propertyUserNames.indexOf(target->propertyUserName());
    if (propertyIndex == -1)
    {
      propertyUserNames.append(target->propertyUserName());
      QVector<const CalibrationTarget*> newVector{target};
      targetsInWell.append(newVector);
    }
    else
    {
      targetsInWell[propertyIndex].append(target);
    }
  }

  return targetsInWell;
}

QStringList CalibrationTargetManager::getPropertyUserNamesForWell(const int wellIndex) const
{
  if (wellIndex >= wells_.size() || wellIndex < 0)
  {
    return {};
  }

  QStringList propertyUserNames;
  for (const CalibrationTarget* target : wells_[wellIndex].calibrationTargets())
  {
    const int propertyIndex = propertyUserNames.indexOf(target->propertyUserName());
    if (propertyIndex == -1)
    {
      propertyUserNames.append(target->propertyUserName());
    }
  }

  return propertyUserNames;
}


QVector<QVector<const CalibrationTarget*> > CalibrationTargetManager::extractWellTargets(QStringList& propertyUserNames, const QVector<int> wellIndices) const
{
  QVector<QVector<const CalibrationTarget*>> targetsInWells;
  if (wellIndices.size()==0)
  {
    return {};
  }

  for (const int wellIndex : wellIndices)
  {
    QStringList wellProperties;
    for (const QVector<const CalibrationTarget*>& targets : extractWellTargets(wellProperties, wellIndex))
    {
      targetsInWells.append(targets);
    }
    propertyUserNames.append(wellProperties);
  }

  // Remove duplicate properties
  QSet<QString> setProperties = propertyUserNames.toSet();
  propertyUserNames = setProperties.toList();
  std::sort(propertyUserNames.begin(), propertyUserNames.end());
  return targetsInWells;
}

const ObjectiveFunctionManager& CalibrationTargetManager::objectiveFunctionManager() const
{
  return objectiveFunctionManager_;
}

void CalibrationTargetManager::updateObjectiveFunctionFromTargets()
{
  QSet<QString> targetVariableUserNames;
  for (const CalibrationTarget *const target : calibrationTargets())
  {
    targetVariableUserNames.insert(target->propertyUserName());
  }
  objectiveFunctionManager_.setVariables(targetVariableUserNames.toList());
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

void CalibrationTargetManager::setObjectiveFunctionEnabledState(const bool state, const int row)
{
  objectiveFunctionManager_.setEnabledState(state, row);
}

bool CalibrationTargetManager::propertyIsActive(const QString& property) const
{
  return objectiveFunctionManager_.enabled(objectiveFunctionManager_.indexOfUserName(property));
}

void CalibrationTargetManager::addToMapping(const QString& userName, const QString& cauldronName)
{
  userNameToCauldronNameMapping_[userName] = cauldronName;
}

QString CalibrationTargetManager::getCauldronPropertyName(const QString& userPropertyName) const
{
  return userNameToCauldronNameMapping_.value(userPropertyName, "Unknown");
}


} // namespace casaWizard
