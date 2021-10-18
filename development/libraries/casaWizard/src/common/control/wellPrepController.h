//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include <QObject>
#include <QMessageBox>

namespace casaWizard
{

class CalibrationTargetManager;
class CalibrationTargetWellPrepController;
class CasaScenario;
class ImportWellPopupController;
class ScriptRunController;
class UserPropertyChoicePopup;
class UserPropertyChoiceCutOffPopup;
class WellPrepTab;

class WellPrepController : public QObject
{
  Q_OBJECT

public:
  explicit WellPrepController(WellPrepTab* wellPrepTab,
                              CasaScenario& scenario,
                              ScriptRunController& scriptRunController,
                              QObject* parent);

protected:
  virtual bool checkTabID(int tabID) const = 0;
  void refreshGUI();

signals:
  void signalRefreshChildWidgets();

public slots:
  void slotUpdateTabGUI(int tabID);

private slots:
  void slotPushSaveDataClicked();
  void slotCloseWaitingDialog();
  void slotConvertDTtoTWT();
  void slotConvertVPToDT();

  void slotPushSelectCalibrationClicked();
  void slotRemoveDataBelowBasementAndAboveMudline();
  void slotRemoveWellsOutOfBasinOutline();
  void slotWellSelectionChanged();
  void slotSelectPropertiesForSmoothing();
  void slotApplySmoothing();
  void slotSelectPropertiesForSubsampling();
  void slotApplySubsampling();
  void slotSelectPropertiesForCutOff();
  void slotApplyCutOff();

private:
  void importOnSeparateThread(CalibrationTargetManager& calibrationTargetManager, const QString& fileName);
  void exportOnSeparateThread(const CalibrationTargetManager& calibrationTargetManager, const QString& fileName);
  bool allActiveWellsHave1DResults() const;
  void checkEnabledStateButtons() const;

  WellPrepTab* wellPrepTab_;
  CasaScenario& casaScenario_;
  ScriptRunController& scriptRunController_;
  CalibrationTargetWellPrepController* calibrationTargetController_;
  ImportWellPopupController* importWellPopupController_;
  UserPropertyChoicePopup* userPropertyChoicePopup_;
  UserPropertyChoiceCutOffPopup* userPropertyChoiceCutOffPopup_;
  QMessageBox waitingDialog_;
  bool waitingDialogNeeded_;
};

} // namespace casaWizard


