//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include <QLabel>

namespace casaWizard
{

class HelpLabel : public QLabel
{
  Q_OBJECT
public:
  HelpLabel(QWidget *parent = nullptr, const QString& tooltipText = "");
  void setSize(int pts);
};

}
