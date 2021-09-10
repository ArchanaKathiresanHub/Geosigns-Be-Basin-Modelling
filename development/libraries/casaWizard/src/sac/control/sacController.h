//
// Copyright (C) 2020 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include <QObject>
#include <QVector>
#include <QMessageBox>

class QString;
class QTableWidgetItem;


namespace casaWizard
{

class CalibrationTargetController;
class CalibrationTargetManager;
class ScriptRunController;
class ObjectiveFunctionControllerSAC;
class ImportWellPopupController;

namespace sac
{

class CaseExtractor;
class DataExtractionController;
class SACScenario;
class SACtab;
class LithofractionController;

class SACcontroller : public QObject
{
  Q_OBJECT

public:
  SACcontroller(SACtab* sacTab,
                SACScenario& casaScenario,
                ScriptRunController& scriptRunController,
                QObject* parent);

private slots:
  void slotPushSelectProject3dClicked();
  void slotPushSelectCalibrationClicked();
  void slotPushButtonSACrunCasaClicked();

  void slotComboBoxClusterCurrentTextChanged(QString clusterName);
  void slotComboBoxApplicationChanged(QString application);

  void slotExtractData();
  void slotUpdateTabGUI(int tabID);
  void slotPushSelectAllWellsClicked();
  void slotPushClearSelectionClicked();

  void slotRunOriginal1D();
  void slotRunOriginal3D();

  void slotCloseWaitingDialog();

signals:
  void signalRefreshChildWidgets();

private:
  void refreshGUI();
  void importOnSeparateThread(casaWizard::CalibrationTargetManager& temporaryImportCalibrationTargetManager, const QString& fileName);

  SACtab* sacTab_;
  SACScenario& casaScenario_;
  ScriptRunController& scriptRunController_;
  QMessageBox importingPopup_;

  CalibrationTargetController* calibrationTargetController_;
  DataExtractionController* dataExtractionController_;
  LithofractionController* lithofractionController_;
  ObjectiveFunctionControllerSAC* objectiveFunctionController_;
  ImportWellPopupController* importWellPopupController_;
};

} // namespace sac

} // namespace casaWizard
