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
   m_tableWidgetTargets{new QTableWidget(this)},
   m_pushButtonAddTarget{new QPushButton("Add", this)},
   m_pushButtonAddTargetsAtWellLocations{new QPushButton("Add targets at well locations", this)},
   m_pushButtonDelTarget{new QPushButton("Delete", this)},
   m_pushButtonCopyTarget{new QPushButton("Copy", this)},
   m_checkBoxColumnNumber{5},
   m_propertyColumns{}
{
   m_pushButtonAddTargetsAtWellLocations->setEnabled(false);

   m_tableWidgetTargets->setRowCount(0);
   m_tableWidgetTargets->setColumnCount(6);
   m_tableWidgetTargets->horizontalHeader()->setStretchLastSection(true);
   m_tableWidgetTargets->setHorizontalHeaderItem(0, new QTableWidgetItem("Location name"));
   m_tableWidgetTargets->setHorizontalHeaderItem(1, new QTableWidgetItem("x [m]"));
   m_tableWidgetTargets->setHorizontalHeaderItem(2, new QTableWidgetItem("y [m]"));
   m_tableWidgetTargets->setHorizontalHeaderItem(3, new QTableWidgetItem("z [m]"));
   m_tableWidgetTargets->setHorizontalHeaderItem(4, new QTableWidgetItem("Surface"));
   m_tableWidgetTargets->setHorizontalHeaderItem(m_checkBoxColumnNumber, new QTableWidgetItem("Time series"));
   m_tableWidgetTargets->horizontalHeader()->stretchLastSection();

   m_tableWidgetTargets->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
   m_tableWidgetTargets->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);

   QHBoxLayout* horizontalLayoutPredictionButtons = new QHBoxLayout();
   horizontalLayoutPredictionButtons->addWidget(new QWidget(this));
   horizontalLayoutPredictionButtons->addWidget(m_pushButtonAddTargetsAtWellLocations);
   horizontalLayoutPredictionButtons->addWidget(m_pushButtonCopyTarget);
   horizontalLayoutPredictionButtons->addWidget(m_pushButtonAddTarget);
   horizontalLayoutPredictionButtons->addWidget(m_pushButtonDelTarget);
   horizontalLayoutPredictionButtons->setStretch(0, 1);

   QVBoxLayout* verticalLayoutPrediction = new QVBoxLayout(this);
   verticalLayoutPrediction->addWidget(m_tableWidgetTargets);
   verticalLayoutPrediction->addLayout(horizontalLayoutPredictionButtons);
   verticalLayoutPrediction->setStretch(0, 1);
   verticalLayoutPrediction->setContentsMargins(0, 0, 0, 0);
}

void PredictionTargetTable::updateTable(const QVector<const PredictionTarget*> predictionTargets,
                                        const QVector<QString>& predictionTargetOptions,
                                        const QVector<bool>& hasTimeSeriesForPredictionTargets,
                                        const QStringList validLayerNames)
{
   QSignalBlocker blocker(m_tableWidgetTargets);
   m_tableWidgetTargets->clearContents();
   m_tableWidgetTargets->setRowCount(predictionTargets.size());
   for (int i : m_propertyColumns)
   {
      m_tableWidgetTargets->removeColumn(i);
   }
   m_propertyColumns.clear();

   for (const QString& option : predictionTargetOptions)
   {
      m_propertyColumns.append(m_tableWidgetTargets->columnCount() - 1);
      m_tableWidgetTargets->setColumnCount(m_tableWidgetTargets->columnCount() + 1);
      m_tableWidgetTargets->setHorizontalHeaderItem(m_tableWidgetTargets->columnCount() - 2, new QTableWidgetItem(option));
   }
   m_tableWidgetTargets->setHorizontalHeaderItem(m_checkBoxColumnNumber + predictionTargetOptions.size(), new QTableWidgetItem("Time series"));

   int row = 0;
   for (const PredictionTarget* target : predictionTargets)
   {
      TableRowComboBox* layerComboBox = new TableRowComboBox(row, m_tableWidgetTargets);
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


      m_tableWidgetTargets->setItem(row, 0, new QTableWidgetItem(target->locationName()));
      m_tableWidgetTargets->setItem(row, 1, new QTableWidgetItem(QString::number(target->x(), 'f', 0)));
      m_tableWidgetTargets->setItem(row, 2, new QTableWidgetItem(QString::number(target->y(), 'f', 0)));
      m_tableWidgetTargets->setItem(row, 3, new QTableWidgetItem(QString::number(target->z(), 'f', 0)));

      m_tableWidgetTargets->setCellWidget(row, 4, layerComboBox);
      m_tableWidgetTargets->setItem(row, 4, itemLayer);

      if (target->surfaceName() != "")
      {
         m_tableWidgetTargets->item(row, 3)->setFlags(m_tableWidgetTargets->item(row, 3)->flags() & Qt::ItemIsEditable);
      }

      int i = 0;
      for (const int col : m_propertyColumns)
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

         m_tableWidgetTargets->setCellWidget(row, col, checkBoxWidget);

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

      m_tableWidgetTargets->setCellWidget(row, checkBoxColumnNumber() + predictionTargetOptions.size(), checkBoxWidget);

      ++row;
   }
}

void PredictionTargetTable::updateTableAtRow(const PredictionTarget* predictionTarget,
                                             const int row)
{
   QSignalBlocker blocker(m_tableWidgetTargets);

   m_tableWidgetTargets->setItem(row, 0, new QTableWidgetItem(predictionTarget->locationName()));
   m_tableWidgetTargets->setItem(row, 1, new QTableWidgetItem(QString::number(predictionTarget->x(), 'f', 0)));
   m_tableWidgetTargets->setItem(row, 2, new QTableWidgetItem(QString::number(predictionTarget->y(), 'f', 0)));
   m_tableWidgetTargets->setItem(row, 3, new QTableWidgetItem(QString::number(predictionTarget->z(), 'f', 0)));

   if (predictionTarget->surfaceName() != "")
   {
      m_tableWidgetTargets->item(row, 3)->setFlags(m_tableWidgetTargets->item(row, 3)->flags() & Qt::ItemIsEditable);
   }
}

const QTableWidget* PredictionTargetTable::tableWidgetSurfaceTargets() const
{
   return m_tableWidgetTargets;
}

const QPushButton* PredictionTargetTable::pushButtonAddSurfaceTarget() const
{
   return m_pushButtonAddTarget;
}

const QPushButton* PredictionTargetTable::pushButtonAddTargetsAtWellLocations() const
{
   return m_pushButtonAddTargetsAtWellLocations;
}

void PredictionTargetTable::setTargetsAtWellLocationsButtonEnabled(bool state)
{
   m_pushButtonAddTargetsAtWellLocations->setEnabled(state);
}

const QPushButton* PredictionTargetTable::pushButtonDelSurfaceTarget() const
{
   return m_pushButtonDelTarget;
}

const QPushButton* PredictionTargetTable::pushButtonCopySurfaceTarget() const
{
   return m_pushButtonCopyTarget;
}

int PredictionTargetTable::checkBoxColumnNumber() const
{
   return m_checkBoxColumnNumber;
}

} // namespace ua

} // namespace casaWizard
