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

class SacScenario;
class SacResultsTab;

class SacResultsController : public QObject
{
  Q_OBJECT

public:
  explicit SacResultsController(SacResultsTab* resultsTab,
                                ScriptRunController& scriptRunController,
                                QObject* parent);
  void updateTab();

  virtual SacScenario& scenario() = 0;
  virtual SacScenario& scenario() const = 0;

protected slots:
  virtual void slotTogglePlotType(const int currentIndex);

protected:

  void refreshGUI();
  void refreshPlot();
  void updateWellPlot();
  void updateCorrelationPlot();
  void updateBirdView();
  virtual void updateOptimizedTable() = 0;

  QVector<int> RefreshSelectedWellsIDs();
  const CalibrationTargetManager& calibrationTargetManager() const;
  void initializeWellSelection();
  bool wellPrepWells() const;
  void setWellPrepWells(bool state);

  virtual SacResultsTab* resultsTab() = 0;
  virtual const SacResultsTab* resultsTab() const = 0;

  QVector<int>& selectedWellsIDs();

private slots:
  void slotUpdateWell();
  void slotActiveChanged();
  void slotUpdateProperty(QString property);
  void slotUpdateWellFromBirdView(const int lineIndex, const int pointIndex);
  void slotSelectedWellFromCorrelation(const int wellIndex);
  void slotUpdateSurfaceLines(const bool showSurfaceLines);
  void slotUpdateFitRangeToData(const bool fitRangeToData);
  void slotUpdateIsExpanded(int state, int plotID);
  virtual void slotUpdateTabGUI(int tabID) = 0;

private:
  ScriptRunController& m_scriptRunController;
  QString m_activeProperty;
  bool m_wellPrepWells;
  QVector<int> m_selectedWellsIDs;

  void setGuiOptionsInPlots();
  void setActiveWells();
  void setActivePropertiesToWells();
  void setDomainBirdsView();
  void setDefaultWellSelection();
  QMap<QString, double> getSurfaceValues();
  QStringList getUnitsForProperties(const QVector<QVector<const CalibrationTarget*>> targets);

  QVector<QVector<WellTrajectory>> getAllTrajectories(const QStringList& propertyUserNames) const;
};

} // namespace sac

} // namespace casaWizard
