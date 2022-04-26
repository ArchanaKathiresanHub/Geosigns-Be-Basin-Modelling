// Object responsible for the calibration targets
#pragma once

#include "calibrationTarget.h"
#include "objectiveFunctionManager.h"
#include "well.h"
#include "writable.h"

#include <QMap>
#include <QVector>

#include <memory>

namespace casaWizard
{

class CalibrationTargetManager : public Writable
{
public:
   CalibrationTargetManager();

   QVector<const CalibrationTarget*> calibrationTargets() const;
   void addCalibrationTarget(const QString& name, const QString& propertyUserName,
                             const int wellIndex, const double z, const double value,
                             double standardDeviation = 0.0, double uaWeight = 1.0);
   void setCalibrationTargetStandardDeviation(int index, double value);
   void setCalibrationTargetUAWeight(int index, double value);

   void appendFrom(const CalibrationTargetManager& calibrationTargetManager);
   void copyMappingFrom(const CalibrationTargetManager& calibrationTargetManager);
   void renameUserPropertyNameInWells(const QString& oldName, const QString& newName);
   QMap<QString, QString> userNameToCauldronNameMapping() const;

   const QVector<const Well*> wells() const;
   const QVector<const Well*> activeWells() const;
   const QVector<const Well*> activeAndIncludedWells() const;
   const Well& well(const int wellIndex) const;
   void setWellIsActive(bool active, int wellIndex);
   void setWellIsExcluded(bool excluded, int wellIndex);
   void renamePropertiesAfterImport();

   int addWell(const QString& wellName, double x, double y);

   int amountOfActiveCalibrationTargets() const;
   QVector<QVector<const CalibrationTarget*>> extractWellTargets(QStringList& propertyUserNames, const QVector<int> wellIndices) const;
   QVector<QVector<const CalibrationTarget*>> extractWellTargets(QStringList& propertyUserNames, const int wellIndex) const;

   void applyObjectiveFunctionOnCalibrationTargets(const ObjectiveFunctionManager& objectiveFunctionManager);

   void addToMapping(const QString& userName, const QString& cauldronName);
   QString getCauldronPropertyName(const QString& userPropertyName) const;
   QString getUnit(const QString& userPropertyName);

   void removeDataOutsideModelDepths(const std::vector<double>& basementDepthsAtActiveWellLocations, const std::vector<double>& mudlineDepthsAtActiveWellLocations);
   void removeWellsOutsideBasinOutline(const std::string& projectFileName, const std::string& depthGridName);
   void clear() override;
   void writeToFile(ScenarioWriter& writer) const override;
   void readFromFile(const ScenarioReader& reader) override;

   QVector<const CalibrationTarget*> activeCalibrationTargets() const;
   QStringList activePropertyUserNames() const;
   void disableInvalidWells(const std::string& projectFileName, const std::string& depthGridName);
   void setWellHasDataInLayer(const std::string& projectFileName, const QStringList& layerNames);

   QStringList getPropertyUserNamesForWell(const int wellIndex) const;
   QStringList getWellNames() const;

   void convertDTtoTWT(const std::string& iterationFolder, const std::string& project3dFilename);
   void convertVPtoDT();

   void deleteWells(const QVector<int>& wellIDs);

   void setWellMetaData(const int wellIndex, const QString& metaData);

   void scaleData(const QStringList& selectedProperties, const double scalingFactor);
   void smoothenData(const QStringList& selectedProperties, const double radius);
   void subsampleData(const QStringList& selectedProperties, const double length);
   void applyCutOff(const QMap<QString,QPair<double,double>>& propertiesWithCutOffRanges);
   void setWellActiveProperty(const QString& property, const bool active, const int wellIndex);

   QMap<QString, QString> userNameUnits() const;

   void addUnits(const QString& userName, const QString& unit);
   void setPropertyActiveForAllWells(const QString& propertyName);

   bool showPropertiesInTable() const;
   void setShowPropertiesInTable(const bool showPropertiesInTable);

   QMap<QString, QSet<int> > getPropertyNamesPerWellForTargetTable() const;
private:
   CalibrationTargetManager(const CalibrationTargetManager&) = delete;
   CalibrationTargetManager& operator=(CalibrationTargetManager) = delete;

   QMap<QString, QSet<int> > getPropertyNamesPerWell() const;
   int addWell(Well well);
   double getShiftToAvoidOverlap(const QString& wellName, const double x, const double y);

   QMap<QString, QString> m_userNameToCauldronNameMapping;
   QMap<QString, QString> m_userNameUnits;
   QVector<Well> m_wells;
   bool m_showPropertiesInTable;
};

} // namespace casaWizard
