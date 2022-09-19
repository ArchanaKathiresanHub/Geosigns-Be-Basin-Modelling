//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "correlationTab.h"
#include "correlationWidget.h"
#include "correlationSelectionTable.h"
#include <QHBoxLayout>
#include <algorithm>
#include <QScrollArea>
#include "view/ColorBar.h"
#include "view/colormap.h"

namespace casaWizard
{

namespace ua
{

CorrelationTab::CorrelationTab(QWidget* parent) :
   QWidget(parent),
   scrollArea_{new QScrollArea{this}},
   correlationSelectTable_{new CorrelationSelectionTable{this}},
   correlationWidget_{new CorrelationWidget{this}},
   m_colorBar(new ColorBar(ColorBar::left, this))
{
   m_colorBar->setColorMap(ColorMap(RAINBOW));
   m_colorBar->setTitle("Normalized RMSE");
   m_colorBar->setDecimals(3);
   m_colorBar->setFixedWidth(this->width());
   m_colorBar->setVisible(false);

   QVBoxLayout* vBox = new QVBoxLayout();
   vBox->addWidget(correlationSelectTable_);

   QHBoxLayout* hBox = new QHBoxLayout();
   hBox->addStretch(1);
   hBox->addWidget(m_colorBar);

   vBox->addLayout(hBox);
   vBox->setStretch(0, 1);
   vBox->setStretch(1, 2);

   QHBoxLayout* layoutCorrelation = new QHBoxLayout();
   layoutCorrelation->addLayout(vBox);

   layoutCorrelation->addWidget(scrollArea_);
   layoutCorrelation->setStretch(0, 1); //table
   layoutCorrelation->setStretch(1, 3); //scrollarea

   setLayout(layoutCorrelation);

   scrollArea_->setBackgroundRole(QPalette::Base);
   scrollArea_->setWidget(correlationWidget_);
}

void CorrelationTab::fillCorrelationSelectionTable(const QVector<InfluentialParameter*> influentialParameters, const QVector<bool>& isInfluentialParameterSelected)
{
   correlationSelectTable_->updateTable(influentialParameters, isInfluentialParameterSelected);
}

void CorrelationTab::updateCorrelationPlotLayout(const QVector<InfluentialParameter*> influentialParameters, const QVector<bool>& isInfluentialParameterSelected,
                                                 const QVector<QVector<double>>& parameterMatrix, const QVector<double>& rmse)
{
   correlationWidget_->updateLayoutPlots(influentialParameters, isInfluentialParameterSelected, parameterMatrix, rmse);
   m_colorBar->setRange(*std::min_element(rmse.begin(), rmse.end()), *std::max_element(rmse.begin(), rmse.end()), 4); //change last parameter to fit desired amount of intervals
}

QTableWidget* CorrelationTab::tableParameters() const
{
   return correlationSelectTable_->tableParameters();
}

QVector<bool> CorrelationTab::isCorrelationSelectTableItemSelected() const
{
   return correlationSelectTable_->getStatus();
}

} // namespace ua

} // namespace casaWizard
