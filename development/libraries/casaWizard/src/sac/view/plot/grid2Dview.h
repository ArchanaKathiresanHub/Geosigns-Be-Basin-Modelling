// Widget to display 2D view of grids
#pragma once

#include "../common/view/plot/plot.h"
#include "wellBirdsView.h"

namespace casaWizard
{

namespace sac
{

class ColorMap;

class Grid2DView : public WellBirdsView
{
  Q_OBJECT

public:
  explicit Grid2DView(const ColorMap& colormap, QWidget* parent = nullptr);

  void updatePlots(const QVector<QVector<double> > values);
  void updateRange(const double xMin, const double xMax, const double yMin, const double yMax);
  void setWellsVisible(const bool wellsVisible);
  void setStretch(const bool stretched);
  void clearData();
  const std::pair<double, double>& getValueRange() const;
  void setFixedValueRange(const std::pair<double, double>& valueRange);
  void setVariableValueRange();

  void drawData(QPainter &painter) override;
  void updateMinMaxData() override;

private:
  const ColorMap& colorMap_;
  std::unique_ptr<std::pair<double, double>> range_;
  QVector<QVector<double>> values_;

  bool fixedRange_;
  bool stretched_;
  bool wellsVisible_;

  void determineRange();
  void stretch();
};

} // namespace sac

} // namespace casaWizard
