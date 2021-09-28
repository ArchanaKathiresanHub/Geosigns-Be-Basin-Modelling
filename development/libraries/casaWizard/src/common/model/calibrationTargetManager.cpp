#include "calibrationTargetManager.h"

#include "logger.h"
#include "model/dtToTwoWayTimeConverter.h"
#include "model/input/cmbMapReader.h"
#include "model/oneDModelDataExtractor.h"
#include "model/vpToDTConverter.h"
#include "scenarioReader.h"
#include "scenarioWriter.h"
#include "targetParameterMapCreator.h"
#include "wellValidator.h"

#include <QSet>

#include <algorithm>
#include <cmath>
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

void CalibrationTargetManager::appendFrom(const CalibrationTargetManager& calibrationTargetManager)
{
  for (const QString& key : calibrationTargetManager.userNameToCauldronNameMapping_.keys())
  {
    if (calibrationTargetManager.userNameToCauldronNameMapping_[key] != "Unknown")
    {
      addToMapping(key, calibrationTargetManager.userNameToCauldronNameMapping_[key]);
    }
  }

  for (const Well& well : calibrationTargetManager.wells_)
  {
    if (well.isActive())
    {
      addWell(well);
    }
  }
}

void CalibrationTargetManager::copyMappingFrom(const CalibrationTargetManager& calibrationTargetManager)
{
  for (const QString& key : calibrationTargetManager.userNameToCauldronNameMapping().keys())
  {
    addToMapping(key, calibrationTargetManager.getCauldronPropertyName(key));
  }
}

void CalibrationTargetManager::renameUserPropertyNameInWells(const QString& oldName, const QString& newName)
{
  if (oldName == newName)
  {
    return;
  }

  for (Well& well : wells_)
  {
    well.renameUserPropertyName(oldName, newName);
  }
}


QMap<QString, QString> CalibrationTargetManager::userNameToCauldronNameMapping() const
{
  return userNameToCauldronNameMapping_;
}

int CalibrationTargetManager::addWell(const QString& wellName, double x, double y)
{    
  double xShift = getShiftToAvoidOverlap(wellName, x, y);
  const int newId = wells_.size();

  Well newWell(newId, wellName, x + xShift, y);
  wells_.append(newWell);
  return newId;
}

int CalibrationTargetManager::addWell(Well well)
{
  int index = 0;
  for (const Well& presentWell : wells_)
  {
    if (well.name() == presentWell.name())
    {
      well.setId(index);
      wells_.replace(index, well);
      return index;
    }
    index++;
  }

  double xShift = getShiftToAvoidOverlap(well.name(), well.x(), well.y());
  well.shift(xShift);

  const int newId = wells_.size();
  well.setId(newId);
  wells_.append(well);
  return newId;
}

double CalibrationTargetManager::getShiftToAvoidOverlap(const QString& wellName, const double x, const double y)
{
  int i = 0;
  double xShift = 0.0;
  while (i<wells_.size())
  {
    if (std::fabs(wells_[i].x() + xShift - x) < 1.0 && std::fabs(wells_[i].y() -  y) < 1.0)
    {
      Logger::log() << "Shifting well " << wellName << " by 1m in x to not overlap with well: " << wells_[i].name() << Logger::endl();
      xShift += 1.0; // Shift 1m so CASA will not see it as the same well
      i = 0;    // Start again, as we might have overlapped with an other well now
    }
    else
    {
      ++i;
    }
  }

  return xShift;
}

void CalibrationTargetManager::setHasDataInLayer(const int wellIndex, QVector<bool> hasDataInLayer)
{
  wells_[wellIndex].setHasDataInLayer(hasDataInLayer);
}

void CalibrationTargetManager::setWellMetaData(const int wellIndex, const QString& metaData)
{
  wells_[wellIndex].setMetaData(metaData);
}

void CalibrationTargetManager::removeCalibrationTargetsFromActiveWellsWithPropertyUserName(const QString& propertyUserName)
{
  for (Well& well: wells_)
  {
    if (well.isActive())
    {
      well.removeCalibrationTargetsWithPropertyUserName(propertyUserName);
    }
  }
}

void CalibrationTargetManager::removeCalibrationTargetsWithUnknownCauldronProperty()
{
  QStringList unknownPropertyUserNames;
  for (const QString& propertyUserName : userNameToCauldronNameMapping_.keys())
  {
    if (getCauldronPropertyName(propertyUserName) == "Unknown")
    {
      unknownPropertyUserNames.push_back(propertyUserName);
    }
  }

  for (const QString& unknownPropertyUserName : unknownPropertyUserNames)
  {
    for (Well& well: wells_)
    {
      well.removeCalibrationTargetsWithPropertyUserName(unknownPropertyUserName);
    }
  }
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
  QStringList usedWellNames;
  for (Well& well : wells_)
  {
    const WellState wellState = validator.wellState(well, depthGridName, usedWellNames);
    usedWellNames.push_back(well.name());

    switch (wellState)
    {
      case WellState::invalidData:
        well.setIsActive(false);
        well.setIsInvalid(true);
        Logger::log() << "Well " << well.name() << " does not have calibration data and is therefore disabled. Check if the input file is valid." << Logger::endl();
        break;
      case WellState::invalidLocation:
        well.setIsActive(false);
        well.setIsInvalid(true);
        Logger::log() << "Well " << well.name() << " is outside of the basin model and is therefore disabled." << Logger::endl();
        break;
      case WellState::invalidDuplicateName:
        well.setIsActive(false);
        well.setIsInvalid(true);
        Logger::log() << "Well " << well.name() << " has a duplicate name and is therefore disabled." << Logger::endl();
        break;
      case WellState::valid:
        break;
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

void CalibrationTargetManager::convertVPtoDT()
{
  const VPToDTConverter converter;

  const QString convertedDTName = "DT_FROM_VP";
  removeCalibrationTargetsFromActiveWellsWithPropertyUserName(convertedDTName);

  for (Well& well : wells_)
  {
    if (well.isActive())
    {
      QStringList properties;
      const QVector<QVector<const CalibrationTarget*>> targetsInWell = extractWellTargets(properties, well.id());

      const QString VelocityUserName = getVelocityUserNameForConversion(properties);

      if (VelocityUserName == "")
      {
        Logger::log() << "Well " << well.name() << " does not have any Velocity data to convert to SonicSlowness." << Logger::endl();
        continue;
      }

      const QVector<const CalibrationTarget*> vpTargets = targetsInWell[properties.indexOf(VelocityUserName)];

      std::vector<double> VP;
      std::vector<double> depth;
      for (const CalibrationTarget* target : vpTargets)
      {
        VP.push_back(target->value());
        depth.push_back(target->z());
      }

      const std::vector<double> DT = converter.convertToDT(VP);

      for (int i = 0; i < DT.size(); i++)
      {
        const QString targetName(QString("SonicSlowness") + "(" +
                                 QString::number(well.x(),'f',1) + "," +
                                 QString::number(well.y(),'f',1) + "," +
                                 QString::number(depth[i],'f',1) + ")");
        addCalibrationTarget(targetName, convertedDTName, well.id(), depth[i], DT[i]);
      }

      well.appendMetaData("Created " + convertedDTName + " targets converted from " + VelocityUserName + ".");
    }
  }

  addToMapping("DT_FROM_VP", "SonicSlowness");
  updateObjectiveFunctionFromTargets();
}

void CalibrationTargetManager::deleteWells(const QVector<int>& wellIDs)
{
  if (wellIDs.empty()) return;
  int iSelect = 0;
  int i = 0;
  while (i < wells_.size())
  {
    if (iSelect < wellIDs.size() && wellIDs[iSelect] == i + iSelect)
    {
      wells_.remove(i);
      ++iSelect;
    }
    else
    {
      wells_[i].setId(i);
      ++i;
    }
  }
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
  for (Well& well : wells_)
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

QString CalibrationTargetManager::getSonicSlownessUserNameForConversion(const QStringList& propertyUserNames)
{
  QStringList sonicSlownessProperties;
  for (const QString& propertyUserName : propertyUserNames)
  {
    if (getCauldronPropertyName(propertyUserName) == "SonicSlowness")
    {
      sonicSlownessProperties.push_back(propertyUserName);
    }
  }

  if (sonicSlownessProperties.empty())
  {
    return "";
  }
  else if (sonicSlownessProperties.contains("DT"))
  {
    return "DT";
  }
  else
  {
    return sonicSlownessProperties[0];
  }
}

QString CalibrationTargetManager::getVelocityUserNameForConversion(const QStringList& propertyUserNames)
{
  QStringList velocityProperties;
  for (const QString& propertyUserName : propertyUserNames)
  {
    if (getCauldronPropertyName(propertyUserName) == "Velocity")
    {
      velocityProperties.push_back(propertyUserName);
    }
  }

  if (velocityProperties.empty())
  {
    return "";
  }
  else if (velocityProperties.contains("VP"))
  {
    return "VP";
  }
  else
  {
    return velocityProperties[0];
  }
}

void CalibrationTargetManager::removeDataOutsideModelDepths(const std::vector<double>& basementDepthsAtActiveWellLocations, const std::vector<double>& mudlineDepthsAtActiveWellLocations)
{
  int counter = 0;
  for (Well& well : wells_)
  {
    if (well.isActive())
    {
      if (well.removeDataBelowDepth(basementDepthsAtActiveWellLocations[counter]))
      {
        Logger::log() << "Data below the basement has been removed for well: " << well.name() << Logger::endl();
        well.appendMetaData("Removed data below basement.");
      }
      if (well.removeDataAboveDepth(mudlineDepthsAtActiveWellLocations[counter]))
      {
        Logger::log() << "Data above the mudline has been removed for well: " << well.name() << Logger::endl();
        well.appendMetaData("Removed data above mudline.");
      }
      counter++;
    }
  }
}

void CalibrationTargetManager::removeWellsOutsideBasinOutline(const std::string& projectFileName, const std::string& depthGridName)
{
  CMBMapReader mapReader;
  mapReader.load(projectFileName);
  WellValidator validator(mapReader);
  QStringList usedWellNames;

  QVector<int> idsToBeRemoved;
  int wellId = 0;
  for (const Well& well : wells_)
  {
    if (well.isActive())
    {
      const WellState wellState = validator.wellState(well, depthGridName, usedWellNames);

      if (wellState == WellState::invalidLocation)
      {
        Logger::log() << "Well " << well.name() << " is outside of the basin model outline and is therefore removed." << Logger::endl();
        idsToBeRemoved.push_back(wellId);
      }
    }
    ++wellId;
  }

  deleteWells(idsToBeRemoved);
}

void CalibrationTargetManager::convertDTtoTWT(const std::string& iterationFolder, const std::string& project3dFilename)
{
  const OneDModelDataExtractor extractor(*this, iterationFolder, project3dFilename);

  const ModelDataPropertyMap data = extractor.extract("TwoWayTime");

  const DTToTwoWayTimeConverter converter;

  const QString convertedTWTName = "TWT_FROM_DT";
  removeCalibrationTargetsFromActiveWellsWithPropertyUserName(convertedTWTName);

  for (Well& well : wells_)
  {
    if (well.isActive())
    {
      QStringList properties;
      const QVector<QVector<const CalibrationTarget*>> targetsInWell = extractWellTargets(properties, well.id());

      const QString sonicSlownessUserName = getSonicSlownessUserNameForConversion(properties);

      if (sonicSlownessUserName == "")
      {
        Logger::log() << "Well " << well.name() << " does not have any SonicSlowness data to convert to TwoWayTime." << Logger::endl();
        continue;
      }

      const QVector<const CalibrationTarget*> dtTargets = targetsInWell[properties.indexOf(sonicSlownessUserName)];
      const DepthPropertyPair modelDataForWell = data.at(well.name().toStdString());

      PropertyVector sonicSlowness;
      DepthVector depth;
      for (const CalibrationTarget* target : dtTargets)
      {
        sonicSlowness.push_back(target->value());
        depth.push_back(target->z());
      }

      const std::vector<double> twt = converter.convertToTWT(sonicSlowness, depth, modelDataForWell.second, modelDataForWell.first);

      for (int i = 0; i < twt.size(); i++)
      {
        const QString targetName(QString("TwoWayTime") + "(" +
                                 QString::number(well.x(),'f',1) + "," +
                                 QString::number(well.y(),'f',1) + "," +
                                 QString::number(depth[i],'f',1) + ")");
        addCalibrationTarget(targetName, convertedTWTName, well.id(), depth[i], twt[i]);
      }

      well.appendMetaData("Created " + convertedTWTName + " targets converted from " + sonicSlownessUserName + ".");
    }
  }

  addToMapping(convertedTWTName, "TwoWayTime");
  updateObjectiveFunctionFromTargets();
}

} // namespace casaWizard
