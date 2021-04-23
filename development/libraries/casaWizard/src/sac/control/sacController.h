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

class QString;
class QTableWidgetItem;


namespace casaWizard
{

class CalibrationTargetController;
class ScriptRunController;
class ObjectiveFunctionController;


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
signals:
  void signalRefreshChildWidgets();

private:
  void refreshGUI();

  SACtab* sacTab_;
  SACScenario& casaScenario_;
  ScriptRunController& scriptRunController_;

  CalibrationTargetController* calibrationTargetController_;
  DataExtractionController* dataExtractionController_;
  LithofractionController* lithofractionController_;
  ObjectiveFunctionController* objectiveFunctionController_;
};

} // namespace sac

} // namespace casaWizard
