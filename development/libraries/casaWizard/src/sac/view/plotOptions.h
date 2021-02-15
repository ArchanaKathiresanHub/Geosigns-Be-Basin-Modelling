//
// Copyright (C) 2021 Shell International Exploration & Production.
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

  void setProperties(const QStringList& properties, const int activeIndex);

signals:
  void activeChanged();
  void plotTypeChange(int currentIndex);
  void propertyChanged(QString property);

private slots:
  void plotTypeButtonToggle(int index, bool checked);

private:
  QCheckBox* base1d_;
  QCheckBox* optimized1d_;
  QCheckBox* base3d_;
  QCheckBox* optimized3d_;
  QComboBox* properties_;

  QButtonGroup* plotType_;
  CustomRadioButton* table_;
  CustomRadioButton* linePlot_;
  CustomRadioButton* scatterPlot_;
};

}  // namespace sac

}  // namespace casaWizard
