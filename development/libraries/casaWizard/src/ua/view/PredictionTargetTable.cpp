//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "PredictionTargetTable.h"

#include "model/predictionTargetSurface.h"
#include "view/tableRowComboBox.h"
#include "view/components/customcheckbox.h"

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

PredictionTargetTable::PredictionTargetTable(QWidget* parent) : QWidget(parent),
  tableWidgetTargets_{new QTableWidget(this)},
  pushButtonAddTarget_{new QPushButton("Add", this)},
  pushButtonDelTarget_{new QPushButton("Delete", this)},
  pushButtonCopyTarget_{new QPushButton("Copy", this)},
  checkBoxColumnNumber_{5},
  propertyColumns_{}
{
  tableWidgetTargets_->setRowCount(0);
  tableWidgetTargets_->setColumnCount(6);
  tableWidgetTargets_->horizontalHeader()->setStretchLastSection(true);
  tableWidgetTargets_->setHorizontalHeaderItem(0, new QTableWidgetItem("Location name"));
  tableWidgetTargets_->setHorizontalHeaderItem(1, new QTableWidgetItem("x [m]"));
  tableWidgetTargets_->setHorizontalHeaderItem(2, new QTableWidgetItem("y [m]"));
  tableWidgetTargets_->setHorizontalHeaderItem(3, new QTableWidgetItem("z [m]"));
  tableWidgetTargets_->setHorizontalHeaderItem(4, new QTableWidgetItem("Surface"));
  tableWidgetTargets_->setHorizontalHeaderItem(checkBoxColumnNumber_, new QTableWidgetItem("Time series"));
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
  verticalLayoutPrediction->setContentsMargins(0, 0, 0, 0);
}

void PredictionTargetTable::updateTable(const QVector<const PredictionTarget*> predictionTargets,
                                        const QVector<QString>& predictionTargetOptions,
                                        const QVector<bool>& hasTimeSeriesForPredictionTargets,
                                        const QStringList validLayerNames)
{
   QSignalBlocker blocker(tableWidgetTargets_);
   tableWidgetTargets_->clearContents();
   tableWidgetTargets_->setRowCount(0);
   for (int i : propertyColumns_)
   {
     tableWidgetTargets_->removeColumn(i);
   }
   propertyColumns_.clear();

   for (const QString& option : predictionTargetOptions)
   {
      propertyColumns_.append(tableWidgetTargets_->columnCount() - 1);
      tableWidgetTargets_->setColumnCount(tableWidgetTargets_->columnCount() + 1);
      tableWidgetTargets_->setHorizontalHeaderItem(tableWidgetTargets_->columnCount() - 2, new QTableWidgetItem(option));
   }
   tableWidgetTargets_->setHorizontalHeaderItem(checkBoxColumnNumber_ + predictionTargetOptions.size(), new QTableWidgetItem("Time series"));

   int row = 0;
   for (const PredictionTarget* target : predictionTargets)
   {
      TableRowComboBox* layerComboBox = new TableRowComboBox(row, tableWidgetTargets_);
      layerComboBox->insertItems(0, validLayerNames);
      int j = 0;
      for (const QString& layer: validLayerNames)
      {
         if (target->surfaceName() == layer)
         {
            layerComboBox->setCurrentIndex(j);
            break;
         }
         ++j;
      }

      QTableWidgetItem* itemLayer = new QTableWidgetItem(target->surfaceName());
      connect(layerComboBox, &TableRowComboBox::currentIndexChanged,
              [=](){ itemLayer->setText(layerComboBox->currentText());});


      tableWidgetTargets_->setRowCount(row+1);
      tableWidgetTargets_->setItem(row, 0, new QTableWidgetItem(target->locationName()));
      tableWidgetTargets_->setItem(row, 1, new QTableWidgetItem(QString::number(target->x(), 'f', 0)));
      tableWidgetTargets_->setItem(row, 2, new QTableWidgetItem(QString::number(target->y(), 'f', 0)));
      tableWidgetTargets_->setItem(row, 3, new QTableWidgetItem(QString::number(target->z(), 'f', 0)));

      tableWidgetTargets_->setCellWidget(row, 4, layerComboBox);
      tableWidgetTargets_->setItem(row, 4, itemLayer);

      if (target->surfaceName() != "")
      {
         tableWidgetTargets_->item(row, 3)->setFlags(tableWidgetTargets_->item(row, 3)->flags() & Qt::ItemIsEditable);
      }

      int i = 0;
      for (const int col : propertyColumns_)
      {
         const QString& option = predictionTargetOptions[i];
         QWidget* checkBoxWidget = new QWidget();
         CustomCheckbox* itemCheckBox = new CustomCheckbox();
         itemCheckBox->setCheckState(target->properties().contains(option) ? Qt::Checked : Qt::Unchecked);

         QHBoxLayout* layoutCheckBox = new QHBoxLayout(checkBoxWidget);
         layoutCheckBox->addWidget(itemCheckBox);
         layoutCheckBox->setAlignment(Qt::AlignCenter);
         layoutCheckBox->setContentsMargins(0,0,0,0);

         connect(itemCheckBox, &CustomCheckbox::stateChanged, [=](int state){emit activePropertyCheckBoxChanged(state, row, option);});

         tableWidgetTargets_->setCellWidget(row, col, checkBoxWidget);

         ++i;
      }
      // Note: this should always be the last column
      QWidget* checkBoxWidget = new QWidget();
      CustomCheckbox* itemCheckBox = new CustomCheckbox();
      itemCheckBox->setCheckState(hasTimeSeriesForPredictionTargets[row] ? Qt::Checked : Qt::Unchecked);

      QHBoxLayout* layoutCheckBox = new QHBoxLayout(checkBoxWidget);
      layoutCheckBox->addWidget(itemCheckBox);
      layoutCheckBox->setAlignment(Qt::AlignCenter);
      layoutCheckBox->setContentsMargins(0,0,0,0);

      connect(itemCheckBox, &CustomCheckbox::stateChanged, [=](int state){emit targetHasTimeSeriesChanged(state, row);});

      tableWidgetTargets_->setCellWidget(row, checkBoxColumnNumber() + predictionTargetOptions.size(), checkBoxWidget);


      ++row;
   }
}

const QTableWidget* PredictionTargetTable::tableWidgetSurfaceTargets() const
{
  return tableWidgetTargets_;
}

const QPushButton* PredictionTargetTable::pushButtonAddSurfaceTarget() const
{
  return pushButtonAddTarget_;
}

const QPushButton* PredictionTargetTable::pushButtonDelSurfaceTarget() const
{
  return pushButtonDelTarget_;
}

const QPushButton* PredictionTargetTable::pushButtonCopySurfaceTarget() const
{
  return pushButtonCopyTarget_;
}

int PredictionTargetTable::checkBoxColumnNumber() const
{
  return checkBoxColumnNumber_;
}

} // namespace ua

} // namespace casaWizard
