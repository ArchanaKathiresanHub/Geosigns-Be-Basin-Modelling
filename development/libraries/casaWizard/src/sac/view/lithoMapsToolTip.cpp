//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "lithoMapsToolTip.h"

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

LithoMapsToolTip::LithoMapsToolTip(QWidget *parent) :
  QWidget(parent),
  domainPosition_(0,0),
  header_{new QLabel(this)},
  valueLabel_{new QLabel(this)},
  pieChart_{new QChart()},
  chartView_{new QChartView(pieChart_, this)},
  series_{new QPieSeries(this)},
  overlay_{new LithoMapsToolTipOverlay(this)}
{
  initializeChart();
  initializeHeader();
  initializeValueLabel();

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

void LithoMapsToolTip::initializeHeader()
{
  header_->setFixedHeight(20);
  header_->setStyleSheet("QLabel { background-color : rgb(74,74,74); color : rgb(255,255,255) ; font-weight : bold; font-size : 11px}");
  header_->setAlignment(Qt::AlignCenter);
}

void LithoMapsToolTip::initializeValueLabel()
{
  valueLabel_->setStyleSheet("QLabel { color : rgb(0,0,0) ; font-weight : bold; font-size : 11px}");
  valueLabel_->setAlignment(Qt::AlignCenter);
  valueLabel_->setFixedHeight(15);
}

void LithoMapsToolTip::setTotalLayout()
{
  QVBoxLayout* totalLayout = new QVBoxLayout(this);
  totalLayout->addWidget(header_);
  totalLayout->addWidget(chartView_);
  totalLayout->addWidget(valueLabel_);
  totalLayout->setMargin(0);
  totalLayout->setSpacing(0);

  setFixedWidth(100);
  QPalette pal = palette();
  pal.setColor(QPalette::Base, QColor(255,255,255, 100));
  setAutoFillBackground(true);
  setPalette(pal);
}

void LithoMapsToolTip::setLithofractions(const std::vector<double>& lithofractions, const int activePlot)
{
  series_->clear();
  series_->append("Litho 1", lithofractions[0]);
  series_->append("Litho 2", lithofractions[1]);
  series_->append("Litho 3", lithofractions[2]);

  QPieSlice* slice = series_->slices().at(activePlot);
  slice->setExploded(true);
  slice->setPen(QPen(Qt::red));

  header_->setText("(" + QString::number(domainPosition_.x(),'f', 1) + ", " + QString::number(domainPosition_.y(),'f', 1) + ")");
  valueLabel_->setText("Value: " + QString::number(lithofractions[activePlot], 'f', 1) + "%");
  overlay_->setFixedSize(size());
  overlay_->move(QPoint(0,0));
  overlay_->update();
}

void LithoMapsToolTip::setCorner(const bool movedX, const bool movedY)
{
  if (movedX)
  {
    if (movedY)
    {
      overlay_->setCornerPosition(bottomRight);
    }
    else
    {
      overlay_->setCornerPosition(topRight);
    }
  }
  else
  {
    if (movedY)
    {
      overlay_->setCornerPosition(bottomLeft);
    }
    else
      {
        overlay_->setCornerPosition(topLeft);
      }
  }

  overlay_->update();
}

void LithoMapsToolTip::setDomainPosition(const QPointF& domainPosition)
{
  domainPosition_ = domainPosition;
}

LithoMapsToolTipOverlay::LithoMapsToolTipOverlay(QWidget* parent) :
  QWidget(parent),
  cornerPosition_{topLeft}
{

}

void LithoMapsToolTipOverlay::paintEvent(QPaintEvent* /*event*/)
{
  QPoint position;
  switch (cornerPosition_)
  {
    case topLeft :
      position = QPoint(0,0);
      break;
    case topRight :
      position = QPoint(width(), 0);
      break;
    case bottomLeft:
      position = QPoint(0, height());
      break;
    case bottomRight:
      position = QPoint(width(),height());
      break;
  }

  QPainter painter(this);
  painter.setPen(Qt::red);
  painter.setBrush(Qt::red);
  painter.setRenderHint(QPainter::Antialiasing, true);
  painter.drawEllipse(position, 7, 7);
}

} // namespace sac

} // namespace casaWizard
