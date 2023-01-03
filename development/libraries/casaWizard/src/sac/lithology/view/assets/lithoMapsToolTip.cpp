//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "lithoMapsToolTip.h"

#include "view/lithoTypeBackgroundMapping.h"

#include <QChart>
#include <QChartView>
#include <QLabel>
#include <QPieSeries>
#include <QPieSlice>
#include <QVBoxLayout>

using namespace QtCharts;

namespace casaWizard
{

namespace sac
{

namespace lithology
{

LithoMapsToolTip::LithoMapsToolTip(QWidget *parent) :
  SacMapToolTip(parent),
  pieChart_{new QChart()},
  chartView_{new QChartView(pieChart_, this)},
  series_{new QPieSeries(this)},
  lithoNames_{}
{
  initializeChart();

  setTotalLayout();
}

void LithoMapsToolTip::initializeChart()
{
  initializePieChart();
  initializeChartView();
}

void LithoMapsToolTip::initializePieChart()
{
  pieChart_->addSeries(series_);
  pieChart_->legend()->setVisible(false);
  pieChart_->setPlotAreaBackgroundVisible(true);
  pieChart_->setBackgroundBrush(QBrush(QColor(0, 0, 0, 0)));
  pieChart_->setMargins(QMargins(-18,-16,-18,-18));
}

void LithoMapsToolTip::initializeChartView()
{
  chartView_->setContentsMargins(0, 0, 0, 0);
  chartView_->setRenderHint(QPainter::Antialiasing, true);
  chartView_->setAttribute(Qt::WA_TranslucentBackground);
  chartView_->setStyleSheet("background:transparent;");
}

void LithoMapsToolTip::setTotalLayout()
{
  QVBoxLayout* totalLayout = new QVBoxLayout(this);
  totalLayout->addWidget(header());
  totalLayout->addWidget(infoLabel());
  totalLayout->addWidget(chartView_);
  totalLayout->addWidget(valueLabel());
  totalLayout->setMargin(0);
  totalLayout->setSpacing(0);

  setFixedWidth(100);
  QPalette pal = palette();
  pal.setColor(QPalette::Base, QColor(255,255,255, 100));
  setAutoFillBackground(true);
  setPalette(pal);
}


void LithoMapsToolTip::setLithofractions(const std::vector<double>& lithofractions, const QString& wellName, const int activePlot)
{

  header()->setText("(" + QString::number(domainPosition().x(),'f', 1) + ", " + QString::number(domainPosition().y(),'f', 1) + ")");
  if (wellName == "")
  {
    infoLabel()->setText("Map Value");
    infoLabel()->update();
  }
  else
  {
    infoLabel()->setText(wellName);
    infoLabel()->update();
  }
  valueLabel()->setText("Value: " + QString::number(lithofractions[activePlot], 'f', 1) + "%");
  series_->clear();
  series_->append("Litho 1", lithofractions[0]);
  series_->append("Litho 2", lithofractions[1]);
  series_->append("Litho 3", lithofractions[2]);

  if (!lithoNames_.empty())
  {
    updatePieSliceColors();
  }

  QPieSlice* activeSlice = series_->slices().at(activePlot);
  activeSlice->setExploded(true);
  QPen pen(Qt::red);
  pen.setWidth(2);
  activeSlice->setPen(pen);
  update();
  overlay()->setFixedSize(size());
  overlay()->move(QPoint(0,0));
  overlay()->update();
}


void LithoMapsToolTip::updatePieSliceColors()
{
  for (int i = 0; i < lithoNames_.size(); i++)
  {
    QPieSlice* slice = series_->slices().at(i);
    slice->setPen(QPen(Qt::black));
    QBrush brush = slice->brush();
    LithoTypeBackgroundMapping::getBackgroundBrush(lithoNames_[i], brush);
    slice->setBrush(brush);
  }
}

void LithoMapsToolTip::setLithoNames(const QStringList& lithoNames)
{
  lithoNames_ = lithoNames;
  if (!series_->slices().empty())
  {
    updatePieSliceColors();
  }
}

} // namespace lithology

} // namespace sac

} // namespace casaWizard
