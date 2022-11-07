//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "objectiveFunctionControllerSAC.h"

#include "model/calibrationTargetManager.h"
#include "model/SacScenario.h"
#include "model/objectiveFunctionManager.h"
#include "view/assets/objectiveFunctionTableSAC.h"

#include <QTableWidgetItem>

namespace casaWizard
{

namespace sac
{

ObjectiveFunctionControllerSAC::ObjectiveFunctionControllerSAC(ObjectiveFunctionTableSAC* objectiveFunctionTableSAC,
                                                               SacScenario& scenario,
                                                               QObject* parent) :
  ObjectiveFunctionController{objectiveFunctionTableSAC, scenario, parent},
  m_objectiveFunctionTable{objectiveFunctionTableSAC},
  m_scenario{scenario}
{
   connect(m_objectiveFunctionTable, SIGNAL(enabledStateChanged(int, int)), this, SLOT(slotEnabledStateChanged(int, int)));
}

void ObjectiveFunctionControllerSAC::slotEnabledStateChanged(int state, int row)
{
  m_scenario.objectiveFunctionManager().setEnabledState(state == Qt::CheckState::Checked, row);
  m_scenario.setCalibrationTargetsBasedOnObjectiveFunctions();
  emit refresh();
}

int ObjectiveFunctionControllerSAC::offsetColumnToObjectiveFunctionManagerValue() const
{
  return 1;
}

}  // namespace sac

}  // namespace casaWizard
