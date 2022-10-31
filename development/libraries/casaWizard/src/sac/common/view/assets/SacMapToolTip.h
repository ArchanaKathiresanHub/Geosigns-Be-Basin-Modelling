//
// Copyright (C) 2012-2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include <QWidget>

namespace QtCharts
{
  class QChart;
  class QChartView;
  class QPieSeries;
}

class QLabel;

namespace casaWizard
{

namespace sac
{

enum CornerPosition
{
  topLeft,
  topRight,
  bottomLeft,
  bottomRight
};


class SacMapToolTipOverlay : public QWidget
{
  Q_OBJECT
public:
  explicit SacMapToolTipOverlay(QWidget *parent = nullptr);

  CornerPosition cornerPosition_;
  void setCornerPosition(const CornerPosition& cornerPosition) {cornerPosition_ = cornerPosition;}
  void paintEvent(QPaintEvent *event) override;
};

class SacMapToolTip : public QWidget
{
  Q_OBJECT
public:
  explicit SacMapToolTip(QWidget *parent = nullptr);
  void setDomainPosition(const QPointF& domainPosition);
  void setCorner(const bool movedX, const bool movedY);

protected:
  const QPointF& domainPosition() const;
  QLabel* header() const;
  QLabel* valueLabel() const;
  QLabel* infoLabel() const;
  SacMapToolTipOverlay* overlay() const;

private:
  QPointF domainPosition_;
  QLabel* header_;
  QLabel* valueLabel_;
  QLabel* infoLabel_;
  SacMapToolTipOverlay* overlay_;

  void initializeHeader();
  void initializeValueLabel();
  void initializeWellLabel();
};

} // namespace sac

} // namespace casaWizard
