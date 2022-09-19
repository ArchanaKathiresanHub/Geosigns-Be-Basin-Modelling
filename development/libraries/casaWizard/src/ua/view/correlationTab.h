//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// Tab to view correlations between influential parameters
#pragma once

#include <QVector>
#include <QWidget>
#include "view/ColorBar.h"

class QTableWidget;
class QTableWidgetItem;
class QScrollArea;

namespace casaWizard
{

namespace ua
{

class CorrelationWidget;
class CorrelationSelectionTable;
class InfluentialParameter;

class CorrelationTab : public QWidget
{
   Q_OBJECT

public:
   explicit CorrelationTab(QWidget* parent = nullptr);

   void fillCorrelationSelectionTable(const QVector<InfluentialParameter*> influentialParameters, const QVector<bool>& isInfluentialParameterSelected);
   void updateCorrelationPlotLayout(const QVector<InfluentialParameter*> influentialParameters, const QVector<bool>& isInfluentialParameterSelected,
                                    const QVector<QVector<double>>& parameterMatrix, const QVector<double>& rmse);

   ColorBar* colorbar() { return m_colorBar; }
   QTableWidget* tableParameters() const;
   QVector<bool> isCorrelationSelectTableItemSelected() const;

private:
   QScrollArea* scrollArea_;
   CorrelationSelectionTable* correlationSelectTable_;
   CorrelationWidget* correlationWidget_;
   ColorBar* m_colorBar;
};

} // namespace ua

} // namespace casaWizard
