//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "SacMapPlotOptions.h"

#include "view/sharedComponents/customcheckbox.h"

#include <QComboBox>
#include <QLabel>

namespace casaWizard
{

namespace sac
{

SacMapPlotOptions::SacMapPlotOptions(QWidget *parent) :
   QWidget(parent),
   colorMapSelection_{new QComboBox(this)},
   stretched_{new CustomCheckbox(this)},
   wellsVisible_{new CustomCheckbox(this)}
{
   colorMapSelection_->insertItems(0, {"Viridis", "Gray scale", "Rainbow"});
   wellsVisible_->setCheckState(Qt::CheckState::Checked);
   stretched_->setCheckState(Qt::CheckState::Unchecked);
}

QComboBox* SacMapPlotOptions::colorMapSelection() const
{
   return colorMapSelection_;
}

CustomCheckbox* SacMapPlotOptions::stretched() const
{
   return stretched_;
}

CustomCheckbox* SacMapPlotOptions::wellsVisible() const
{
   return wellsVisible_;
}

} //namespace sac

} //namespace casaWizard
