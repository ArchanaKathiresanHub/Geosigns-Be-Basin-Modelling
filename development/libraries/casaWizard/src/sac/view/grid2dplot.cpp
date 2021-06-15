//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "grid2dplot.h"

#include "colorbar.h"
#include "../common/view/components/customtitle.h"
#include "plot/lithoPercent2Dview.h"

#include <QLabel>
#include <QLayout>

namespace casaWizard
{

namespace sac
{

Grid2DPlot::Grid2DPlot(const ColorMap& colormap, QWidget *parent) :
  QWidget(parent),
  colorBar_(new ColorBar(colormap, this)),
  lithoPercent2dView_(new LithoPercent2DView(colormap, this)),
  title_(new CustomTitle("Test Title", this))
{
  QHBoxLayout* gridAndColorBar = new QHBoxLayout();
  gridAndColorBar->setMargin(0);
  colorBar_->setFixedWidth(100);

  gridAndColorBar->addWidget(lithoPercent2dView_);
  gridAndColorBar->addWidget(colorBar_);

  setTitleLayout();

  QVBoxLayout* plotLayout = new QVBoxLayout(this);
  plotLayout->addWidget(title_);
  plotLayout->addLayout(gridAndColorBar);
  plotLayout->setMargin(0);

  QSizePolicy retainSize = sizePolicy();
  retainSize.setRetainSizeWhenHidden(true);
  setSizePolicy(retainSize);

  setLayout(plotLayout);
}

void Grid2DPlot::setTitleLayout()
{
  title_->setPixelSize(13);
  title_->setAlignment(Qt::Alignment(Qt::AlignmentFlag::AlignCenter));
}

LithoPercent2DView* Grid2DPlot::lithoPercent2DView() const
{
  return lithoPercent2dView_;
}

void Grid2DPlot::updateColorBar()
{
  colorBar_->setRange(lithoPercent2dView_->getValueRange());
}

void Grid2DPlot::hideColorBar()
{
  colorBar_->hide();
}

void Grid2DPlot::showColorBar()
{
  colorBar_->show();
}

void Grid2DPlot::setTitle(const QString& lithoType, int id)
{
  title_->setText("Lithotype " + QString::number(id + 1) + ": " + lithoType);
}

} // namespace sac

} // namespace casaWizard
