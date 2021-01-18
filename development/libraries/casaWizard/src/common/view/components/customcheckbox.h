//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include <QCheckBox>

namespace casaWizard
{

class CustomCheckbox : public QCheckBox
{
  Q_OBJECT
public:
  explicit CustomCheckbox(const QString& text, QWidget *parent = nullptr);
  explicit CustomCheckbox(QWidget* parent = nullptr);
};

}
