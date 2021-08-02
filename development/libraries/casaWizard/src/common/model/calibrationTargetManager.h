// Object responsible for the calibration targets
#pragma once

#include "calibrationTarget.h"
#include "objectiveFunctionManager.h"
#include "well.h"
#include "writable.h"

#include <QMap>
#include <QVector>

namespace casaWizard
{

class CalibrationTargetManager : public Writable
{
public:
  CalibrationTargetManager();

  QVector<const CalibrationTarget*> calibrationTargets() const;
  void addCalibrationTarget(const QString& name, const QString& propertyUserName,
                            const int wellIndex, const double z, const double value);
  void setCalibrationTargetStandardDeviation(int index, double value);
  void setCalibrationTargetUAWeight(int index, double value);

  const QVector<const Well*> wells() const;
  const QVector<const Well*> activeWells() const;
  const QVector<const Well*> activeAndIncludedWells() const;
  const Well& well(const int wellIndex) const;
  void setWellIsActive(bool active, int wellIndex);
  void setWellIsExcluded(bool excluded, int wellIndex);
  int addWell(const QString& wellName, double x, double y);
  void setHasDataInLayer(const int wellIndex, QVector<bool> hasDataInLayer);

  int amountOfActiveCalibrationTargets() const;
  QVector<QVector<const CalibrationTarget*>> extractWellTargets(QStringList& propertyUserNames, const QVector<int> wellIndices) const;
  QVector<QVector<const CalibrationTarget*>> extractWellTargets(QStringList& propertyUserNames, const int wellIndex) const;

  const ObjectiveFunctionManager& objectiveFunctionManager() const;

  void updateObjectiveFunctionFromTargets();
  void applyObjectiveFunctionOnCalibrationTargets();
  void setObjectiveFunction(int row, int col, double value);
  void setObjectiveFunctionVariables(const QStringList& variables);
  void setObjectiveFunctionEnabledState(const bool state, const int row);
  bool propertyIsActive(const QString& property) const;

  void addToMapping(const QString& userName, const QString& cauldronName);
  QString getCauldronPropertyName(const QString& userPropertyName) const;

  void clear() override;
  void writeToFile(ScenarioWriter& writer) const override;
  void readFromFile(const ScenarioReader& reader) override;

  QVector<const CalibrationTarget*> activeCalibrationTargets() const;
  QStringList activePropertyUserNames() const;
  void disableInvalidWells(const std::string& projectFileName, const std::string& depthGridName);

  QStringList getPropertyUserNamesForWell(const int wellIndex) const;
private:
  CalibrationTargetManager(const CalibrationTargetManager&) = delete;
  CalibrationTargetManager& operator=(CalibrationTargetManager) = delete;

  ObjectiveFunctionManager objectiveFunctionManager_;
  QMap<QString, QString> userNameToCauldronNameMapping_;
  QVector<Well> wells_;

};

} // namespace casaWizard
