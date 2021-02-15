#include "riftingHistoryController.h"

#include "model/ctcScenario.h"
#include "view/riftingHistoryTable.h"

#include <QPushButton>
#include <QString>
#include <QTableWidgetItem>

namespace ctcWizard
{

RiftingHistoryController::RiftingHistoryController(RiftingHistoryTable* table,
                                                       CtcScenario& ctcScenario,
                                                       QObject* parent) :
  QObject(parent),
  table_(table),
  ctcScenario_(ctcScenario)
{
  connect(table_->tableWidgetRiftingHistory(),  SIGNAL(itemChanged(QTableWidgetItem*)),
          this,                                    SLOT(slotTableWidgetRiftingHistoryItemChanged(QTableWidgetItem*)));

  updateRiftingHistoryTable();
}

void RiftingHistoryController::slotTableWidgetRiftingHistoryItemChanged(QTableWidgetItem* item)
{
  ctcScenario_.setRiftingHistory(item->row(), item->column(), item->data(0).toString());
}

void RiftingHistoryController::updateRiftingHistoryTable()
{
  table_->updateTable(ctcScenario_.riftingHistory(), ctcScenario_.riftingHistoryOptions(), ctcScenario_.riftingHistoryRDAMaps(), ctcScenario_.riftingHistoryBasaltMaps());
}

void RiftingHistoryController::fillRiftTable()
{
  updateRiftingHistoryTable();
}

} // namespace ctcWizard
