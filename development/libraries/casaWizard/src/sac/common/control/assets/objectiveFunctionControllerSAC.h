//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// Controller to list the objective functions
#pragma once

#include "control/objectiveFunctionController.h"

namespace casaWizard
{

class ObjectiveFunctionTableSAC;
class CasaScenario;

class ObjectiveFunctionControllerSAC : public ObjectiveFunctionController
{
  Q_OBJECT

public:
  ObjectiveFunctionControllerSAC(ObjectiveFunctionTableSAC* objectiveFunctionTableSAC,
                                 casaWizard::CasaScenario& scenario, QObject* parent);
private slots:  
  void slotEnabledStateChanged(int state, int row);

private:
  int offsetColumnToObjectiveFunctionManagerValue() const override;

  ObjectiveFunctionTableSAC* objectiveFunctionTableSAC_;  
  casaWizard::CasaScenario& scenario_;
};

}  // namespace casaWizard
