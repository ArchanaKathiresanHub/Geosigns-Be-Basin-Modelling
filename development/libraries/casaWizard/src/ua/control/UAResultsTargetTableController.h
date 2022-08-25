//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include "model/UAResultsTargetsData.h"

#include <QObject>

namespace casaWizard
{

namespace ua
{

class UAResultsTargetTable;
class PredictionTargetManager;

class UAResultsTargetTableController : public QObject
{
   Q_OBJECT

public:
   UAResultsTargetTableController(UAResultsTargetTable* targetTable);
   void setTableData(const QVector<const PredictionTarget*> predictionTargets, const QVector<bool>& hasTimeSeries);

   void disableRowsWithoutTimeSeries();
   void enableAllRows();

   bool hasData();
   int currentTarget();

signals:
   void targetClicked(int);

public slots:
   void slotTablePredictionTargetClicked(int, int);

private slots:
   void headerClicked(int column);

private:
   UAResultsTargetTable* m_targetTable;
   UAResultsTargetsData m_tableData;
   bool m_rowsWithoutTimeSeriesDisabled;
};

} //ua
} //casaWizard

