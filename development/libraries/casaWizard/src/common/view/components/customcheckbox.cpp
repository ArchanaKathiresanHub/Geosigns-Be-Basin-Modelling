//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "customcheckbox.h"

namespace casaWizard
{

CustomCheckbox::CustomCheckbox(const QString& text, QWidget *parent) :
    QCheckBox(text, parent)
{
  setStyleSheet("QCheckBox::indicator { width: 15px; height: 15px}"
                "QCheckBox::indicator:checked { image : url(:/Checked.png) }"
                "QCheckBox::indicator:unchecked { image : url(:/Unchecked.png) }");
}

CustomCheckbox::CustomCheckbox(QWidget *parent) :
    QCheckBox(parent)
{
  setStyleSheet("QCheckBox::indicator { width: 15px; height: 15px}"
                "QCheckBox::indicator:checked { image : url(:/Checked.png) }"
                "QCheckBox::indicator:unchecked { image : url(:/Unchecked.png) }");
}


}

