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
   m_targetTable(targetTable)
{
   connect(m_targetTable->horizontalHeader(), SIGNAL(sectionClicked(int)), this, SLOT(headerClicked(int)));
   connect(m_targetTable, SIGNAL(cellClicked(int,int)), this, SLOT(slotTablePredictionTargetClicked(int, int)));
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

void UAResultsTargetTableController::setTableData(const QVector<const PredictionTarget*> predictionTargets)
{
   const QVector<QString>& targetOptions = PredictionTargetManager::predictionTargetOptions();
   m_tableData.setData(predictionTargets,targetOptions);
   m_targetTable->fillTable(m_tableData);
}

void UAResultsTargetTableController::headerClicked(int column)
{
   int targetIndex = currentTarget();

   m_tableData.sortData(column);
   m_targetTable->fillTable(m_tableData);

   //Such that the selected target is again selected after sorting.
   int rowIdx = m_tableData.rowIndex(targetIndex);
   m_targetTable->setCurrentCell(rowIdx,0);
}

void UAResultsTargetTableController::slotTablePredictionTargetClicked(int row, int /*column*/)
{
   if (row < 0)
   {
      return;
   }
   emit targetClicked(m_tableData.targetIndex(row));
}

} //ua
} //casaWizard
