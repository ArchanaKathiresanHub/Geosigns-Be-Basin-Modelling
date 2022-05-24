//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "MapPlotOptions.h"

#include "view/components/customcheckbox.h"
#include "view/components/customtitle.h"

#include <QComboBox>
#include <QGridLayout>
#include <QLabel>

namespace casaWizard
{

namespace sac
{


MapPlotOptions::MapPlotOptions(QWidget *parent) :
   QWidget(parent),
   percentageRange_(new QComboBox(this)),
   colorMapSelection_{new QComboBox(this)},
   layerSelection_{new QComboBox(this)},
   lithotypeSelection_{new QComboBox(this)},
   lithoSelectionLabel_{new QLabel(" Selected Lithotype: ", this)},
   singleMapLayout_{new CustomCheckbox(this)},
   stretched_{new CustomCheckbox(this)},
   wellsVisible_{new CustomCheckbox(this)}
{
   colorMapSelection_->insertItems(0, {"Viridis", "Gray scale", "Rainbow"});
   wellsVisible_->setCheckState(Qt::CheckState::Checked);
   stretched_->setCheckState(Qt::CheckState::Unchecked);
   singleMapLayout_->setCheckState(Qt::CheckState::Unchecked);
   percentageRange_->insertItems(0, {"Variable", "Fixed between 0 and 100", "Fixed between global min and max"});

   lithotypeSelection_->hide();
   lithoSelectionLabel_->hide();

   QGridLayout* plotOptionsLayout = new QGridLayout(this);

   CustomTitle* plotOptionsLabel = new CustomTitle(" Plot Options ", this);
   plotOptionsLayout->addWidget(plotOptionsLabel, 0, 0);
   plotOptionsLayout->addWidget(new QLabel(" Selected layer: ", this), 1, 0);
   plotOptionsLayout->addWidget(layerSelection_, 1, 1);
   plotOptionsLayout->addWidget(new QLabel(" Color map: ", this), 2, 0);
   plotOptionsLayout->addWidget(colorMapSelection_, 2, 1);
   plotOptionsLayout->addWidget(new QLabel(" Percentage range: ", this), 3, 0);
   plotOptionsLayout->addWidget(percentageRange_, 3, 1);

   plotOptionsLayout->addWidget(new QLabel(" Wells visible: ", this), 4, 0);
   plotOptionsLayout->addWidget(wellsVisible_, 4, 1);

   plotOptionsLayout->addWidget(new QLabel(" Fit to page: ", this), 5, 0);
   plotOptionsLayout->addWidget(stretched_, 5, 1);

   plotOptionsLayout->addWidget(new QLabel(" Single map layout: ", this), 6, 0);
   plotOptionsLayout->addWidget(singleMapLayout_, 6, 1);

   plotOptionsLayout->addWidget(lithoSelectionLabel_, 7, 0);
   plotOptionsLayout->addWidget(lithotypeSelection_, 7, 1);

   setMaximumWidth(400);
   setMaximumHeight(190);

   connect(singleMapLayout_, SIGNAL(stateChanged(int)), this, SLOT(slotUpdateSingleMapLayout(int)));
}

QComboBox* MapPlotOptions::percentageRange() const
{
   return percentageRange_;
}

QComboBox* MapPlotOptions::colorMapSelection() const
{
   return colorMapSelection_;
}

QComboBox* MapPlotOptions::layerSelection() const
{
   return layerSelection_;
}

QComboBox* MapPlotOptions::lithotypeSelection() const
{
   return lithotypeSelection_;
}

CustomCheckbox* MapPlotOptions::singleMapLayout() const
{
   return singleMapLayout_;
}

CustomCheckbox* MapPlotOptions::stretched() const
{
   return stretched_;
}

CustomCheckbox* MapPlotOptions::wellsVisible() const
{
   return wellsVisible_;
}

void MapPlotOptions::slotUpdateSingleMapLayout(int state)
{
   lithotypeSelection_->setVisible(state == Qt::CheckState::Checked);
   lithoSelectionLabel_->setVisible(state == Qt::CheckState::Checked);
}

}

}
