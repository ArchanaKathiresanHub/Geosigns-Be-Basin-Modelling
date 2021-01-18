//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include "model/well.h"
#include <QTableWidget>

namespace casaWizard
{

namespace sac
{

class ActiveWellsTable : public QTableWidget
{
  Q_OBJECT

public:
  explicit ActiveWellsTable(QWidget* parent = nullptr);

  void updateTable(const QVector<const Well*> wells);

signals:
  void checkBoxChanged(int, int);
  void selectedWell(const QString&);

};

}  // namespace sac

}  // namespace casaWizard
