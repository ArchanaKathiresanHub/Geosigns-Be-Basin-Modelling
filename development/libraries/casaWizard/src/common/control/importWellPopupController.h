//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include "model/calibrationTargetManager.h"
#include "model/input/importOptions.h"

#include <QMessageBox>
#include <QObject>
#include <QSet>

#include <memory>

class QTableWidgetItem;

namespace casaWizard
{

class CalibrationTargetCreator;
class CasaScenario;
class ExtractWellData;
class ImportWellPopup;

class ImportWellPopupController : public QObject
{
  Q_OBJECT
public:
  explicit ImportWellPopupController(QObject* parent, CasaScenario& casaScenario);
  CalibrationTargetManager& importCalibrationTargetManager();

  void importWells(const QString& fileName);

  virtual ImportWellPopup* importWellPopup() const = 0;
  virtual bool importWellsToCalibrationTargetManager(const QString& fileName) = 0;

protected:
  CalibrationTargetManager importCalibrationTargetManager_;
  CasaScenario& casaScenario_;

  void importOnSeparateThread(CalibrationTargetCreator& calibrationTargetCreator);
  void addNewMapping();

private slots:
  void slotCloseWaitingDialog();
private:
  QMessageBox waitingDialog_;
  bool waitingDialogNeeded_;
};

} // namespace casaWizard
