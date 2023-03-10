#include "qcDoeOptionController.h"

#include "model/uaScenario.h"
#include "model/logger.h"
#include "view/qcDoeOptionTable.h"

#include <QComboBox>
#include <QString>
#include <QTableWidgetItem>

namespace casaWizard
{

namespace ua
{

QCDoeOptionController::QCDoeOptionController(QCDoeOptionTable* table,
                                             UAScenario& casaScenario,
                                             QObject* parent) :
  QObject(parent),
  table_(table),
  casaScenario_(casaScenario)
{
  table_->setProxyComboBoxes(casaScenario_.proxy());

  connect(table_->tableWidgetQcDoeOption(), SIGNAL(itemChanged(QTableWidgetItem*)),
          this,                             SLOT(slotTableWidgetQcDoeOptionItemChanged(QTableWidgetItem*)));
  connect(table_->comboBoxProxyOrder(),     SIGNAL(currentTextChanged(QString)) ,
          this,                             SLOT(slotComboBoxProxyOrderItemChanged(QString)));
  connect(table_->comboBoxKriging(),        SIGNAL(currentTextChanged(QString)),
          this,                             SLOT(slotComboBoxKrigingItemChanged(QString)));

  connect(parent, SIGNAL(signalRefreshChildWidgets()), this, SLOT(slotRefresh()));
  slotRefresh();
}

void QCDoeOptionController::updateAfterModification()
{
  emit modelChange();
}

void QCDoeOptionController::slotTableWidgetQcDoeOptionItemChanged(QTableWidgetItem* item)
{
  casaScenario_.setIsQcDoeOptionSelected(item->row(), item->checkState() == Qt::Checked);
  updateAfterModification();
}

void QCDoeOptionController::slotComboBoxProxyOrderItemChanged(QString proxyOrder)
{
  casaScenario_.setProxyOrder(proxyOrder.toInt());
  updateAfterModification();
}

void QCDoeOptionController::slotComboBoxKrigingItemChanged(QString krigingMethod)
{
  casaScenario_.setProxyKrigingMethod(krigingMethod);
  updateAfterModification();
}

void QCDoeOptionController::slotRefresh()
{
  table_->updateTable(casaScenario_.doeOptionSelectedNames(), casaScenario_.isQcDoeOptionSelected());
  table_->setProxyComboBoxes(casaScenario_.proxy());
}

} // namespace ua

} // namespace casaWizard
