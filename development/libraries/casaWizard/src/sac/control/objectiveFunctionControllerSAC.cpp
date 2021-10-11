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
#include "model/objectiveFunctionManager.h"
#include "view/objectiveFunctionTableSAC.h"

#include <QTableWidgetItem>

namespace casaWizard
{

ObjectiveFunctionControllerSAC::ObjectiveFunctionControllerSAC(ObjectiveFunctionTableSAC* objectiveFunctionTableSAC,
                                                               casaWizard::CasaScenario& scenario,
                                                               QObject* parent) :
  ObjectiveFunctionController{objectiveFunctionTableSAC, scenario, parent},
  objectiveFunctionTableSAC_{objectiveFunctionTableSAC},  
  scenario_{scenario}
{
  connect(objectiveFunctionTableSAC_, SIGNAL(enabledStateChanged(int, int)),
          this,                       SLOT(slotEnabledStateChanged(int, int)));
}

void ObjectiveFunctionControllerSAC::slotEnabledStateChanged(int state, int row)
{
  scenario_.objectiveFunctionManager().setEnabledState(state == Qt::CheckState::Checked, row);
}

int ObjectiveFunctionControllerSAC::offsetColumnToObjectiveFunctionManagerValue() const
{
  return 1;
}

}  // namespace casaWizard
