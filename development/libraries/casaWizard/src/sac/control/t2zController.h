//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include <QObject>
#include <QVector>

class QString;

namespace casaWizard
{

class ScriptRunController;

namespace sac
{

class SACScenario;
class T2Ztab;

class T2Zcontroller : public QObject
{
  Q_OBJECT

public:
  T2Zcontroller(T2Ztab* t2zTab,
                SACScenario& casaScenario,
                ScriptRunController& scriptRunController,
                QObject* parent);

private slots:
  void slotPushButtonSACrunT2ZClicked();
  void slotComboBoxReferenceSurfaceValueChanged(int referenceSurface);
  void slotSpinBoxSubSamplingValueChanged(int subSampling);
  void slotComboBoxProjectSelectionTextChanged(const QString& projectSelection);
  void slotSpinBoxNumberOfCPUsValueChanged(int numberOfCPUs);
  void slotUpdateTabGUI(int tabID);
  void slotComboBoxClusterSelectionTextChanged(const QString& clusterName);
  void slotExportT2ZScenarioClicked();  
  void slotOutputFile();

private:
  bool noProjectAvailable() const;
  bool userCancelsRun() const;
  bool prepareT2ZWorkSpace();
  void getSourceAndT2zDir();
  void setSubSampling();
  void runDepthConversion();
  void updateSurfaces();
  void updateProjectSelectionOptions();

  T2Ztab* t2zTab_;
  SACScenario& casaScenario_;
  ScriptRunController& scriptRunController_;
  QString t2zDir_;
  QString sourceDir_;
  int lineNr_;
};

} // namespace sac

} // namespace casaWizard
