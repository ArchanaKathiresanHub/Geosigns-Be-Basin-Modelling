//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include "importWellPopup.h"

namespace casaWizard
{

class ImportWellPopupXlsx : public ImportWellPopup
{
  Q_OBJECT
public:
  explicit ImportWellPopupXlsx(QWidget *parent = nullptr);
};

} // namespace casaWizard
