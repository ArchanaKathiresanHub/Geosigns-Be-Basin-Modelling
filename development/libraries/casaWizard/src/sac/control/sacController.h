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
class CalibrationTargetManager;
class ScriptRunController;
class ObjectiveFunctionControllerSAC;

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
  void slotPushButtonSACrunCasaClicked();

  void slotComboBoxClusterCurrentTextChanged(QString clusterName);
  void slotComboBoxApplicationChanged(QString application);

  void slotExtractData();
  void slotUpdateTabGUI(int tabID);

  void slotRunOriginal1D();
  void slotRunOriginal3D();  

signals:
  void signalRefreshChildWidgets();

private:
  void refreshGUI();  
  void renameCaseFolders(const QString& pathName);

  SACtab* sacTab_;
  SACScenario& casaScenario_;
  ScriptRunController& scriptRunController_;

  CalibrationTargetController* calibrationTargetController_;
  DataExtractionController* dataExtractionController_;
  LithofractionController* lithofractionController_;
  ObjectiveFunctionControllerSAC* objectiveFunctionController_;  
};

} // namespace sac

} // namespace casaWizard
