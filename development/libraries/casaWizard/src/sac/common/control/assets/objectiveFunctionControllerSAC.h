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

namespace sac
{

class ObjectiveFunctionTableSAC;
class SacScenario;

class ObjectiveFunctionControllerSAC : public ObjectiveFunctionController
{
   Q_OBJECT

public:
   ObjectiveFunctionControllerSAC(ObjectiveFunctionTableSAC* objectiveFunctionTableSAC,
                                  SacScenario& scenario, QObject* parent);
private slots:  
   void slotEnabledStateChanged(int state, int row);

private:
   int offsetColumnToObjectiveFunctionManagerValue() const override;

   ObjectiveFunctionTableSAC* m_objectiveFunctionTable;
   SacScenario& m_scenario;

signals:
   void refresh();
};

}  // namespace sac

}  // namespace casaWizard
