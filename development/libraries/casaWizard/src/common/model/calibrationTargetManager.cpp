#include "calibrationTargetManager.h"

#include "logger.h"
#include "model/DTToTwoWayTimeConverter.h"
#include "model/input/cmbMapReader.h"
#include "model/oneDModelDataExtractor.h"
#include "model/vpToDTConverter.h"
#include "scenarioReader.h"
#include "scenarioWriter.h"
#include "targetParameterMapCreator.h"
#include "wellValidator.h"
#include "WellDataManipulator.h"

#include <QSet>

#include <algorithm>
#include <cmath>
#include <assert.h>

namespace casaWizard
{

CalibrationTargetManager::CalibrationTargetManager() :
   m_userNameToCauldronNameMapping{},
   m_userNameUnits{},
   m_wells{},
   m_showPropertiesInTable{true}
{
}

QVector<const CalibrationTarget*> CalibrationTargetManager::calibrationTargets() const
{
   QVector<const CalibrationTarget*> targets;
   for (const Well& well : m_wells)
   {
      for (const CalibrationTarget* target : well.calibrationTargets())
      {
         targets.push_back(target);
      }
   }
   return targets;
}

QMap<QString, QSet<int>> CalibrationTargetManager::getPropertyNamesPerWellForTargetTable() const
{
   QMap<QString, QSet<int>> properties;
   if (m_showPropertiesInTable)
   {
      properties = getPropertyNamesPerWell();
   }

   return properties;
}

QMap<QString, QSet<int>> CalibrationTargetManager::getPropertyNamesPerWell() const
{
   QMap<QString, QSet<int>> propertyNamesPerWell;
   for (const Well& well : m_wells)
   {
      const int wellId = well.id();
      for ( const QString& propertyUserName : getPropertyUserNamesForWell(wellId))
      {
         propertyNamesPerWell[propertyUserName].insert(wellId);
      }
   }
   return propertyNamesPerWell;
}

void CalibrationTargetManager::addCalibrationTarget(const QString& name, const QString& propertyUserName,
                                                    const int wellIndex, const double z, const double value,
                                                    double standardDeviation, double uaWeight)
{
   if (wellIndex >= m_wells.size())
   {
      return;
   }

   m_wells[wellIndex].addCalibrationTarget(name, propertyUserName, z, value, standardDeviation, uaWeight);
}

void CalibrationTargetManager::appendFrom(const CalibrationTargetManager& calibrationTargetManager)
{
   for (const QString& key : calibrationTargetManager.m_userNameToCauldronNameMapping.keys())
   {
      if (calibrationTargetManager.m_userNameToCauldronNameMapping[key] != "Unknown")
      {
         addToMapping(key, calibrationTargetManager.m_userNameToCauldronNameMapping[key]);
      }
   }

   for (const Well& well : calibrationTargetManager.m_wells)
   {
      if (well.isIncludedInOptimization() )
      {
         addWell(well);
      }
   }
}

void CalibrationTargetManager::copyMappingFrom(const CalibrationTargetManager& calibrationTargetManager)
{
   for (const QString& key : calibrationTargetManager.m_userNameToCauldronNameMapping.keys())
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

   for (Well& well : m_wells)
   {
      well.renameUserPropertyName(oldName, newName);
   }
}

QMap<QString, QString> CalibrationTargetManager::userNameToCauldronNameMapping() const
{
   return m_userNameToCauldronNameMapping;
}

int CalibrationTargetManager::addWell(const QString& wellName, double x, double y)
{    
   double xShift = getShiftToAvoidOverlap(wellName, x, y);
   const int newId = m_wells.size();

   Well newWell(newId, wellName, x + xShift, y);
   m_wells.append(newWell);
   return newId;
}

int CalibrationTargetManager::addWell(Well well)
{
   int index = 0;
   for (const Well& presentWell : m_wells)
   {
      if (well.name() == presentWell.name())
      {
         well.setId(index);
         well.setX(presentWell.x());
         well.setY(presentWell.y());
         m_wells.replace(index, well);

         Logger::log() << "Well: " << well.name() << " is overwritten by the newly imported well with the same name." << Logger::endl();

         return index;
      }
      index++;
   }

   double xShift = getShiftToAvoidOverlap(well.name(), well.x(), well.y());
   well.shift(xShift);

   const int newId = m_wells.size();
   well.setId(newId);
   m_wells.append(well);
   return newId;
}

double CalibrationTargetManager::getShiftToAvoidOverlap(const QString& wellName, const double x, const double y)
{
   int i = 0;
   double xShift = 0.0;
   while (i<m_wells.size())
   {
      if (std::fabs(m_wells[i].x() - xShift - x) < 1.0 && std::fabs(m_wells[i].y() -  y) < 1.0)
      {
         Logger::log() << "Shifting well " << wellName << " by 1m in x to not overlap with well: " << m_wells[i].name() << Logger::endl();
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

void CalibrationTargetManager::setWellMetaData(const int wellIndex, const QString& metaData)
{
   m_wells[wellIndex].setMetaData(metaData);
}

void CalibrationTargetManager::scaleData(const QStringList& selectedProperties, const double scalingFactor)
{
   WellDataManipulator::scaleData(selectedProperties, scalingFactor, m_wells);
}

void CalibrationTargetManager::smoothenData(const QStringList& selectedProperties, const double radius)
{
   WellDataManipulator::smoothenData(selectedProperties, radius, m_wells);
}

void CalibrationTargetManager::subsampleData(const QStringList& selectedProperties, const double length)
{
   WellDataManipulator::subsampleData(selectedProperties, length, m_wells);
}

void CalibrationTargetManager::applyCutOff(const QMap<QString, QPair<double, double>>& propertiesWithCutOffRanges)
{
   WellDataManipulator::applyCutOff(propertiesWithCutOffRanges, m_wells);
}

void CalibrationTargetManager::setWellActiveProperty(const QString& property, const bool active, const int wellIndex)
{
   assert(wellIndex>=0 && wellIndex<m_wells.size());
   m_wells[wellIndex].setPropertyActive(property, active);
}

QMap<QString, QString> CalibrationTargetManager::userNameUnits() const
{
   return m_userNameUnits;
}

const QVector<const Well*> CalibrationTargetManager::wells() const
{
   QVector<const Well*> wells;
   for (const Well& well : m_wells)
   {
      wells.push_back(&well);
   }
   return wells;
}

const QVector<const Well*> CalibrationTargetManager::activeWells() const
{
   QVector<const Well*> activeWells;
   for (const Well& well : m_wells)
   {
      if ( well.isIncludedInOptimization() )
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
   assert(wellIndex>=0 && wellIndex<m_wells.size());
   return m_wells[wellIndex];
}

void CalibrationTargetManager::setWellHasActiveProperties(bool hasActiveProperties, int wellIndex)
{
   assert(wellIndex>=0 && wellIndex<m_wells.size());
   m_wells[wellIndex].setHasActiveProperties(hasActiveProperties);
}

void CalibrationTargetManager::setWellIsInvalid(bool invalid, int wellIndex)
{
   assert(wellIndex>=0 && wellIndex<m_wells.size());
   m_wells[wellIndex].setIsInvalid(invalid);
}

void CalibrationTargetManager::setWellIsActive(bool active, int wellIndex)
{
   assert(wellIndex>=0 && wellIndex<m_wells.size());
   m_wells[wellIndex].setIsActive(active);
}

void CalibrationTargetManager::setWellIsExcluded(bool excluded, int wellIndex)
{
   assert(wellIndex>=0 && wellIndex<m_wells.size());
   m_wells[wellIndex].setIsExcluded(excluded);
}

void CalibrationTargetManager::renamePropertiesAfterImport()
{
   for (const QString& key : m_userNameToCauldronNameMapping.keys())
   {
      QString cauldronName = m_userNameToCauldronNameMapping[key];
      // User name is the Cauldron name, or "TWT_FROM_DT" or "DT_FROM_VP"
      renameUserPropertyNameInWells(key, cauldronName);

      // Real Cauldron property name, so remove the fake ones
      if (cauldronName == "TWT_FROM_DT") cauldronName="TwoWayTime";
      if (cauldronName == "DT_FROM_VP")  cauldronName="SonicSlowness";
      addToMapping(m_userNameToCauldronNameMapping[key], cauldronName);
   }
}

int CalibrationTargetManager::amountOfActiveCalibrationTargets() const
{
   return activeCalibrationTargets().size();
}

void CalibrationTargetManager::clear()
{  
   m_userNameToCauldronNameMapping.clear();
   m_wells.clear();
}

void CalibrationTargetManager::writeToFile(ScenarioWriter& writer) const
{
   writer.writeValue("CalibrationTargetManagerVersion", 2);
   QVector<int> wellIds;
   for (const Well& well : m_wells)
   {
      well.writeToFile(writer);
      wellIds.push_back(well.id());
   }
   writer.writeValue("wellIndices", wellIds);

   int counter = 0;
   for (const QString& key: m_userNameToCauldronNameMapping.keys())
   {
      writer.writeValue("MappingEntry_" + QString::number(counter), key + "," + m_userNameToCauldronNameMapping.value(key));
      counter++;
   }
}

void CalibrationTargetManager::readFromFile(const ScenarioReader& reader)
{
   const int version = reader.readInt("CalibrationTargetManagerVersion");

   m_wells.clear();

   QVector<int> wellIds = reader.readVector<int>("wellIndices");
   for (const int wellIndex : wellIds)
   {
      Well newWell;
      newWell.readFromFile(reader, wellIndex);
      m_wells.push_back(newWell);
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
}

QVector<const CalibrationTarget*> CalibrationTargetManager::activeCalibrationTargets() const
{
   QVector<const CalibrationTarget*> targets;
   for (const Well* well : activeWells())
   {
      for (const CalibrationTarget* target : well->activePropertyTargets())
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

void CalibrationTargetManager::setPropertyActiveForAllWells(const QString& propertyName)
{
   bool allWellsSelected = true;
   for (auto& well : m_wells)
   {
      if (!well.propertyActiveState().value(propertyName, true)) // Check if the property in this well is active
      {
         allWellsSelected = false;
         break;
      }
   }

   // If the property is active for all wells, set property inactive for all wells.
   // Otherwise, set property active for all wells.
   for (auto& well : m_wells)
   {
      well.setPropertyActive(propertyName, !allWellsSelected);
   }
}

bool CalibrationTargetManager::showPropertiesInTable() const
{
   return m_showPropertiesInTable;
}

void CalibrationTargetManager::setShowPropertiesInTable(const bool showPropertiesInTable)
{
   m_showPropertiesInTable = showPropertiesInTable;
}

void CalibrationTargetManager::disableInvalidWells(const std::string& projectFileName, const std::string& depthGridName)
{
   CMBMapReader mapReader;
   mapReader.load(projectFileName);
   WellValidator validator(mapReader);
   QStringList usedWellNames;
   for (Well& well : m_wells)
   {
      const WellState wellState = validator.wellState(well, depthGridName, usedWellNames);
      usedWellNames.push_back(well.name());

      switch (wellState)
      {
      case WellState::invalidData:
         well.setIsActive(false);
         well.setIsInvalid(true);
         well.setHasActiveProperties(true);
         Logger::log() << "Well " << well.name() << " does not have calibration data and is therefore disabled. Check if the input file is valid." << Logger::endl();
         break;
      case WellState::invalidLocation:
         well.setIsActive(false);
         well.setIsInvalid(true);
         well.setHasActiveProperties(true);
         Logger::log() << "Well " << well.name() << " is outside of the basin model and is therefore disabled." << Logger::endl();
         break;
      case WellState::invalidDuplicateName:
         well.setIsActive(false);
         well.setIsInvalid(true);
         well.setHasActiveProperties(true);
         Logger::log() << "Well " << well.name() << " has a duplicate name and is therefore disabled." << Logger::endl();
         break;
      case WellState::valid:
         break;
      }
   }
}

void CalibrationTargetManager::setWellHasDataInLayer(const std::string& projectFileName, const QStringList& layerNames)
{
   CMBMapReader mapReader;
   mapReader.load(projectFileName);

   for (Well& well : m_wells)
   {
      QVector<bool> hasDataInLayer;

      for (const QString& layer : layerNames)
      {
         bool hasDataInCurrentLayer = false;
         for (const CalibrationTarget* target : well.calibrationTargets())
         {
            if (mapReader.checkIfPointIsInLayer(well.x(), well.y(), target->z(), layer.toStdString()))
            {
               hasDataInCurrentLayer = true;
               break;
            }
         }
         hasDataInLayer.push_back(hasDataInCurrentLayer);
      }

      well.setHasDataInLayer(hasDataInLayer);
   }
}

QVector<QVector<const CalibrationTarget*>> CalibrationTargetManager::extractWellTargets(QStringList& propertyUserNames, const int wellIndex) const
{
   if (wellIndex >= m_wells.size() || wellIndex < 0)
   {
      return {};
   }
   QVector<QVector<const CalibrationTarget*>> targetsInWell;
   propertyUserNames.clear();

   for (const CalibrationTarget* target : m_wells[wellIndex].calibrationTargets())
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
   if (wellIndex >= m_wells.size() || wellIndex < 0)
   {
      return {};
   }

   QStringList propertyUserNames;
   for (const CalibrationTarget* target : m_wells[wellIndex].calibrationTargets())
   {
      const int propertyIndex = propertyUserNames.indexOf(target->propertyUserName());
      if (propertyIndex == -1)
      {
         propertyUserNames.append(target->propertyUserName());
      }
   }

   return propertyUserNames;
}

QStringList CalibrationTargetManager::getWellNames() const
{
   QStringList wellNames;
   for (const Well& well : m_wells)
   {
      wellNames.append(well.name());
   }
   return wellNames;
}

void CalibrationTargetManager::convertVPtoDT()
{
   const VPToDTConverter converter;

   const QString convertedDTName = "DT_FROM_VP";
   const QString velocityUserName = "Velocity";

   for (Well& well : m_wells)
   {
      if ( well.isIncludedInOptimization() )
      {
         if (well.removeCalibrationTargetsWithPropertyUserName(convertedDTName))
         {
            Logger::log() << "Overwriting " << convertedDTName << " data for well: " << well.name() << Logger::endl();
         }

         QStringList properties;
         const QVector<QVector<const CalibrationTarget*>> targetsInWell = extractWellTargets(properties, well.id());

         if (properties.indexOf(velocityUserName) < 0)
         {
            Logger::log() << "Well " << well.name() << " does not have any Velocity data to convert to SonicSlowness." << Logger::endl();
            continue;
         }

         const QVector<const CalibrationTarget*> vpTargets = targetsInWell[properties.indexOf(velocityUserName)];

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

         well.appendMetaData("Created " + convertedDTName + " targets converted from " + velocityUserName + ".");
      }
   }

   addToMapping("DT_FROM_VP", "SonicSlowness");
}

void CalibrationTargetManager::deleteWells(const QVector<int>& wellIDs)
{
   if (wellIDs.empty()) return;
   int iSelect = 0;
   int i = 0;
   while (i < m_wells.size())
   {
      if (iSelect < wellIDs.size() && wellIDs[iSelect] == i + iSelect)
      {
         m_wells.remove(i);
         ++iSelect;
      }
      else
      {
         m_wells[i].setId(i);
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

void CalibrationTargetManager::applyObjectiveFunctionOnCalibrationTargets(const ObjectiveFunctionManager& objectiveFunctionManager)
{
   for (Well& well : m_wells)
   {
      well.applyObjectiveFunction(objectiveFunctionManager);
   }
}

void CalibrationTargetManager::addToMapping(const QString& userName, const QString& cauldronName)
{
   m_userNameToCauldronNameMapping[userName] = cauldronName;
}

void CalibrationTargetManager::addUnits(const QString& userName, const QString& unit)
{
   m_userNameUnits[userName] = unit;
}

QString CalibrationTargetManager::getCauldronPropertyName(const QString& userPropertyName) const
{
   return m_userNameToCauldronNameMapping.value(userPropertyName, "Unknown");
}

QString CalibrationTargetManager::getUnit(const QString& userPropertyName)
{
   return m_userNameUnits[userPropertyName];
}

void CalibrationTargetManager::removeDataOutsideModelDepths(const std::vector<double>& basementDepthsAtActiveWellLocations, const std::vector<double>& mudlineDepthsAtActiveWellLocations)
{
   int counter = 0;
   for (Well& well : m_wells)
   {
      if ( well.isIncludedInOptimization() )
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
   for (const Well& well : m_wells)
   {
      if ( well.isIncludedInOptimization() )
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

   const QString convertedTWTName = "TWT_FROM_DT";
   const QString sonicSlownessUserName = "SonicSlowness";

   for (Well& well : m_wells)
   {
      if ( well.isIncludedInOptimization() )
      {
         if (well.removeCalibrationTargetsWithPropertyUserName(convertedTWTName))
         {
            Logger::log() << "Overwriting " << convertedTWTName << " data for well: " << well.name() << Logger::endl();
         }

         QStringList properties;
         const QVector<QVector<const CalibrationTarget*>> targetsInWell = extractWellTargets(properties, well.id());

         QString sonicSlownessUserNameLocal = sonicSlownessUserName;
         if (properties.indexOf(sonicSlownessUserNameLocal) < 0)
         {
            sonicSlownessUserNameLocal = "DT_FROM_VP";
            if (properties.indexOf(sonicSlownessUserNameLocal) < 0)
            {
               Logger::log() << "Well " << well.name() << " does not have any SonicSlowness data to convert to TwoWayTime." << Logger::endl();
               continue;
            }
         }

         const QVector<const CalibrationTarget*> dtTargets = targetsInWell[properties.indexOf(sonicSlownessUserNameLocal)];
         const DepthPropertyPair modelDataForWell = data.at(well.name().toStdString());

         PropertyVector sonicSlowness;
         DepthVector depth;
         for (const CalibrationTarget* target : dtTargets)
         {
            sonicSlowness.push_back(target->value());
            depth.push_back(target->z());
         }

         const std::vector<double> twt = DTToTwoWayTimeConverter::convertToTWT(sonicSlowness, depth, modelDataForWell.second, modelDataForWell.first);

         for (int i = 0; i < twt.size(); i++)
         {
            const QString targetName(QString("TwoWayTime") + "(" +
                                     QString::number(well.x(),'f',1) + "," +
                                     QString::number(well.y(),'f',1) + "," +
                                     QString::number(depth[i],'f',1) + ")");
            addCalibrationTarget(targetName, convertedTWTName, well.id(), depth[i], twt[i]);
         }

         well.appendMetaData("Created " + convertedTWTName + " targets converted from " + sonicSlownessUserNameLocal + ".");
      }
   }

   addToMapping(convertedTWTName, "TwoWayTime");
}

QVector<int> CalibrationTargetManager::getExcludedWellsFromActiveWells()
{
  QVector<int> excludedWells;
  int counter = 0;
  for (const Well* well : activeWells())
  {
    if (well->isExcluded())
    {
      excludedWells.push_back(counter);
    }
    counter++;
  }

  return excludedWells;
}

} // namespace casaWizard
