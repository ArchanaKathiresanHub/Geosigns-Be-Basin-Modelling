//
// Copyright (C) 2012-2022 Shell International Exploration & Production.
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

namespace lithology
{

class LithologyResultsTab;

class LithologyResultsController : public QObject
{
  Q_OBJECT

public:
  explicit LithologyResultsController(LithologyResultsTab* resultsTab,
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

  LithologyResultsTab* m_resultsTab;
  SacLithologyScenario& m_scenario;
  ScriptRunController& m_scriptRunController;
  QVector<int> m_selectedWellsIDs;
  QString m_activeProperty;
  bool m_wellPrepWells;

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

} // namespace lithology

} // namespace sac

} // namespace casaWizard
