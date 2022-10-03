//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "userPropertyChoicePopup.h"

#include "view/sharedComponents/customtitle.h"
#include "view/sharedComponents/customcheckbox.h"

#include <QDialogButtonBox>
#include <QLabel>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QHeaderView>

namespace casaWizard
{

UserPropertyChoicePopup::UserPropertyChoicePopup(QWidget *parent) :
  QDialog(parent),
  propertyTable_{new QTableWidget(this)}
{
  setWindowTitle("Select properties");
  QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

  setFixedWidth(400);

  connect(buttons, SIGNAL(accepted()), this, SIGNAL(acceptedClicked()));
  connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));

  QVBoxLayout* layout = new QVBoxLayout(this);
  layout->addWidget(propertyTable_);
  layout->addWidget(buttons);
}

void UserPropertyChoicePopup::updateTable(const QStringList& propertyUserNames)
{
  propertyTable_->clear();  
  propertyTable_->setRowCount(propertyUserNames.size());
  propertyTable_->setColumnCount(2);
  for (int i=0; i<propertyUserNames.size(); ++i)
  {
    QWidget* checkBoxWidget = new QWidget();
    CustomCheckbox* itemCheckBox = new CustomCheckbox();
    itemCheckBox->setCheckState(Qt::Checked);

    QHBoxLayout* layoutCheckBox = new QHBoxLayout(checkBoxWidget);
    layoutCheckBox->addWidget(itemCheckBox);
    layoutCheckBox->setAlignment(Qt::AlignCenter);
    layoutCheckBox->setContentsMargins(0,0,0,0);

    propertyTable_->setCellWidget(i, 0, checkBoxWidget);
    QTableWidgetItem* nameItem = new QTableWidgetItem(propertyUserNames[i]);
    propertyTable_->setItem(i,1, nameItem);    
  }
  propertyTable_->setHorizontalHeaderLabels({"Select", "Property"});
  propertyTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

QStringList UserPropertyChoicePopup::selectedProperties() const
{
  QStringList properties;
  for (int i=0; i<propertyTable_->rowCount(); ++i)
  {
    const QString property = getPropertyIfChecked(i);
    if (!property.isEmpty())
    {      
      properties.push_back(property);
    }
  }
  return properties;
}

QTableWidget* UserPropertyChoicePopup::propertyTable() const
{
  return propertyTable_;
}

QString UserPropertyChoicePopup::getPropertyIfChecked(int row) const
{
  const CustomCheckbox* itemCheckBox = static_cast<CustomCheckbox*>(propertyTable_->cellWidget(row, 0)->children()[1]);
  if (itemCheckBox->isChecked())
  {
    return propertyTable_->item(row, 1)->text();
  }
  return "";
}

} // namespace casaWizard
