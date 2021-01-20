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
  void updateWell(const QString& name);
  void slotUpdateTabGUI(int tabID);
  void slotPvalueChanged(int pIDW);
  void slotInterpolationTypeCurrentIndexChanged(int interpolationType);
  void slotSmoothingTypeCurrentIndexChanged(int smoothingType);
  void slotSmoothingRadiusValueChanged(int smoothingRadius);
  void slotThreadsValueChanged(int threads);
  void slotGenerateLithoMaps();

signals:
  void signalRefreshChildWidgets();

private:
  void refreshGUI();

  QVector<int> selectedWells();

  MapsTab* mapsTab_;
  SACScenario& scenario_;
  ScriptRunController& scriptRunController_;
  ActiveWellsController* activeWellsController_;
  LithofractionVisualisationController* lithofractionVisualisationController_;

  int activeWell_;
};

} // namespace sac

} // namespace casaWizard