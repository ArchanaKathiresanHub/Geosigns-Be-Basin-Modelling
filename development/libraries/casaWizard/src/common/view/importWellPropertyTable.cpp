//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "importWellPropertyTable.h"

#include <QComboBox>
#include <QHeaderView>

namespace casaWizard
{

ImportWellPropertyTable::ImportWellPropertyTable(QWidget *parent) :
  QTableWidget(parent)
{
  setColumnCount(2);
  setHorizontalHeaderItem(0, new QTableWidgetItem("User Property Name"));
  setHorizontalHeaderItem(1, new QTableWidgetItem("Cauldron Property Name"));
  auto header = horizontalHeader();
  header->setSectionResizeMode(QHeaderView::ResizeToContents);
}

void ImportWellPropertyTable::updateTable(const QStringList& propertyUserNames, const QStringList& defaultCauldronNames, const QStringList& availableCauldronNames)
{
  clearContents();

  int counter = 0;
  for (const QString& propertyUserName : propertyUserNames)
  {
    setRowCount(counter+1);
    setItem(counter, 0, new QTableWidgetItem(propertyUserName));
    QComboBox* propertySelector = new QComboBox();

    propertySelector->addItems(availableCauldronNames);
    propertySelector->setCurrentText(defaultCauldronNames[counter]);
    setCellWidget(counter, 1, propertySelector);
    counter++;
  }

  update();
}

QMap<QString, QString> ImportWellPropertyTable::getCurrentMapping() const
{
  QMap<QString, QString> mapping;
  for (int i = 0; i < rowCount(); i++)
  {
    mapping[item(i, 0)->text()] = static_cast<QComboBox*>(cellWidget(i,1))->currentText();
  }

  return mapping;
}

} // namespace casaWizard
