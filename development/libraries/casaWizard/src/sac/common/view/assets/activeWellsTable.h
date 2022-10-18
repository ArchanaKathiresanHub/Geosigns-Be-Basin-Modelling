//
// Copyright (C) 2012-2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include <QTableWidget>

namespace casaWizard
{

class Well;

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
   void checkBoxSelectionChanged();
};

}  // namespace sac

}  // namespace casaWizard
