//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include <QPushButton>

class EmphasisButton : public QPushButton
{
  Q_OBJECT
public:
  explicit EmphasisButton(const QString& text, QWidget *parent = nullptr);
};

