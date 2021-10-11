//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "objectiveFunctionController.h"

#include "model/casaScenario.h"
#include "model/calibrationTargetManager.h"
#include "model/objectiveFunctionManager.h"
#include "view/objectiveFunctionTable.h"

#include <QTableWidgetItem>

namespace casaWizard
{

ObjectiveFunctionController::ObjectiveFunctionController(ObjectiveFunctionTable* objectiveFunctionTable,
                                                         CasaScenario& casaScenario,
                                                         QObject* parent) :
  QObject{parent},
  objectiveFunctionTable_{objectiveFunctionTable},
  casaScenario_{casaScenario}
{
  connect(objectiveFunctionTable_, SIGNAL(itemChanged(QTableWidgetItem*)),
          this,                    SLOT(slotTableObjectiveFunctionChanged(QTableWidgetItem*)));

  connect(parent, SIGNAL(signalRefreshChildWidgets()), this, SLOT(slotRefresh()));
}

void ObjectiveFunctionController::slotRefresh()
{
  objectiveFunctionTable_->updateTable(casaScenario_.objectiveFunctionManager());
}

void ObjectiveFunctionController::slotTableObjectiveFunctionChanged(QTableWidgetItem* item)
{
  casaScenario_.objectiveFunctionManager().setValue(item->row(), item->column() - offsetColumnToObjectiveFunctionManagerValue(), item->data(0).toDouble());
  casaScenario_.applyObjectiveFunctionOnCalibrationTargets();
}

int ObjectiveFunctionController::offsetColumnToObjectiveFunctionManagerValue() const
{
  return 0;
}

}  // namespace casaWizard
