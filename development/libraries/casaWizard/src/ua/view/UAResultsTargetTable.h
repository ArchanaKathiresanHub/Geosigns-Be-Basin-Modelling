//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include <QTableWidget>

#include <memory>

namespace casaWizard
{

namespace ua
{

class PredictionTarget;
class UAResultsTargetsData;

class UAResultsTargetTable : public QTableWidget
{
   Q_OBJECT
public:
   UAResultsTargetTable(QWidget *parent = Q_NULLPTR);
   void fillTable(const UAResultsTargetsData& targetsData);

   void disableRows(const QVector<int>& rowsToDisable);
   void enableAllRows();

   QStyleOptionViewItem viewOptions() const override;

signals:
   void enabledCellClicked(int,int);

private slots:
   void slotCellClicked(int,int);

private:
   int m_numDefaultCols;
   QSet<int> m_disabledRows;
   std::unique_ptr<QIcon> m_checkIcon;
};

} //ua
} //casaWizard
