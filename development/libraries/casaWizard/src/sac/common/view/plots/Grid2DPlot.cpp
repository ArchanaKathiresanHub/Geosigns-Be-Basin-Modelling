//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "Grid2DPlot.h"

#include "view/ColorBar.h"
#include "view/sharedComponents/customtitle.h"
#include "view/plots/wellBirdsView.h"

#include <QLabel>
#include <QLayout>

namespace casaWizard
{

namespace sac
{

Grid2DPlot::Grid2DPlot(const ColorMap& colormap, QWidget *parent) :
   QWidget(parent),
   m_colorBar(new ColorBar(ColorBar::right, this)),
   m_title(new CustomTitle("", this)),
   m_layout(new QGridLayout(this))
{
   m_colorBar->setDecimals(2);
   m_colorBar->setColorMap(colormap);
   m_colorBar->setFixedWidth(150);
}

void Grid2DPlot::initGrid2DPlot()
{
   m_layout->setMargin(0);

   m_layout->addWidget(get2DView(), 1, 0);
   m_layout->addWidget(m_colorBar, 1, 1);

   setTitleLayout();
   m_layout->addWidget(m_title, 0, 0, 1, 2);

   QSizePolicy retainSize = sizePolicy();
   retainSize.setRetainSizeWhenHidden(true);
   setSizePolicy(retainSize);

   setLayout(m_layout);
}

CustomTitle* Grid2DPlot::title() const
{
   return m_title;
}

void Grid2DPlot::setTitleLayout()
{
   m_title->setPixelSize(13);
   m_title->setAlignment(Qt::Alignment(Qt::AlignmentFlag::AlignCenter));
}

void Grid2DPlot::updateColorBar(const int& maxDecimals)
{
   std::pair<double, double> range = get2DView()->getValueRange();
   m_colorBar->setRange(range.first, range.second);
   m_colorBar->setDecimals(maxDecimals);
}

void Grid2DPlot::setColorBarMap(const ColorMap& map)
{
   m_colorBar->setColorMap(map);
}

void Grid2DPlot::hideColorBar()
{
   m_colorBar->hide();
}

void Grid2DPlot::showColorBar()
{
   m_colorBar->show();
}

void Grid2DPlot::refresh()
{
   update();
   get2DView()->update();
}

void Grid2DPlot::setTitle(const QString& title)
{
   m_title->setText(title);
}



} // namespace sac

} // namespace casaWizard
