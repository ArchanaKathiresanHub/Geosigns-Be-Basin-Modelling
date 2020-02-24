#include "surfaceTargetController.h"

#include "model/logger.h"
#include "model/uaScenario.h"
#include "view/surfaceTargetTable.h"

#include <QPushButton>
#include <QString>
#include <QTableWidgetItem>

namespace casaWizard
{

namespace ua
{

SurfaceTargetController::SurfaceTargetController(SurfaceTargetTable* table,
                                                 PredictionTargetManager& manager,
                                                 QObject* parent) :
  QObject(parent),
  table_(table),
  manager_(manager)
{
  connect(table_->pushButtonAddSurfaceTarget(),  SIGNAL(clicked()), this, SLOT(slotPushButtonAddSurfaceTargetClicked()));
  connect(table_->pushButtonDelSurfaceTarget(),  SIGNAL(clicked()), this, SLOT(slotPushButtonDelSurfaceTargetClicked()));
  connect(table_->pushButtonCopySurfaceTarget(), SIGNAL(clicked()), this, SLOT(slotPushButtonCopySurfaceTargetClicked()));
  connect(table_->tableWidgetSurfaceTargets(),   SIGNAL(itemChanged(QTableWidgetItem*)),
          this,                                  SLOT(slotTableWidgetSurfaceTargetsItemChanged(QTableWidgetItem*)));
  connect(table_->tableWidgetSurfaceTargets(),   SIGNAL(itemChanged(QTableWidgetItem*)),
          this,                                  SLOT(slotSurfaceTargetCheckBoxStateChanged(QTableWidgetItem*)));

  connect(parent, SIGNAL(signalRefreshChildWidgets()), this, SLOT(slotRefresh()));
}

void  SurfaceTargetController::slotPushButtonAddSurfaceTargetClicked()
{
  manager_.addSurfaceTarget(manager_.predictionTargetOptions()[0], 0.0, 0.0);
  slotRefresh();
}

void SurfaceTargetController::slotPushButtonDelSurfaceTargetClicked()
{
  int row = table_->tableWidgetSurfaceTargets()->currentRow();
  if (row < 0)
  {
    row = table_->tableWidgetSurfaceTargets()->rowCount()-1;
  }
  if (row >= 0)
  {
    manager_.removeSurfaceTarget(row);
  }
  slotRefresh();
}

void SurfaceTargetController::slotPushButtonCopySurfaceTargetClicked()
{
  int row = table_->tableWidgetSurfaceTargets()->currentRow();
  if (row < 0)
  {
    Logger::log() << "Select an existing prediction target to copy" << Logger::endl();
    return;
  }
  manager_.copySurfaceTarget(row);
  slotRefresh();
}

void SurfaceTargetController::slotTableWidgetSurfaceTargetsItemChanged(QTableWidgetItem* item)
{
  manager_.setSurfaceTarget(item->row(), item->column(), item->data(0).toString());
}

void SurfaceTargetController::slotRefresh()
{
  table_->updateTable(manager_.surfaceTargets(), manager_.predictionTargetOptions(), manager_.surfaceTargetHasTimeSeries(), manager_.validLayerNames());
}

void SurfaceTargetController::slotSurfaceTargetCheckBoxStateChanged(QTableWidgetItem* item)
{
  if (item->column() == table_->checkBoxColumnNumber())
  {
    manager_.setSurfaceTargetHasTimeSeries(item->row(), item->checkState() == Qt::Checked);
  }
}

} // namespace ua

} // namespace casaWizard
