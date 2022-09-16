#include "qcDoeOptionTable.h"

#include "model/proxy.h"
#include "view/components/helpLabel.h"

#include <QComboBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QTableWidget>
#include "model/logger.h"

namespace casaWizard
{

namespace ua
{

const int columnCheckBox = 0;
const int columnNames = 1;

QCDoeOptionTable::QCDoeOptionTable(QWidget* parent) :
  QWidget(parent),
  tableWidgetQcDoeOption_{new QTableWidget{this}},
  comboBoxProxyOrder_{new QComboBox{this}},
  comboBoxKriging_{new QComboBox{this}}
{
  QSizePolicy policy = sizePolicy();
  policy.setHorizontalPolicy(QSizePolicy::Policy::Expanding);
  policy.setVerticalPolicy(QSizePolicy::Policy::Expanding);
  tableWidgetQcDoeOption_->setSizePolicy(policy);
  tableWidgetQcDoeOption_->horizontalHeader()->setVisible(true);
  tableWidgetQcDoeOption_->horizontalHeader()->setStretchLastSection(true);
  tableWidgetQcDoeOption_->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  tableWidgetQcDoeOption_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  tableWidgetQcDoeOption_->setRowCount(0);
  tableWidgetQcDoeOption_->setColumnCount(2);
  tableWidgetQcDoeOption_->resizeColumnToContents(0);
  tableWidgetQcDoeOption_->setHorizontalHeaderItem(columnCheckBox, new QTableWidgetItem(""));
  tableWidgetQcDoeOption_->setHorizontalHeaderItem(columnNames, new QTableWidgetItem("DoE"));

  QFormLayout* layoutComboBoxes = new QFormLayout();
  for (const int& order : Proxy::orderOptions())
  {
    comboBoxProxyOrder_->addItem(QString::number(order));
  }
  comboBoxProxyOrder_->setCurrentIndex(2); //default to '2'
  comboBoxKriging_->insertItems(0, Proxy::krigingMethodOptions());

  QHBoxLayout* hbox = new QHBoxLayout();
  hbox->addWidget(comboBoxProxyOrder_);
  hbox->addWidget(new HelpLabel(this, "Order 3 leads to an iterative search for the optimal terms in the response surface proxy"));

  layoutComboBoxes->addRow(new QLabel("RSP order", this), hbox);
  layoutComboBoxes->addRow(new QLabel("Kriging", this), comboBoxKriging_);

  QHBoxLayout* layoutTableAndCombos = new QHBoxLayout(this);
  layoutTableAndCombos->addWidget(tableWidgetQcDoeOption_);
  layoutTableAndCombos->addLayout(layoutComboBoxes);
  layoutTableAndCombos->addWidget(new QWidget(this));
  layoutTableAndCombos->setStretch(2, 1);
}

void QCDoeOptionTable::updateTable(const QStringList& doeOptionNames, const QVector<bool>& isQcDoeOptionsSelected)
{
  QSignalBlocker blocker(tableWidgetQcDoeOption_);

  tableWidgetQcDoeOption_->clearContents();
  tableWidgetQcDoeOption_->setRowCount(doeOptionNames.size());

  int row = 0;
  for (const QString& doeOption : doeOptionNames)
  {
    QTableWidgetItem* itemCheckBox = new QTableWidgetItem("");
    itemCheckBox->setCheckState(isQcDoeOptionsSelected[row] ? Qt::Checked : Qt::Unchecked);
    tableWidgetQcDoeOption_->setItem(row, columnCheckBox, itemCheckBox);

    QTableWidgetItem* itemDoeName = new QTableWidgetItem(doeOption);
    itemDoeName->setFlags(itemDoeName->flags() ^ Qt::ItemIsEditable);
    tableWidgetQcDoeOption_->setItem(row, columnNames, itemDoeName);
    ++row;
  }
  tableWidgetQcDoeOption_->resizeColumnToContents(0);
}

void QCDoeOptionTable::setProxyComboBoxes(const Proxy& proxy)
{
  QSignalBlocker blocker1(comboBoxProxyOrder_);
  comboBoxProxyOrder_->setCurrentIndex(Proxy::orderOptions().indexOf(proxy.order()));

  QSignalBlocker blocker2(comboBoxKriging_);
  comboBoxKriging_->setCurrentIndex(Proxy::krigingMethodOptions().indexOf(proxy.krigingMethod()));
}

int QCDoeOptionTable::columnCheckBoxQcDoeOptionTable() const
{
  return columnCheckBox;
}

QComboBox* QCDoeOptionTable::comboBoxProxyOrder() const
{
  return comboBoxProxyOrder_;
}

QComboBox* QCDoeOptionTable::comboBoxKriging() const
{
  return comboBoxKriging_;
}

QTableWidget* QCDoeOptionTable::tableWidgetQcDoeOption() const
{
  return tableWidgetQcDoeOption_;
}

} // namespace ua

} // namespace casaWizard
