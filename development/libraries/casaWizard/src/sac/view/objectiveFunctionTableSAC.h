//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include <QTableWidget>

namespace casaWizard
{

class ObjectiveFunctionManager;

class ObjectiveFunctionTableSAC : public QTableWidget
{
  Q_OBJECT

public:
  explicit ObjectiveFunctionTableSAC(QWidget* parent = nullptr);

  void updateTable(const ObjectiveFunctionManager& objectiveFunction);

private:
  QWidget* createEnabledCheckBox(const bool enabled);

signals:
  void enabledStateChanged(int, int);
};

}  // namespace casaWizard
