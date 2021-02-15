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
#include "plot/grid2Dview.h"

#include <QLabel>
#include <QLayout>

namespace casaWizard
{

namespace sac
{

Grid2DPlot::Grid2DPlot(const ColorMap& colormap, QWidget *parent) :
  QWidget(parent),
  colorBar_(new ColorBar(colormap, this)),
  grid2dView_(new Grid2DView(colormap, this)),
  title_(new CustomTitle("Test Title", this))
{
  QHBoxLayout* gridAndColorBar = new QHBoxLayout();
  gridAndColorBar->setMargin(0);
  colorBar_->setFixedWidth(100);

  gridAndColorBar->addWidget(grid2dView_);
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

Grid2DView* Grid2DPlot::grid2DView() const
{
  return grid2dView_;
}

void Grid2DPlot::updateColorBar()
{
  colorBar_->setRange(grid2dView_->getValueRange());
}

void Grid2DPlot::setTitle(const QString& lithoType, int id)
{
  title_->setText("Lithotype " + QString::number(id + 1) + ": " + lithoType);
}

} // namespace sac

} // namespace casaWizard
