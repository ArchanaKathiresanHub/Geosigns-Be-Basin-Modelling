#include "depthTargetTable.h"

#include "model/predictionTargetDepth.h"
#include "view/tableRowComboBox.h"

#include <QComboBox>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QSignalBlocker>
#include <QTableWidget>
#include <QVBoxLayout>

namespace casaWizard
{

namespace ua
{

DepthTargetTable::DepthTargetTable(QWidget* parent) : QWidget(parent),
  tableWidgetPredictionTargets_{new QTableWidget(this)},
  pushButtonAddPredictionTarget_{new QPushButton("Add", this)},
  pushButtonDelPredictionTarget_{new QPushButton("Delete", this)},
  pushButtonCopyPredictionTarget_{new QPushButton("Copy", this)},
  checkBoxColumnNumber_{4}
{
  tableWidgetPredictionTargets_->setRowCount(0);
  tableWidgetPredictionTargets_->setColumnCount(5);
  tableWidgetPredictionTargets_->horizontalHeader()->setStretchLastSection(true);
  tableWidgetPredictionTargets_->setHorizontalHeaderItem(0, new QTableWidgetItem("Property"));
  tableWidgetPredictionTargets_->setHorizontalHeaderItem(1, new QTableWidgetItem("x [m]"));
  tableWidgetPredictionTargets_->setHorizontalHeaderItem(2, new QTableWidgetItem("y [m]"));
  tableWidgetPredictionTargets_->setHorizontalHeaderItem(3, new QTableWidgetItem("z [m]"));
  tableWidgetPredictionTargets_->setHorizontalHeaderItem(checkBoxColumnNumber_, new QTableWidgetItem("time series"));
  tableWidgetPredictionTargets_->horizontalHeader()->stretchLastSection();

  tableWidgetPredictionTargets_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

  QHBoxLayout* horizontalLayoutPredictionButtons = new QHBoxLayout();
  horizontalLayoutPredictionButtons->addWidget(new QWidget(this));
  horizontalLayoutPredictionButtons->addWidget(pushButtonCopyPredictionTarget_);
  horizontalLayoutPredictionButtons->addWidget(pushButtonAddPredictionTarget_);
  horizontalLayoutPredictionButtons->addWidget(pushButtonDelPredictionTarget_);
  horizontalLayoutPredictionButtons->setStretch(0, 1);

  QVBoxLayout* verticalLayoutPrediction = new QVBoxLayout(this);
  verticalLayoutPrediction->addWidget(tableWidgetPredictionTargets_);
  verticalLayoutPrediction->addLayout(horizontalLayoutPredictionButtons);
  verticalLayoutPrediction->setStretch(0, 1);
}

void DepthTargetTable::updateTable(const QVector<PredictionTargetDepth> depthTargets,
                                        const QStringList& predictionTargetOptions,
                                        const QVector<bool>& hasTimeSeriesForPredictionTargets)
{
  QSignalBlocker blocker(tableWidgetPredictionTargets_);
  tableWidgetPredictionTargets_->clearContents();
  tableWidgetPredictionTargets_->setRowCount(0);

  int row = 0;
  for (const PredictionTargetDepth& target : depthTargets)
  {
    TableRowComboBox* propertyComboBox = new TableRowComboBox(row, tableWidgetPredictionTargets_);
    propertyComboBox->insertItems(0, predictionTargetOptions);
    int i =0;
    for (const QString& option : predictionTargetOptions)
    {
      if (target.property() == option)
      {
        propertyComboBox->setCurrentIndex(i);
        break;
      }
      ++i;
    }

    QTableWidgetItem* item = new QTableWidgetItem(target.property());
    connect(propertyComboBox, &TableRowComboBox::currentTextChanged,
            [=](){ item->setText(propertyComboBox->currentText());});

    QTableWidgetItem* itemCheckBox = new QTableWidgetItem();
    itemCheckBox->data(Qt::CheckStateRole);
    itemCheckBox->setCheckState(hasTimeSeriesForPredictionTargets[row] ? Qt::Checked : Qt::Unchecked);

    tableWidgetPredictionTargets_->setRowCount(row+1);
    tableWidgetPredictionTargets_->setCellWidget(row, 0, propertyComboBox);
    tableWidgetPredictionTargets_->setItem(row, 0, item);
    tableWidgetPredictionTargets_->setItem(row, 1, new QTableWidgetItem(QString::number(target.x(), 'f', 0)));
    tableWidgetPredictionTargets_->setItem(row, 2, new QTableWidgetItem(QString::number(target.y(), 'f', 0)));
    tableWidgetPredictionTargets_->setItem(row, 3, new QTableWidgetItem(QString::number(target.z(), 'f', 0)));

    // Note: this should always be the last column
    tableWidgetPredictionTargets_->setItem(row, checkBoxColumnNumber(), itemCheckBox);
    ++row;
  }
}

const QTableWidget* DepthTargetTable::tableWidgetDepthTargets() const
{
  return tableWidgetPredictionTargets_;
}

const QPushButton* DepthTargetTable::pushButtonAddDepthTarget() const
{
  return pushButtonAddPredictionTarget_;
}

const QPushButton* DepthTargetTable::pushButtonDelDepthTarget() const
{
  return pushButtonDelPredictionTarget_;
}

const QPushButton* DepthTargetTable::pushButtonCopyDepthTarget() const
{
  return pushButtonCopyPredictionTarget_;
}

int DepthTargetTable::checkBoxColumnNumber() const
{
  return checkBoxColumnNumber_;
}

} // namespace ua

} // namespace casaWizard
