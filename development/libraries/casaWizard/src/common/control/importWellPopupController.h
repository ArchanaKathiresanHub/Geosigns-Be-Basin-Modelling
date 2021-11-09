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
#include <QMessageBox>

#include <memory>

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

  virtual ImportWellPopup* importWellPopup() const = 0;
  virtual void importWellsToCalibrationTargetManager(const QString& fileName, CalibrationTargetManager& calibrationTargetManager) = 0;

protected:  
  CasaScenario& casaScenario_;

  void importOnSeparateThread(CalibrationTargetCreator& calibrationTargetCreator);  

private slots:
  void slotCloseWaitingDialog();
  void slotExceptionThrown(QString message);
private:
  QMessageBox waitingDialog_;
  bool waitingDialogNeeded_;
};

} // namespace casaWizard
