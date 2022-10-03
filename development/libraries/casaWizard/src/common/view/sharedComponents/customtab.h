//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include <QTabWidget>

namespace casaWizard
{

class CustomTab : public QTabWidget
{
  Q_OBJECT
public:
  explicit CustomTab(QWidget *parent = nullptr);
};

} // namespace casaWizard

