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

class ObjectiveFunctionTable;
class CalibrationTargetManager;

class ObjectiveFunctionController : public QObject
{
  Q_OBJECT

public:
  ObjectiveFunctionController(ObjectiveFunctionTable* objFunTable,
                              CalibrationTargetManager& casaScenario,
                              QObject* parent);

private slots:
  void slotTableObjectiveFunctionChanged(QTableWidgetItem* item);

  void slotRefresh();

private:
  ObjectiveFunctionTable* objectiveFunctionTable_;
  CalibrationTargetManager& calibrationTargetManager_;
};

}  // namespace casaWizard
