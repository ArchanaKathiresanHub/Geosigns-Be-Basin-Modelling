//
// Copyright (C) 2012-2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// Widget that contains the plot options for the SAC results
#pragma once

#include <QWidget>

class QButtonGroup;
class QCheckBox;
class QComboBox;
class QPushButton;
class CustomRadioButton;

namespace casaWizard
{

namespace sac
{

class PlotOptions : public QWidget
{
  Q_OBJECT

public:
  explicit PlotOptions(QWidget* parent = 0);

  QVector<bool> activePlots() const;
  void setActivePlots(const QVector<bool> activePlots);

signals:
  void activeChanged();
  void plotTypeChange(int currentIndex);

private slots:
  void plotTypeButtonToggle(int index, bool checked);

private:
  QCheckBox* original1d_;
  QCheckBox* optimized1d_;
  QCheckBox* original3d_;
  QCheckBox* optimized3d_;

  QButtonGroup* plotType_;
  CustomRadioButton* table_;
  CustomRadioButton* depthPlot_;
  CustomRadioButton* correlationPlot_;
};

}  // namespace sac

}  // namespace casaWizard
