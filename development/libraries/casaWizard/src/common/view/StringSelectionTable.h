//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include <QTableWidget>
#include <QVector>

namespace casaWizard
{

class CustomCheckbox;

class StringSelectionTable : public QTableWidget
{
   Q_OBJECT

public:
   StringSelectionTable(QString headerName = "", QWidget *parent = nullptr);

   void updateTable(const QStringList& strings);
   QVector<bool> selectionStates() const;

public slots:
   void slotSelectAll();

private:
   QVector<CustomCheckbox*> m_selectionStates;
};

} // namespace casaWizard
