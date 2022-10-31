// Plot widget for response surfce quality check
#pragma once

#include "view/plots/plot.h"

namespace casaWizard
{

namespace ua
{

class CorrelationPlot : public Plot
{
  Q_OBJECT

public:
  explicit CorrelationPlot(QWidget* parent = 0);
  void setCorrelations(const QVector<double> &parameterX,
                       const QVector<double> &parameterY,
                       const QVector<double> &parameterZ);

protected:
  void paintEvent(QPaintEvent* event) override;

private:
  bool hasData_;
};

} // namespace ua

} // namespace casaWizard
