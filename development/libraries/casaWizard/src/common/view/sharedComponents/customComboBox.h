//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include <QComboBox>

class CustomComboBox : public QComboBox
{
  Q_OBJECT
public:
  explicit CustomComboBox(QWidget *parent = nullptr);
  void wheelEvent(QWheelEvent *event) override;
};

