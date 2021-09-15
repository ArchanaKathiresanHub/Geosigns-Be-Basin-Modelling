//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "importWellPopup.h"

#include "importWellPropertyTable.h"
#include "view/components/customtitle.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QDialogButtonBox>

namespace casaWizard
{

ImportWellPopup::ImportWellPopup(QWidget *parent) :
  QDialog(parent),
  propertyMappingTable_{new ImportWellPropertyTable(this)}
{
  setWindowTitle("Import options");
  QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

  setFixedWidth(400);

  connect(buttons, SIGNAL(accepted()), this, SIGNAL(acceptedClicked()));
  connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));

  QVBoxLayout* layout = new QVBoxLayout(this);
  layout->addWidget(propertyMappingTable_);
  layout->addWidget(buttons);
}

void ImportWellPopup::updateTable(const QStringList& propertyUserNames, const QStringList& defaultCauldronNames, const QStringList& availableCauldronNames)
{
  propertyMappingTable_->updateTable(propertyUserNames, defaultCauldronNames, availableCauldronNames);
}

const ImportWellPropertyTable* ImportWellPopup::propertyMappingTable() const
{
  return propertyMappingTable_;
}


} // namespace casaWizard
