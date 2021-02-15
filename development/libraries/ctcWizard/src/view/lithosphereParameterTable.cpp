#include "lithosphereParameterTable.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

namespace ctcWizard
{

LithosphereParameterTable::LithosphereParameterTable(QWidget *parent) : QWidget(parent),
  tableWidgetLithosphereParameter_{new QTableWidget(this)}
{
  tableWidgetLithosphereParameter_->setRowCount(3);
  tableWidgetLithosphereParameter_->setColumnCount(2);
  tableWidgetLithosphereParameter_->setHorizontalHeaderItem(0, new QTableWidgetItem("Lithosphere"));
  tableWidgetLithosphereParameter_->setHorizontalHeaderItem(1, new QTableWidgetItem("Value"));
  QSizePolicy sizePolicy1(QSizePolicy::Maximum, QSizePolicy::Maximum);
  sizePolicy1.setHorizontalStretch(1);
  sizePolicy1.setVerticalStretch(1);
  sizePolicy1.setHeightForWidth(tableWidgetLithosphereParameter_->sizePolicy().hasHeightForWidth());
  tableWidgetLithosphereParameter_->setSizePolicy(sizePolicy1);
  tableWidgetLithosphereParameter_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  tableWidgetLithosphereParameter_->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
  tableWidgetLithosphereParameter_->setSortingEnabled(false);
  tableWidgetLithosphereParameter_->setRowCount(3);
  tableWidgetLithosphereParameter_->setColumnCount(2);
  tableWidgetLithosphereParameter_->horizontalHeader()->setVisible(true);
  tableWidgetLithosphereParameter_->horizontalHeader()->setCascadingSectionResizes(true);
  tableWidgetLithosphereParameter_->horizontalHeader()->resizeSection(0,50);
  tableWidgetLithosphereParameter_->horizontalHeader()->resizeSection(1,50);
  tableWidgetLithosphereParameter_->horizontalHeader()->setStretchLastSection(false);

  QVBoxLayout* verticalLayout = new QVBoxLayout(this);
  verticalLayout->addWidget(tableWidgetLithosphereParameter_);
}

void LithosphereParameterTable::updateTable(const QVector<LithosphereParameter>& varPrms)
{
  tableWidgetLithosphereParameter_->clearContents();
  tableWidgetLithosphereParameter_->setRowCount(0);
  tableWidgetLithosphereParameter_->horizontalHeader()->resizeSection(0,350);
  tableWidgetLithosphereParameter_->horizontalHeader()->resizeSection(1,100);

  int row = 0;
  for (const LithosphereParameter& varPrm : varPrms)
  {
    tableWidgetLithosphereParameter_->setRowCount(row + 1);
    tableWidgetLithosphereParameter_->setItem(row, 0, new QTableWidgetItem(varPrm.param));
    tableWidgetLithosphereParameter_->setItem(row, 1, new QTableWidgetItem(varPrm.value));
    ++row;
  }
  tableWidgetLithosphereParameter_->setRowCount(3);
}

const QTableWidget* LithosphereParameterTable::tableWidgetLithosphereParameter() const
{
  return tableWidgetLithosphereParameter_;
}

} // namespace ctcWizard
