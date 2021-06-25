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

class WellTrajectory;
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
  void updateWell();
  void activeChanged();
  void togglePlotType(const int currentIndex);
  void updateProperty(const QString property);
  void updateWellFromBirdView(const int lineIndex, const int pointIndex);
  void selectedWellFromCorrelation(const int wellIndex);
  void slotUpdateTabGUI(int tabID);

  void slotUpdateSurfaceLines(const bool showSurfaceLines);
  void slotUpdateFitRangeToData(const bool fitRangeToData);
  void slotUpdateIsExpanded(int state, int plotID);
private:
  void refreshGUI();
  void refreshPlot();
  void updateOptimizedTable();
  void updateWellPlot();
  void updateCorrelationPlot();
  void updateBirdView();
  QVector<int> selectedWellsIDs();

  ResultsTab* resultsTab_;
  SACScenario& scenario_;
  ScriptRunController& scriptRunController_;
  QVector<int> selectedWellsIDs_;
  QString activeProperty_;

  void initializeWellSelection();
  void setActivePlots();
  void setActiveWells();
  void setDomainBirdsView();
  void setDefaultWellSelection();
  QMap<QString, double> getSurfaceValues();
  QStringList getUnitsForProperties(const QVector<QVector<WellTrajectory>>& allTrajectories);
};

} // namespace sac

} // namespace casaWizard
