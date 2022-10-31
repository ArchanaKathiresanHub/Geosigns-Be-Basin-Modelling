//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "SacMapToolTip.h"

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

SacMapToolTip::SacMapToolTip(QWidget *parent) :
  QWidget(parent),
  domainPosition_(0,0),
  header_{new QLabel(this)},
  valueLabel_{new QLabel(this)},
  infoLabel_{new QLabel(this)},
  overlay_{new SacMapToolTipOverlay(this)}
{
   initializeHeader();
   initializeValueLabel();
   initializeWellLabel();
}

void SacMapToolTip::initializeHeader()
{
  header_->setFixedHeight(20);
  header_->setStyleSheet("QLabel { background-color : rgb(74,74,74); color : rgb(255,255,255) ; font-weight : bold; font-size : 11px}");
  header_->setAlignment(Qt::AlignCenter);
}

void SacMapToolTip::initializeValueLabel()
{
  valueLabel_->setStyleSheet("QLabel { color : rgb(0,0,0) ; font-weight : bold; font-size : 11px}");
  valueLabel_->setAlignment(Qt::AlignCenter);
  valueLabel_->setFixedHeight(15);
}

void SacMapToolTip::initializeWellLabel()
{
  infoLabel_->setStyleSheet("QLabel { color : rgb(0,0,0) ; font-weight : bold; font-size : 11px}");
  infoLabel_->setAlignment(Qt::AlignCenter);
  infoLabel_->setFixedHeight(15);
}


void SacMapToolTip::setCorner(const bool movedX, const bool movedY)
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


const QPointF& SacMapToolTip::domainPosition() const
{
   return domainPosition_;
}

QLabel* SacMapToolTip::header() const
{
   return header_;
}

QLabel* SacMapToolTip::valueLabel() const
{
   return valueLabel_;
}

QLabel* SacMapToolTip::infoLabel() const
{
   return infoLabel_;
}

SacMapToolTipOverlay* SacMapToolTip::overlay() const
{
   return overlay_;
}

void SacMapToolTip::setDomainPosition(const QPointF& domainPosition)
{
  domainPosition_ = domainPosition;
}

SacMapToolTipOverlay::SacMapToolTipOverlay(QWidget* parent) :
  QWidget(parent),
  cornerPosition_{topLeft}
{

}

void SacMapToolTipOverlay::paintEvent(QPaintEvent* /*event*/)
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
  painter.setPen(QColor(12, 124, 199));
  painter.setBrush(QColor(12, 124, 199));
  painter.setRenderHint(QPainter::Antialiasing, true);
  painter.drawEllipse(position, 7, 7);
}

} // namespace sac

} // namespace casaWizard
