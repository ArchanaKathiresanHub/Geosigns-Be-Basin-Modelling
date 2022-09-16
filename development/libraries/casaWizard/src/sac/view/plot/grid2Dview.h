//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// Widget to display 2D view of grids
#pragma once

#include "../common/view/plot/plot.h"

#include "vector"

namespace casaWizard
{

class ColorMap;

namespace sac
{

class Grid2DView : public Plot
{
  Q_OBJECT

public:
  explicit Grid2DView(const ColorMap& colormap, QWidget* parent = nullptr);

  void setFixedValueRange(const std::pair<double, double>& valueRange);  
  void setStretch(const bool stretched);
  void setVariableValueRange();  

  const std::pair<double, double>& getValueRange() const;
  double getValue(const QPointF& point) const;

  void updatePlots(const std::vector<std::vector<double>>& values);
  void updateRange(const double xMin, const double xMax, const double yMin, const double yMax);

  void clearData() override;
  void drawData(QPainter& painter) override;
  void updateMinMaxData() override;

private:
  const ColorMap& colorMap_;
  std::unique_ptr<std::pair<double, double>> range_;
  std::vector<std::vector<double>> values_;   

  bool fixedRange_;
  bool stretched_;

  void determineRange();  
  void stretch();
};

} // namespace sac

} // namespace casaWizard
