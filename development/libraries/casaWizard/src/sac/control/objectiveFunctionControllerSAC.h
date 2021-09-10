//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// Controller to list the objective functions
#pragma once

#include <QObject>

class QTableWidgetItem;

namespace casaWizard
{

class ObjectiveFunctionTableSAC;
class CalibrationTargetManager;
class CasaScenario;

class ObjectiveFunctionControllerSAC : public QObject
{
  Q_OBJECT

public:
  ObjectiveFunctionControllerSAC(ObjectiveFunctionTableSAC* objectiveFunctionTableSAC, CalibrationTargetManager& calibrationTargetManager,
                                 casaWizard::CasaScenario& scenario, QObject* parent);
private slots:
  void slotTableObjectiveFunctionChanged(QTableWidgetItem* item);
  void slotEnabledStateChanged(int state, int row);
  void slotRefresh();

private:
  ObjectiveFunctionTableSAC* objectiveFunctionTableSAC_;
  CalibrationTargetManager& calibrationTargetManager_;
  casaWizard::CasaScenario& scenario_;
};

}  // namespace casaWizard
