//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "emphasisbutton.h"

EmphasisButton::EmphasisButton(const QString& text, QWidget *parent) : QPushButton(text, parent)
{
  setStyleSheet("background-color: rgb(12, 124, 199); color: rgb(255, 255, 255);");
  QFont font;
  font.setBold(true);
  setFont(font);
}
