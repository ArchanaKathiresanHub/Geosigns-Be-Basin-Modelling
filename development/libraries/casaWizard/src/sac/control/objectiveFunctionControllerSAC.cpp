//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "objectiveFunctionControllerSAC.h"

#include "model/calibrationTargetManager.h"
#include "model/casaScenario.h"
#include "view/objectiveFunctionTableSAC.h"

#include <QTableWidgetItem>

namespace casaWizard
{

ObjectiveFunctionControllerSAC::ObjectiveFunctionControllerSAC(ObjectiveFunctionTableSAC* objectiveFunctionTableSAC,
                                                         CalibrationTargetManager& calibrationTargetManager, casaWizard::CasaScenario& scenario,
                                                         QObject* parent) :
  QObject{parent},
  objectiveFunctionTableSAC_{objectiveFunctionTableSAC},
  calibrationTargetManager_{calibrationTargetManager},
  scenario_{scenario}
{
  connect(objectiveFunctionTableSAC_, SIGNAL(itemChanged(QTableWidgetItem*)),
          this,                    SLOT(slotTableObjectiveFunctionChanged(QTableWidgetItem*)));
  connect(objectiveFunctionTableSAC_, SIGNAL(enabledStateChanged(int, int)),
          this,                    SLOT(slotEnabledStateChanged(int, int)));
  connect(objectiveFunctionTableSAC_, SIGNAL(selectedPropertyChanged(const QString&, const QString&)),
          this,                    SLOT(slotSelectedPropertyChanged(const QString&, const QString&)));
  connect(parent, SIGNAL(signalRefreshChildWidgets()), this, SLOT(slotRefresh()));
}

void ObjectiveFunctionControllerSAC::slotRefresh()
{
  objectiveFunctionTableSAC_->updateTable(calibrationTargetManager_.objectiveFunctionManager());
}

void ObjectiveFunctionControllerSAC::slotTableObjectiveFunctionChanged(QTableWidgetItem* item)
{
  // We set the objective function at item->column - 1, since the SAC objective function table
  // has the extra column for enabling properties.
  calibrationTargetManager_.setObjectiveFunction(item->row(), item->column() - 1, item->data(0).toDouble());
}

void ObjectiveFunctionControllerSAC::slotEnabledStateChanged(int state, int row)
{
  calibrationTargetManager_.setObjectiveFunctionEnabledState(state == Qt::CheckState::Checked, row);
}

void ObjectiveFunctionControllerSAC::slotSelectedPropertyChanged(const QString& propertyCauldronName, const QString& propertyUserName)
{
  scenario_.calibrationTargetManager().addToMapping(propertyUserName, propertyCauldronName);
}

}  // namespace casaWizard
