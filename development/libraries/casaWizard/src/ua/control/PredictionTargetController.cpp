//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "PredictionTargetController.h"
#include "TargetImportWellsController.h"
#include "TargetImportPredictionTargetsController.h"

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
                                                       CalibrationTargetManager& calibrationTargetManager,
                                                       QObject* parent) :
  QObject(parent),
  m_table(table),
  m_predictionTargetManager(manager),
  m_calibrationTargetManager(calibrationTargetManager)
{
  connect(m_table->pushButtonAddTargetsAtWellLocations(),  SIGNAL(clicked()), this, SLOT(slotPushButtonAddTargetWellLocationsClicked()));
  connect(m_table->pushButtonAddPredictionTargets(), SIGNAL(clicked()), this, SLOT(slotPushButtonAddPredictionTargetsClicked()));
  connect(m_table->pushButtonAddTarget(),  SIGNAL(clicked()), this, SLOT(slotPushButtonAddPredictionTargetClicked()));
  connect(m_table->pushButtonDelTarget(),  SIGNAL(clicked()), this, SLOT(slotPushButtonDelPredictionTargetClicked()));
  connect(m_table->pushButtonCopyTarget(), SIGNAL(clicked()), this, SLOT(slotPushButtonCopyPredictionTargetClicked()));
  connect(m_table->tableWidgetTargets(),   SIGNAL(itemChanged(QTableWidgetItem*)),
          this,                                  SLOT(slotTableWidgetPredictionTargetsItemChanged(QTableWidgetItem*)));
  connect(m_table,   SIGNAL(targetHasTimeSeriesChanged(int, int)),
          this,                                  SLOT(slotTargetHasTimeSeriesChanged(int, int)));

  connect(m_table,   SIGNAL(activePropertyCheckBoxChanged(int, int, QString)),
          this,                                  SLOT(slotActivePropertyCheckBoxChanged(int, int, QString)));

  connect(parent, SIGNAL(signalRefreshChildWidgets()), this, SLOT(slotRefresh()));
}

void PredictionTargetController::slotPushButtonAddPredictionTargetClicked()
{
   m_predictionTargetManager.addDepthTarget(0.0, 0.0, 0.0);
   refreshAndEmitDataChanged();
}

void PredictionTargetController::slotPushButtonAddTargetWellLocationsClicked()
{
   TargetImportWellsController targetImportWellsController(m_calibrationTargetManager,
                                                           m_predictionTargetManager);
   refreshAndEmitDataChanged();
}

void PredictionTargetController::slotPushButtonAddPredictionTargetsClicked(){
    TargetImportPredictionTargetsController targetImportPredictionTargetsController(m_predictionTargetManager);
    refreshAndEmitDataChanged();
}

void PredictionTargetController::slotPushButtonDelPredictionTargetClicked()
{
   m_predictionTargetManager.removeTargets(getSelectedRows());
   refreshAndEmitDataChanged();
}

QVector<int> PredictionTargetController::getSelectedRows()
{
   const auto selectedItems = m_table->tableWidgetTargets()->selectedItems();

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
   m_predictionTargetManager.copyTargets(getSelectedRows());
   refreshAndEmitDataChanged();
}

void PredictionTargetController::slotTableWidgetPredictionTargetsItemChanged(QTableWidgetItem* item)
{
  m_predictionTargetManager.setTarget(item->row(), item->column(), item->data(0).toString());
  m_table->updateTableAtRow(m_predictionTargetManager.predictionTargets()[item->row()], item->row());
  emit dataChanged();
}

void PredictionTargetController::refreshAndEmitDataChanged()
{
   slotRefresh();
   emit dataChanged();
}

void PredictionTargetController::slotRefresh()
{
   QStringList validSurfaceNames = m_predictionTargetManager.validSurfaceNames();
   validSurfaceNames.insert(0, ""); // Add empty entry to be able to deselect the surface in the predictionTargetTable
   m_table->updateTable(m_predictionTargetManager.predictionTargets(), m_predictionTargetManager.predictionTargetOptions(), m_predictionTargetManager.targetHasTimeSeries(), validSurfaceNames);
}

void PredictionTargetController::enableTargetsAtWellLocationsButton(bool enable)
{
   m_table->setTargetsAtWellLocationsButtonEnabled(enable);
}

void PredictionTargetController::slotActivePropertyCheckBoxChanged(int state, int row, QString property)
{
   m_predictionTargetManager.setTargetActiveProperty(state == Qt::Checked, row, property);
   emit dataChanged();
}

void PredictionTargetController::slotTargetHasTimeSeriesChanged(int state, int row)
{
   m_predictionTargetManager.setTargetHasTimeSeries(row, state == Qt::Checked);
   emit dataChanged();
}

} // namespace ua

} // namespace casaWizard
