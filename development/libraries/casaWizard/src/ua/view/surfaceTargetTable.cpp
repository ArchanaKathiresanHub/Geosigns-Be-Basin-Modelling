#include "surfaceTargetTable.h"

#include "model/predictionTargetSurface.h"
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

SurfaceTargetTable::SurfaceTargetTable(QWidget* parent) : QWidget(parent),
  tableWidgetTargets_{new QTableWidget(this)},
  pushButtonAddTarget_{new QPushButton("Add", this)},
  pushButtonDelTarget_{new QPushButton("Delete", this)},
  pushButtonCopyTarget_{new QPushButton("Copy", this)},
  checkBoxColumnNumber_{4}
{
  tableWidgetTargets_->setRowCount(0);
  tableWidgetTargets_->setColumnCount(5);
  tableWidgetTargets_->horizontalHeader()->setStretchLastSection(true);
  tableWidgetTargets_->setHorizontalHeaderItem(0, new QTableWidgetItem("Property"));
  tableWidgetTargets_->setHorizontalHeaderItem(1, new QTableWidgetItem("x [m]"));
  tableWidgetTargets_->setHorizontalHeaderItem(2, new QTableWidgetItem("y [m]"));
  tableWidgetTargets_->setHorizontalHeaderItem(3, new QTableWidgetItem("Layer"));
  tableWidgetTargets_->setHorizontalHeaderItem(checkBoxColumnNumber_, new QTableWidgetItem("time series"));
  tableWidgetTargets_->horizontalHeader()->stretchLastSection();

  tableWidgetTargets_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

  QHBoxLayout* horizontalLayoutPredictionButtons = new QHBoxLayout();
  horizontalLayoutPredictionButtons->addWidget(new QWidget(this));
  horizontalLayoutPredictionButtons->addWidget(pushButtonCopyTarget_);
  horizontalLayoutPredictionButtons->addWidget(pushButtonAddTarget_);
  horizontalLayoutPredictionButtons->addWidget(pushButtonDelTarget_);
  horizontalLayoutPredictionButtons->setStretch(0, 1);

  QVBoxLayout* verticalLayoutPrediction = new QVBoxLayout(this);
  verticalLayoutPrediction->addWidget(tableWidgetTargets_);
  verticalLayoutPrediction->addLayout(horizontalLayoutPredictionButtons);
  verticalLayoutPrediction->setStretch(0, 1);
}

void SurfaceTargetTable::updateTable(const QVector<PredictionTargetSurface> surfaceTargets,
                                        const QStringList predictionTargetOptions,
                                        const QVector<bool>& hasTimeSeriesForPredictionTargets,
                                        const QStringList validLayerNames)
{
  QSignalBlocker blocker(tableWidgetTargets_);
  tableWidgetTargets_->clearContents();
  tableWidgetTargets_->setRowCount(0);

  int row = 0;
  for (const PredictionTargetSurface& target : surfaceTargets)
  {
    TableRowComboBox* propertyComboBox = new TableRowComboBox(row, tableWidgetTargets_);
    propertyComboBox->insertItems(0, predictionTargetOptions);
    int i = 0;
    for (const QString& option : predictionTargetOptions)
    {
      if (target.property() == option)
      {
        propertyComboBox->setCurrentIndex(i);
        break;
      }
      ++i;
    }

    TableRowComboBox* layerComboBox = new TableRowComboBox(row, tableWidgetTargets_);
    layerComboBox->insertItems(0, validLayerNames);
    int j = 0;
    for (const QString layer: validLayerNames)
    {
      if (target.layerName() == layer)
      {
        layerComboBox->setCurrentIndex(j);
        break;
      }
      ++j;
    }

    QTableWidgetItem* itemProperty = new QTableWidgetItem(target.property());
    connect(propertyComboBox, &TableRowComboBox::currentTextChanged,
            [=](){ itemProperty->setText(propertyComboBox->currentText());});

    QTableWidgetItem* itemLayer = new QTableWidgetItem(target.layerName());
    connect(layerComboBox, &TableRowComboBox::currentIndexChanged,
            [=](){ itemLayer->setText(layerComboBox->currentText());});

    QTableWidgetItem* itemCheckBox = new QTableWidgetItem();
    itemCheckBox->data(Qt::CheckStateRole);
    itemCheckBox->setCheckState(hasTimeSeriesForPredictionTargets[row] ? Qt::Checked : Qt::Unchecked);

    tableWidgetTargets_->setRowCount(row+1);
    tableWidgetTargets_->setCellWidget(row, 0, propertyComboBox);
    tableWidgetTargets_->setItem(row, 0, itemProperty);
    tableWidgetTargets_->setItem(row, 1, new QTableWidgetItem(QString::number(target.x(), 'f', 0)));
    tableWidgetTargets_->setItem(row, 2, new QTableWidgetItem(QString::number(target.y(), 'f', 0)));

    tableWidgetTargets_->setCellWidget(row, 3, layerComboBox);
    tableWidgetTargets_->setItem(row, 3, itemLayer);

    // Note: this should always be the last column
    tableWidgetTargets_->setItem(row, checkBoxColumnNumber(), itemCheckBox);
    ++row;
  }
}

const QTableWidget* SurfaceTargetTable::tableWidgetSurfaceTargets() const
{
  return tableWidgetTargets_;
}

const QPushButton* SurfaceTargetTable::pushButtonAddSurfaceTarget() const
{
  return pushButtonAddTarget_;
}

const QPushButton* SurfaceTargetTable::pushButtonDelSurfaceTarget() const
{
  return pushButtonDelTarget_;
}

const QPushButton* SurfaceTargetTable::pushButtonCopySurfaceTarget() const
{
  return pushButtonCopyTarget_;
}

int SurfaceTargetTable::checkBoxColumnNumber() const
{
  return checkBoxColumnNumber_;
}

} // namespace ua

} // namespace casaWizard
