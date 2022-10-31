//
// Copyright (C) 2012-2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "ThermalMapPlotOptions.h"

#include "view/sharedComponents/customcheckbox.h"
#include "view/sharedComponents/customtitle.h"

#include <QComboBox>
#include <QGridLayout>
#include <QLabel>

namespace casaWizard
{

namespace sac
{

namespace thermal
{

ThermalMapPlotOptions::ThermalMapPlotOptions(QWidget *parent) :
   SacMapPlotOptions(parent)
{
   QHBoxLayout* plotOptionsLayout = new QHBoxLayout(this);

   CustomTitle* plotOptionsLabel = new CustomTitle(" Plot Options ", this);
   plotOptionsLayout->addWidget(plotOptionsLabel);

   plotOptionsLayout->addWidget(new QLabel(" Color map: ", this));
   plotOptionsLayout->addWidget(colorMapSelection());

   plotOptionsLayout->addWidget(new QLabel(" Wells visible: ", this));
   plotOptionsLayout->addWidget(wellsVisible());

   plotOptionsLayout->addWidget(new QLabel(" Fit to page: ", this));
   plotOptionsLayout->addWidget(stretched(), Qt::Alignment(Qt::AlignLeft));
}

} // namespace thermal

} // namespace sac

} // namespace casaWizard
