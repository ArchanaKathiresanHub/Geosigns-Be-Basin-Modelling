// Plot widget for response surfce quality check
#pragma once

#include "view/plot/plot.h"

namespace casaWizard
{

namespace ua
{

struct TargetQC;

class QCPlot : public Plot
{
  Q_OBJECT

public:
  explicit QCPlot(QWidget* parent = 0);
  void setTargetQC(const TargetQC& targetQC);
  void reset();

protected:
  void paintEvent(QPaintEvent* event) override;
  void resizeEvent(QResizeEvent* event) override;

private:
  double targetValue_;
  double targetStandardDeviation_;
  bool hasCalibrationValue_;
  bool hasData_;
};

} // namespace ua

} // namespace casaWizard
