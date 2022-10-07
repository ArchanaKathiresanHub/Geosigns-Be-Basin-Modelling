//
// Copyright (C) 2012-2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include "view/objectiveFunctionTable.h"

namespace casaWizard
{

class ObjectiveFunctionManager;

class ObjectiveFunctionTableSAC : public ObjectiveFunctionTable
{
  Q_OBJECT

public:
  explicit ObjectiveFunctionTableSAC(QWidget* parent = nullptr);

  void updateTable(const ObjectiveFunctionManager& objectiveFunction) final;

private:
  QWidget* createEnabledCheckBox(const bool enabled);

signals:
  void enabledStateChanged(int, int);
};

}  // namespace casaWizard
