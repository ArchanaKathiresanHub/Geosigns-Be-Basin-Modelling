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

#include "model/wellTrajectoryManager.h"

class QString;

namespace casaWizard
{

class CalibrationTarget;
class CalibrationTargetManager;
class ScriptRunController;

namespace sac
{

class SacLithologyScenario;
class ResultsTab;

class ResultsController : public QObject
{
  Q_OBJECT

public:
  explicit ResultsController(ResultsTab* resultsTab,
                             SacLithologyScenario& scenario,
                             ScriptRunController& scriptRunController,
                             QObject* parent);
  void updateTab();

private slots:
  void slotUpdateWell();
  void slotActiveChanged();
  void slotTogglePlotType(const int currentIndex);
  void slotUpdateProperty(QString property);
  void slotUpdateWellFromBirdView(const int lineIndex, const int pointIndex);
  void slotSelectedWellFromCorrelation(const int wellIndex);
  void slotUpdateTabGUI(int tabID);
  void slotWellPrepOrSAC(int buttonId);

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
  SacLithologyScenario& scenario_;
  ScriptRunController& scriptRunController_;
  QVector<int> selectedWellsIDs_;
  QString activeProperty_;
  bool wellPrepWells_;

  void initializeWellSelection();
  void setGuiOptionsInPlots();
  void setActiveWells();
  void setActivePropertiesToWells();
  void setDomainBirdsView();
  void setDefaultWellSelection();
  QMap<QString, double> getSurfaceValues();
  QStringList getUnitsForProperties(const QVector<QVector<const CalibrationTarget*>> targets);
  const CalibrationTargetManager& calibrationTargetManager() const;
  QVector<QVector<WellTrajectory>> getAllTrajectories(const QStringList& propertyUserNames) const;
};

} // namespace sac

} // namespace casaWizard
