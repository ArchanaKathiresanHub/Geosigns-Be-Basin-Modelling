//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include <QObject>

class QString;

namespace casaWizard
{

class ScriptRunController;

namespace sac
{

class SACScenario;
class ResultsTab;

class ResultsController : public QObject
{
  Q_OBJECT

public:
  explicit ResultsController(ResultsTab* resultsTab,
                             SACScenario& scenario,
                             ScriptRunController& scriptRunController,
                             QObject* parent);
  void updateTab();

private slots:
  void updateWell(const QString& name);
  void refreshPlot();
  void togglePlotType(const int currentIndex);
  void updateProperty(const QString property);
  void updateWellFromBirdView(const int lineIndex, const int pointIndex);
  void selectedWellFromScatter(const int wellIndex);
  void slotUpdateTabGUI(int tabID);

private:
  void refreshGUI();
  void updateOptimizedTable();
  void updateWellPlot();
  void updateScatterPlot();
  void updateBirdView();
  QVector<int> selectedWells();

  ResultsTab* resultsTab_;
  SACScenario& scenario_;
  ScriptRunController& scriptRunController_;
  int activeWell_;
  QString activeProperty_;

  bool noValidWellSelected();
  void setActivePlots();
  void setActiveWells();
  void setDomainBirdsView();
  void setDefaultWellSelection();
};

} // namespace sac

} // namespace casaWizard
