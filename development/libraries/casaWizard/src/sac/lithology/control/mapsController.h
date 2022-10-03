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
class MapsManager;
}

namespace sac
{

class ActiveWellsController;
class LithofractionVisualisationController;
class MapsTabLithology;

class SacLithologyScenario;

class MapsController : public QObject
{
  Q_OBJECT

public:
  explicit MapsController(MapsTabLithology* resultsTab,
                          SacLithologyScenario& scenario,
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
  void slotExportOptimizedToZycor();
  void slotRunOptimized();  

signals:
  void signalRefreshChildWidgets();

private:  
  QVector<int> getExcludedWellsFromActiveWells();
  QVector<int> getSelectedWellIndices();

  void refreshGUI();
  void validateWellsHaveOptimized();

  MapsManager& mapsManager_;
  MapsTabLithology* mapsTab_;
  SacLithologyScenario& scenario_;
  ScriptRunController& scriptRunController_;
  ActiveWellsController* activeWellsController_;
  LithofractionVisualisationController* lithofractionVisualisationController_;
};

} // namespace sac

} // namespace casaWizard
