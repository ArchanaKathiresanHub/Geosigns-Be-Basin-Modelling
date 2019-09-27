#include "correlationTab.h"
#include "correlationWidget.h"
#include "correlationSelectionTable.h"
#include <QHBoxLayout>
#include <algorithm>
#include <QScrollArea>

namespace casaWizard
{

namespace ua
{

CorrelationTab::CorrelationTab(QWidget* parent) :
  QWidget(parent),
  scrollArea_{new QScrollArea{this}},
  correlationSelectTable_{new CorrelationSelectionTable{this}},
  correlationWidget_{new CorrelationWidget{this}}
{
  QHBoxLayout* layoutCorrelation = new QHBoxLayout();
  layoutCorrelation->addWidget(correlationSelectTable_);
  layoutCorrelation->addWidget(scrollArea_);
  layoutCorrelation->setStretch(0, 1);
  layoutCorrelation->setStretch(1, 3);

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
