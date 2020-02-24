#include "depthTargetController.h"

#include "model/logger.h"
#include "model/uaScenario.h"
#include "view/depthTargetTable.h"

#include <QPushButton>
#include <QString>
#include <QTableWidgetItem>

namespace casaWizard
{

namespace ua
{

DepthTargetController::DepthTargetController(DepthTargetTable* table,
                                             PredictionTargetManager& manager,
                                             QObject* parent) :
  QObject(parent),
  table_(table),
  manager_(manager)
{
  connect(table_->pushButtonAddDepthTarget(),  SIGNAL(clicked()), this, SLOT(slotPushButtonAddDepthTargetClicked()));
  connect(table_->pushButtonDelDepthTarget(),  SIGNAL(clicked()), this, SLOT(slotPushButtonDelDepthTargetClicked()));
  connect(table_->pushButtonCopyDepthTarget(), SIGNAL(clicked()), this, SLOT(slotPushButtonCopyDepthTargetClicked()));
  connect(table_->tableWidgetDepthTargets(),   SIGNAL(itemChanged(QTableWidgetItem*)),
          this,                                SLOT(slotTableWidgetDepthTargetsItemChanged(QTableWidgetItem*)));
  connect(table_->tableWidgetDepthTargets(),   SIGNAL(itemChanged(QTableWidgetItem*)),
          this,                                SLOT(slotDepthTargetCheckBoxStateChanged(QTableWidgetItem*)));

  connect(parent, SIGNAL(signalRefreshChildWidgets()), this, SLOT(slotRefresh()));
}

void DepthTargetController::slotPushButtonAddDepthTargetClicked()
{
  manager_.addDepthTarget(manager_.predictionTargetOptions()[0], 0.0, 0.0, 0.0);
  slotRefresh();
}

void DepthTargetController::slotPushButtonDelDepthTargetClicked()
{
  int row = table_->tableWidgetDepthTargets()->currentRow();
  if (row < 0)
  {
    row = table_->tableWidgetDepthTargets()->rowCount()-1;
  }
  if (row >= 0)
  {
    manager_.removeDepthTarget(row);
  }
  slotRefresh();
}

void DepthTargetController::slotPushButtonCopyDepthTargetClicked()
{
  int row = table_->tableWidgetDepthTargets()->currentRow();
  if (row < 0)
  {
    Logger::log() << "Select an existing prediction target to copy" << Logger::endl();
    return;
  }
  manager_.copyDepthTarget(row);
  slotRefresh();
}

void DepthTargetController::slotTableWidgetDepthTargetsItemChanged(QTableWidgetItem* item)
{
  manager_.setDepthTarget(item->row(), item->column(), item->data(0).toString());
}

void DepthTargetController::slotRefresh()
{
  table_->updateTable(manager_.depthTargets(), manager_.predictionTargetOptions(), manager_.depthTargetHasTimeSeries());
}

void DepthTargetController::slotDepthTargetCheckBoxStateChanged(QTableWidgetItem* item)
{
  if (item->column() == table_->checkBoxColumnNumber())
  {
    manager_.setDepthTargetHasTimeSeries(item->row(), item->checkState() == Qt::Checked);
  }
}

} // namespace ua

} // namespace casaWizard
