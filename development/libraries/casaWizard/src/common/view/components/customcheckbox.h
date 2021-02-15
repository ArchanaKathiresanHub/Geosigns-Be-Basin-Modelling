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
  void initialize();

public:
  explicit CustomCheckbox(const QString& text, QWidget *parent = nullptr);
  explicit CustomCheckbox(QWidget* parent = nullptr);

  void enable(bool enabled);
};

}
