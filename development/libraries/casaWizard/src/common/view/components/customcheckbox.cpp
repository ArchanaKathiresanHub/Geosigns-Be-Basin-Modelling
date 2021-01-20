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
  initialize();
}


CustomCheckbox::CustomCheckbox(QWidget *parent) :
    QCheckBox(parent)
{
  initialize();
}

void CustomCheckbox::initialize()
{
  setStyleSheet("QCheckBox::indicator { width: 15px; height: 15px}"
                "QCheckBox::indicator:checked { image : url(:/Checked.png) }"
                "QCheckBox::indicator:unchecked { image : url(:/Unchecked.png) }");
}

void CustomCheckbox::enable(bool enabled)
{
  setEnabled(enabled);

  if (enabled)
  {
    setStyleSheet("QCheckBox::indicator { width: 15px; height: 15px}"
                  "QCheckBox::indicator:checked { image : url(:/Checked.png) }"
                  "QCheckBox::indicator:unchecked { image : url(:/Unchecked.png) }");
  }
  else
  {
    setStyleSheet("QCheckBox::indicator { width: 15px; height: 15px}"
                  "QCheckBox::indicator:unchecked { image : url(:/Default_DisabledCheckBox.png) }"
                  "QCheckBox::indicator:checked { image : url(:/Checked_DisabledCheckBox.png) }");
  }
}

} // namespace casaWizard

