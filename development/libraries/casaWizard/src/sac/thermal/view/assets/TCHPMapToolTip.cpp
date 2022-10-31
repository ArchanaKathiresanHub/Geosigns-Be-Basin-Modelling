//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "TCHPMapToolTip.h"

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

namespace thermal
{

TCHPMapToolTip::TCHPMapToolTip(QWidget *parent) :
   SacMapToolTip(parent)
{
   setTotalLayout();
}

void TCHPMapToolTip::setTotalLayout()
{
  QVBoxLayout* totalLayout = new QVBoxLayout(this);
  totalLayout->addWidget(header());
  totalLayout->addWidget(infoLabel());
  totalLayout->addWidget(valueLabel());
  totalLayout->setMargin(0);
  totalLayout->setSpacing(0);

  setFixedWidth(100);
  QPalette pal = palette();
  pal.setColor(QPalette::Base, QColor(255,255,255, 100));
  setAutoFillBackground(true);
  setPalette(pal);
}

void TCHPMapToolTip::setTCHP(const double& TCHP, const QString& wellName)
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
   valueLabel()->setText("Value: " + QString::number(TCHP, 'f', 2));

   update();
   overlay()->setFixedSize(size());
   overlay()->move(QPoint(0,0));
   overlay()->update();
}

} // namespace thermal

} // namespace sac

} // namespace casaWizard
