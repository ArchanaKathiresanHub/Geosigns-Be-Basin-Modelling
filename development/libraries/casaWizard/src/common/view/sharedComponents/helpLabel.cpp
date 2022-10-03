//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "helpLabel.h"

casaWizard::HelpLabel::HelpLabel(QWidget* parent, const QString& tooltipText) :
  QLabel(parent)
{
  QPixmap helpPixmap = QPixmap(":/Help.png").scaledToHeight(16, Qt::TransformationMode::SmoothTransformation);
  setPixmap(helpPixmap);
  setFixedWidth(helpPixmap.width());
  setToolTip(tooltipText);
}

void casaWizard::HelpLabel::setSize(int pts)
{
   QPixmap helpPixmap = QPixmap(":/Help.png").scaledToHeight(pts, Qt::TransformationMode::SmoothTransformation);
   setPixmap(helpPixmap);
   setFixedWidth(helpPixmap.width());
}
