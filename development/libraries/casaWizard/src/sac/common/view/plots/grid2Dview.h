//
// Copyright (C) 2012-2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// Widget to display 2D view of grids
#pragma once

#include "view/plots/plot.h"

#include <vector>

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

protected:
  std::unique_ptr<std::pair<double, double>> m_range;
  virtual void determineRange();
  const ColorMap& colorMap() const;


private:
  const ColorMap& m_colorMap;
  std::vector<std::vector<double>> m_values;

  bool m_fixedRange;
  bool m_stretched;

  void stretch();
};

} // namespace sac

} // namespace casaWizard
