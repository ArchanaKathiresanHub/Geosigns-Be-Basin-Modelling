//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "importWellPopup.h"

#include "view/components/customtitle.h"
#include "view/components/customComboBox.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QHeaderView>
#include "QTableWidget"

namespace casaWizard
{

ImportWellPopup::ImportWellPopup(QWidget *parent) :
  QDialog(parent),
  propertyMappingTable_{new QTableWidget(this)}
{
  setWindowTitle("Import options");
  buttons_ = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

  connect(buttons_, SIGNAL(accepted()), this, SIGNAL(acceptedClicked()));
  connect(buttons_, SIGNAL(rejected()), this, SLOT(reject()));
}

void ImportWellPopup::updateTable(const QStringList& propertyUserNames, const QStringList& defaultCauldronNames, const QStringList& availableCauldronNames)
{
  propertyMappingTable_->clearContents();
  propertyMappingTable_->setColumnCount(2);
  propertyMappingTable_->setHorizontalHeaderItem(0, new QTableWidgetItem("User Property Name"));
  propertyMappingTable_->setHorizontalHeaderItem(1, new QTableWidgetItem("Cauldron Property Name"));
  propertyMappingTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

  int counter = 0;
  for (const QString& propertyUserName : propertyUserNames)
  {
    propertyMappingTable_->setRowCount(counter+1);
    propertyMappingTable_->setItem(counter, 0, new QTableWidgetItem(propertyUserName));

    QComboBox* propertySelector = new CustomComboBox();

    propertySelector->addItems(availableCauldronNames);
    propertySelector->setCurrentText(defaultCauldronNames[counter]);
    propertyMappingTable_->setCellWidget(counter, 1, propertySelector);
    counter++;
  }

  update();
}

QMap<QString, QString> ImportWellPopup::getCurrentMapping() const
{
  QMap<QString, QString> mapping;
  for (int i = 0; i < propertyMappingTable_->rowCount(); i++)
  {
    QString cauldronName = static_cast<QComboBox*>(propertyMappingTable_->cellWidget(i,1))->currentText();
    mapping[propertyMappingTable_->item(i, 0)->text()] = cauldronName;
  }

  return mapping;
}

const QTableWidget* ImportWellPopup::propertyMappingTable() const
{
  return propertyMappingTable_;
}


} // namespace casaWizard
