//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "customradiobutton.h"

CustomRadioButton::CustomRadioButton(const QString& text, QWidget* parent) :
    QRadioButton(text, parent)
{
  setStyleSheet("QRadioButton::indicator { width: 15px; height: 15px}"
                "QRadioButton::indicator:checked { image : url(:/SelectedRadioButton.png) }"
                "QRadioButton::indicator:unchecked { image : url(:/DefaultRadioButton.png) }");
}
