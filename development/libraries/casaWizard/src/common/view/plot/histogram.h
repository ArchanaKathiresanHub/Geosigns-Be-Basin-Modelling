// Histogram widget
#pragma once

#include "plotBase.h"

#include <QVector>

class QPointF;

namespace casaWizard
{

class Histogram : public PlotBase
{
  Q_OBJECT

public:
  explicit Histogram(QWidget* parent = 0);
  void setData(const QVector<double>& values, int bins = -1);

private:
  void updateMinMaxData() final;
  void drawData(QPainter& painter) final;

  double minValue_;
  double maxValue_;
  QVector<int> dataBins_;
  QVector<double> percentileValues_;
};

} // namespace casaWizard
