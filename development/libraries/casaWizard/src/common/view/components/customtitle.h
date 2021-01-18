//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include <QLabel>

class CustomTitle : public QLabel
{
  Q_OBJECT
public:
  explicit CustomTitle(const QString& text, QWidget *parent = nullptr);
  void setPixelSize(int pixelSize);
};

