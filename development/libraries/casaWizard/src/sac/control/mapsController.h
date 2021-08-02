//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include <QObject>

namespace casaWizard
{

class ScriptRunController;

namespace sac
{

class ActiveWellsController;
class LithofractionVisualisationController;
class SACScenario;
class MapsTab;

class MapsController : public QObject
{
  Q_OBJECT

public:
  explicit MapsController(MapsTab* resultsTab,
                          SACScenario& scenario,
                          ScriptRunController& scriptRunController,
                          QObject* parent);
  void updateTab();

private slots:
  void slotGenerateLithoMaps();
  void slotInterpolationTypeCurrentIndexChanged(int interpolationType);
  void slotPvalueChanged(int pIDW);
  void slotSmoothingTypeCurrentIndexChanged(int smoothingType);
  void slotSmoothingRadiusValueChanged(int smoothingRadius);  
  void slotUpdateBirdView();
  void slotUpdateTabGUI(int tabID);
  void slotUpdateWellSelection();
  void slotSmartGriddingChanged(int state);
  void slotWellClicked(const QString& wellName);

  void slotExportOptimized();
  void slotRunOptimized();  

signals:
  void signalRefreshChildWidgets();

private:  
  QVector<int> getExcludedWells();
  QVector<int> getSelectedWellIndices();
  QVector<int> selectedWells();
  QVector<int> transformToActiveAndIncluded(const QVector<int>& wellIndices, const QVector<int>& excludedWells);
  bool hasOptimizedSuccessfully(const int index) const;

  void refreshGUI();
  void validateWellsHaveOptimized();

  MapsTab* mapsTab_;
  SACScenario& scenario_;
  ScriptRunController& scriptRunController_;
  ActiveWellsController* activeWellsController_;
  LithofractionVisualisationController* lithofractionVisualisationController_;

};

} // namespace sac

} // namespace casaWizard
