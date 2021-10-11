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
class CasaScenario;

class ObjectiveFunctionController : public QObject
{
  Q_OBJECT

public:
  ObjectiveFunctionController(ObjectiveFunctionTable* objFunTable,
                              CasaScenario& casaScenario,
                              QObject* parent);  

private slots:
  void slotTableObjectiveFunctionChanged(QTableWidgetItem* item);

  void slotRefresh();

private:
  virtual int offsetColumnToObjectiveFunctionManagerValue() const;

  ObjectiveFunctionTable* objectiveFunctionTable_;
  CasaScenario& casaScenario_;
};

}  // namespace casaWizard
