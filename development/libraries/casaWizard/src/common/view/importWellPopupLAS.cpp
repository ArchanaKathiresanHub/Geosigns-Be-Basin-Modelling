//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "importWellPopupLAS.h"

#include "view/components/customcheckbox.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QLabel>
#include <QTableWidget>
#include <QVBoxLayout>

namespace casaWizard
{

ImportWellPopupLAS::ImportWellPopupLAS(QWidget *parent) :
  ImportWellPopup(parent),
  correctForElevation_{new CustomCheckbox("Elevation correction", this)},
  depthName_{new QLabel(this)}
{
  setFixedWidth(700);
  QVBoxLayout* layout = new QVBoxLayout(this);
  layout->addWidget(propertyMappingTable_);
  layout->addWidget(correctForElevation_);
  layout->addWidget(depthName_);
  layout->addWidget(buttons_);

  connect(this, SIGNAL(propertySelectorChanged(const QString, int)), this, SLOT(slotComboboxChanged(const QString&, int)));
}

bool ImportWellPopupLAS::correctForElevation() const
{
  return correctForElevation_->checkState() == Qt::Checked;
}

void ImportWellPopupLAS::updatePropertyTableWithUnits(const QStringList& propertyUserNames, const QStringList& defaultCauldronNames, const QStringList& availableCauldronNames,
                                     const QStringList& units, const QVector<double>& conversionFactors)
{
  updateTable(propertyUserNames, defaultCauldronNames, availableCauldronNames);
  propertyMappingTable_->setColumnCount(4);
  propertyMappingTable_->setHorizontalHeaderItem(2, new QTableWidgetItem("Unit"));
  propertyMappingTable_->setHorizontalHeaderItem(3, new QTableWidgetItem("Conversion factor"));
  propertyMappingTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

  for (int row = 0; row < propertyMappingTable_->rowCount(); row++)
  {
    propertyMappingTable_->setItem(row, 2, new QTableWidgetItem(units[row]));

    if (conversionFactors.empty() || conversionFactors[row] < 0)
    {
      propertyMappingTable_->setItem(row, 3, new QTableWidgetItem());
    }
    else
    {
      propertyMappingTable_->setItem(row, 3, new QTableWidgetItem(QString::number(conversionFactors[row])));
    }

    QComboBox* propertySelection = dynamic_cast<QComboBox*>(propertyMappingTable_->cellWidget(row,1));
    if (propertySelection)
    {
      connect(propertySelection, &QComboBox::currentTextChanged, [=](const QString& text){emit propertySelectorChanged(text, row);});
    }


  }

  update();
}

QMap<QString, double> ImportWellPopupLAS::getUnitConversions() const
{
  QMap<QString, double> mapping;
  for (int i = 0; i < propertyMappingTable_->rowCount(); i++)
  {
    double conversion = propertyMappingTable_->item(i, 3)->text() == "" ? 1.0 : propertyMappingTable_->item(i, 3)->text().toDouble();
    mapping[propertyMappingTable_->item(i, 0)->text()] = conversion;
  }

  return mapping;
}

void ImportWellPopupLAS::setElevationInfo(const double elevationCorrection, const QString& elevationCorrectionUnit)
{
  if (elevationCorrection == 0.0)
  {
    correctForElevation_->setDisabled(true);
  }
  else
  {
    correctForElevation_->setText("Correct for Elevation: " + QString::number(elevationCorrection) + elevationCorrectionUnit);
  }
}

void ImportWellPopupLAS::slotComboboxChanged(const QString& text, int row)
{
  QSignalBlocker blocker(this);
  if (text == "Depth")
  {
    for (int i = 0; i < propertyMappingTable_->rowCount(); i++)
    {
      QComboBox* propertySelection = dynamic_cast<QComboBox*>(propertyMappingTable_->cellWidget(i,1));
      if (i != row && propertySelection->currentText() == "Depth")
      {
        propertySelection->setCurrentText("Unknown");
      }
    }
  }
}


} // namespace casaWizard

