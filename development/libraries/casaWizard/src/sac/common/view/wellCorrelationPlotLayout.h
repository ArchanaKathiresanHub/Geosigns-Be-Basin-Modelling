//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include <QWidget>

class QComboBox;
class QDoubleSpinBox;

namespace casaWizard
{

class Legend;
struct CalibrationTarget;

namespace sac
{
struct WellTrajectory;
class WellCorrelationPlot;

class WellCorrelationPlotLayout : public QWidget
{
  Q_OBJECT
public:
  explicit WellCorrelationPlotLayout(QWidget *parent = nullptr);

  WellCorrelationPlot* wellCorrelationPlot() const;
  void updateCorrelationPlot(const QVector<QVector<double> >& measuredValueTrajectories,
                             const QVector<QVector<double> >& simulatedValueTrajectories,
                             const QString activeProperty,
                             const QVector<bool> activePlots, const double minValue, const double maxValue, const QVector<int>& wellIndices);
  void setProperties(const QStringList& properties, const int activeIndex);

signals:
  void propertyChanged(QString property);

private slots:
  void slotAbsoluteErrorChanged(double absoluteError);
  void slotRelativeErrorChanged(double relativeError);

private:
  QWidget*createOptions();

  Legend* legend_;
  WellCorrelationPlot* wellCorrelationPlot_;
  QWidget* correlationPlotOptions_;
  QComboBox* properties_;
  QDoubleSpinBox* absoluteErrorRange_;
  QDoubleSpinBox* relativeErrorRange_;
};

} // namespace sac

} // namespace casaWizard
