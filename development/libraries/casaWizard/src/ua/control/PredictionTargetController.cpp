//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "PredictionTargetController.h"

#include "model/logger.h"
#include "model/uaScenario.h"
#include "view/PredictionTargetTable.h"

#include <QPushButton>
#include <QString>
#include <QTableWidgetItem>

namespace casaWizard
{

namespace ua
{

PredictionTargetController::PredictionTargetController(PredictionTargetTable* table,
                                                       PredictionTargetManager& manager,
                                                       QObject* parent) :
  QObject(parent),
  m_table(table),
  m_manager(manager)
{
  connect(m_table->pushButtonAddSurfaceTarget(),  SIGNAL(clicked()), this, SLOT(slotPushButtonAddPredictionTargetClicked()));
  connect(m_table->pushButtonDelSurfaceTarget(),  SIGNAL(clicked()), this, SLOT(slotPushButtonDelPredictionTargetClicked()));
  connect(m_table->pushButtonCopySurfaceTarget(), SIGNAL(clicked()), this, SLOT(slotPushButtonCopyPredictionTargetClicked()));
  connect(m_table->tableWidgetSurfaceTargets(),   SIGNAL(itemChanged(QTableWidgetItem*)),
          this,                                  SLOT(slotTableWidgetPredictionTargetsItemChanged(QTableWidgetItem*)));
  connect(m_table,   SIGNAL(targetHasTimeSeriesChanged(int, int)),
          this,                                  SLOT(slotTargetHasTimeSeriesChanged(int, int)));

  connect(m_table,   SIGNAL(activePropertyCheckBoxChanged(int, int, QString)),
          this,                                  SLOT(slotActivePropertyCheckBoxChanged(int, int, QString)));

  connect(parent, SIGNAL(signalRefreshChildWidgets()), this, SLOT(slotRefresh()));
}

void PredictionTargetController::slotPushButtonAddPredictionTargetClicked()
{
   m_manager.addDepthTarget(0.0, 0.0, 0.0);
   refreshAndEmitDataChanged();
}

void PredictionTargetController::slotPushButtonDelPredictionTargetClicked()
{
   m_manager.removeTargets(getSelectedRows());
   refreshAndEmitDataChanged();
}

QVector<int> PredictionTargetController::getSelectedRows()
{
   const auto selectedItems = m_table->tableWidgetSurfaceTargets()->selectedItems();

   QVector<int> rows;
   for (const auto& item : selectedItems)
   {
      if (!rows.contains(item->row()))
      {
         rows.append(item->row());
      }
   }

   return rows;
}

void PredictionTargetController::slotPushButtonCopyPredictionTargetClicked()
{
   getSelectedRows();
   m_manager.copyTargets(getSelectedRows());
   refreshAndEmitDataChanged();
}

void PredictionTargetController::slotTableWidgetPredictionTargetsItemChanged(QTableWidgetItem* item)
{
  m_manager.setTarget(item->row(), item->column(), item->data(0).toString());
  refreshAndEmitDataChanged();
}

void PredictionTargetController::refreshAndEmitDataChanged()
{
   slotRefresh();
   emit dataChanged();
}

void PredictionTargetController::slotRefresh()
{
   m_table->updateTable(m_manager.predictionTargets(), m_manager.predictionTargetOptions(), m_manager.targetHasTimeSeries(), m_manager.validSurfaceNames());
}

void PredictionTargetController::slotActivePropertyCheckBoxChanged(int state, int row, QString property)
{
   m_manager.setTargetActiveProperty(state == Qt::Checked, row, property);
   emit dataChanged();
}

void PredictionTargetController::slotTargetHasTimeSeriesChanged(int state, int row)
{
   m_manager.setTargetHasTimeSeries(row, state == Qt::Checked);
   emit dataChanged();
}

} // namespace ua

} // namespace casaWizard
