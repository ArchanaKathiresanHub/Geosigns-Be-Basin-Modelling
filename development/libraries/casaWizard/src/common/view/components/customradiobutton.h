//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include <QRadioButton>

class CustomRadioButton : public QRadioButton
{
  Q_OBJECT
public:
    explicit CustomRadioButton(const QString &text, QWidget *parent = nullptr);
};

