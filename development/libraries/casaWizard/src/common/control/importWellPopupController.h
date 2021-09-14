//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include "model/calibrationTargetManager.h"

#include <QObject>
#include <QSet>

class QTableWidgetItem;

namespace casaWizard
{

class CasaScenario;
class ImportWellPopup;

class ImportWellPopupController : QObject
{
  Q_OBJECT
public:
  ImportWellPopupController(ImportWellPopup* importwellPopup, QObject* parent);
  int executeImportWellPopup();
  CalibrationTargetManager& importCalibrationTargetManager();

private slots:
  void slotAcceptedClicked();

private:
  bool mappingContainsUnknowns(const QMap<QString, QString>& mapping) const;
  QVector<QString> overwrittenMappingKeys(const QMap<QString, QString>& newMapping) const;
  QString getRenamedUserPropertyName(const int rowNumber);
  void renameUserPropertyNames();

  CalibrationTargetManager importCalibrationTargetManager_;
  ImportWellPopup* importWellPopup_;
  QSet<QString> targetVariableUserNames_;
};

} // namespace casaWizard
