//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// Data container for information about a well
#pragma once

#include "calibrationTarget.h"
#include "writable.h"

#include <QStringList>
#include <QVector>

class QString;
class QStringList;

namespace casaWizard
{

class ObjectiveFunctionManager;

class Well : public Writable
{
public:
  Well() = default;
  explicit Well(const int id, const QString& name, const double x, const double y, const bool isActive = true, const bool isExcluded = false,
                const QVector<CalibrationTarget> calibrationTargets = {});

  int version() const;
  void writeToFile(ScenarioWriter& writer) const override;
  void readFromFile(const ScenarioReader& reader) override;
  void readFromFile(const ScenarioReader& reader, const int wellIndex);
  void clear() override;

  QVector<const CalibrationTarget*> calibrationTargets() const;
  QVector<const CalibrationTarget*> calibrationTargetsWithPropertyUserName(const QString& propertyUserName) const;
  QString name() const;
  double x() const;
  double y() const;

  int id() const;
  void setId(const int id);

  bool isActive() const;
  void setIsActive(const bool isActive);

  bool isExcluded() const;
  void setIsExcluded(const bool isExcluded);

  bool isInvalid() const;
  void setIsInvalid(const bool isInvalid);

  QVector<bool> hasDataInLayer() const;
  void setHasDataInLayer(QVector<bool> hasDataInLayer);

  void addCalibrationTarget(const CalibrationTarget& newTarget);
  void addCalibrationTarget(const QString& name, const QString& propertyUserName, const double z, const double value);
  void applyObjectiveFunction(const ObjectiveFunctionManager& objectiveFunction);
  void shift(const double xShift);
  void renameUserPropertyName(const QString& oldName, const QString& newName);
  bool removeCalibrationTargetsWithPropertyUserName(const QString& propertyUserName);
  bool removeDataBelowDepth(const double depth);
  bool removeDataAboveDepth(const double depth);

  QString metaData() const;
  void setMetaData(const QString& metaData);
  void appendMetaData(const QString& metaData);

private:
  int id_;
  QString name_;
  double x_;
  double y_;
  bool isActive_;
  bool isExcluded_;
  bool isInvalid_;
  QVector<CalibrationTarget> calibrationTargets_;
  QVector<bool> hasDataInLayer_;
  QString metaData_;
};


} // namespace casaWizard
