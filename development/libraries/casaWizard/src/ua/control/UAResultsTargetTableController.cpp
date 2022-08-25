//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "UAResultsTargetTableController.h"

#include "view/UAResultsTargetTable.h"

#include "model/predictionTargetManager.h"

#include <QHeaderView>

namespace casaWizard
{

namespace ua
{

UAResultsTargetTableController::UAResultsTargetTableController(UAResultsTargetTable* targetTable):
   m_targetTable(targetTable),
   m_rowsWithoutTimeSeriesDisabled(false)
{
   connect(m_targetTable->horizontalHeader(), SIGNAL(sectionClicked(int)), this, SLOT(headerClicked(int)));
   connect(m_targetTable, SIGNAL(enabledCellClicked(int,int)), this, SLOT(slotTablePredictionTargetClicked(int, int)));
}

bool UAResultsTargetTableController::hasData()
{
   if (m_tableData.targetData().size() > 0) return true;
   return false;
}

int UAResultsTargetTableController::currentTarget()
{
   return m_tableData.targetIndex(m_targetTable->currentRow());
}

void UAResultsTargetTableController::setTableData(const QVector<const PredictionTarget*> predictionTargets, const QVector<bool>& hasTimeSeries)
{
   const QVector<QString>& targetOptions = PredictionTargetManager::predictionTargetOptions();
   m_tableData.setData(predictionTargets,targetOptions, hasTimeSeries);
   m_targetTable->fillTable(m_tableData);
}

void UAResultsTargetTableController::headerClicked(int column)
{
   int targetIndex = currentTarget();

   m_tableData.sortData(column);
   m_targetTable->fillTable(m_tableData);

   if (m_rowsWithoutTimeSeriesDisabled)
   {
      disableRowsWithoutTimeSeries();
   }

   //Such that the selected target is again selected after sorting.
   int rowIdx = m_tableData.rowIndex(targetIndex);
   m_targetTable->setCurrentCell(rowIdx,0);
   emit targetClicked(targetIndex);
}

void UAResultsTargetTableController::slotTablePredictionTargetClicked(int row, int /*column*/)
{
   if (row < 0)
   {
      return;
   }

   emit targetClicked(m_tableData.targetIndex(row));

}

void UAResultsTargetTableController::enableAllRows()
{
   m_rowsWithoutTimeSeriesDisabled = false;
   m_targetTable->enableAllRows();
}

void UAResultsTargetTableController::disableRowsWithoutTimeSeries()
{
   m_rowsWithoutTimeSeriesDisabled = true;
   QVector<int> tableRows = m_tableData.tableRowsWithoutTimeSeries();
   m_targetTable->disableRows(tableRows);
}

} //ua
} //casaWizard
