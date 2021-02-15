//
// Copyright (C) 2020 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include <QWidget>

namespace casaWizard
{
namespace sac
{

class ColorMap;

class ColorBar : public QWidget
{
  Q_OBJECT
public:
  explicit ColorBar(const ColorMap& colorMap, QWidget* parent = nullptr);
  void paintEvent(QPaintEvent*) override;
  void setRange(const std::pair<double, double>& range);

private:
  int colorbarHeight_;
  int colorbarWidth_;
  int colorbarOffsetY_;
  const ColorMap& colorMap_;
  int numberOfTicks_;
  std::pair<double, double> range_;
  std::vector<int> tickPositions_;
  std::vector<std::string> tickLabels_;

  std::string doubleToString(const double number, const int decimals) const;
  void drawColorBar();
  bool duplicateLabel(const std::string& label) const;
  QColor getColor(const int i, const int j) const;
  bool isVerticalTickPosition(int i) const;
  void obtainTickPositionsAndLabels();
  bool placeTicks(const int decimals);
  void setColorbarProperties();
};

} // namespace sac

} // namepsace casaWizard
