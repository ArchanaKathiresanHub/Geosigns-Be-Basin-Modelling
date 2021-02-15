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
class LithofractionMap;
class MapReader;

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
  void slotThreadsValueChanged(int threads);
  void slotUpdateBirdView();
  void slotUpdateTabGUI(int tabID);
  void slotUpdateWell(const QString& name);

  void exportOptimized();
  void runOptimized();
  void runOriginal();

signals:
  void signalRefreshChildWidgets();

private:
  void import3dWellData(const QString baseDirectory, const bool isOptimized);
  void refreshGUI();
  bool run3dCase(const QString directory);

  QVector<int> getExcludedWells();
  QVector<int> getSelectedWellIndices();
  QVector<int> selectedWells();
  QVector<int> transformToActiveAndIncluded(const QVector<int>& wellIndices, const QVector<int>& excludedWells);

  MapsTab* mapsTab_;
  SACScenario& scenario_;
  ScriptRunController& scriptRunController_;
  ActiveWellsController* activeWellsController_;
  LithofractionVisualisationController* lithofractionVisualisationController_;

  int activeWell_;
};

} // namespace sac

} // namespace casaWizard
