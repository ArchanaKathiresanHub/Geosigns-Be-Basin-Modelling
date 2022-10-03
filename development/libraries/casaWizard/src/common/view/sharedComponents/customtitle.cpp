//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "customtitle.h"

CustomTitle::CustomTitle(const QString& text, QWidget *parent) : QLabel(text, parent)
{
  QFont font;
  font.setBold(true);
  setFont(font);
}

void CustomTitle::setPixelSize(int pixelSize)
{
  QFont titleFont = font();
  titleFont.setPixelSize(pixelSize);
  setFont(titleFont);
}
