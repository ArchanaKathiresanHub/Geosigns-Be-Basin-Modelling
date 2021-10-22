//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "importWellPopupXlsx.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QTableWidget>
#include <QVBoxLayout>

namespace casaWizard
{

ImportWellPopupXlsx::ImportWellPopupXlsx(QWidget* parent) :
  ImportWellPopup(parent)
{
  setFixedWidth(400);
  QVBoxLayout* layout = new QVBoxLayout(this);
  layout->addWidget(propertyMappingTable_);
  layout->addWidget(buttons_);
}

} // namespace casaWizard
