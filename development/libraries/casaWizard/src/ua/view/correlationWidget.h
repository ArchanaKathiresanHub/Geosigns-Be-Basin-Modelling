//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// Widget for a table to select influential parameters, intended for plotting the correlation matrix
#pragma once

#include <QWidget>

class QTableWidget;
class QWheelEvent;

namespace casaWizard
{

class PlotBase;

namespace ua
{

class InfluentialParameter;

class CorrelationWidget : public QWidget
{
   Q_OBJECT

public:
   explicit CorrelationWidget(QWidget* parent = 0);

   void updateLayoutPlots(const QVector<InfluentialParameter*> influentialParameters, const QVector<bool>& isInfluentialParameterSelected,
                          const QVector<QVector<double> >& parameterMatrix, const QVector<double>& rmse);
   void wheelEvent(QWheelEvent* event) override;

private:
   QVector<PlotBase*> plots_;
   int nInfluentialParameters_;
   double plotWidth_;
   double plotHeight_;
};

} // namespace ua
} // namespace casaWizard
