// Tab to view correlations between influential parameters

#pragma once

#include <QVector>
#include <QWidget>

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
  explicit CorrelationTab(QWidget* parent = 0);

  void fillCorrelationSelectionTable(const QVector<InfluentialParameter*> influentialParameters, const QVector<bool>& isInfluentialParameterSelected);
  void updateCorrelationPlotLayout(const QVector<InfluentialParameter*> influentialParameters, const QVector<bool>& isInfluentialParameterSelected,
                                   const QVector<QVector<double>>& parameterMatrix, const QVector<double>& rmse);
  QTableWidget* tableParameters() const;
  QVector<bool> isCorrelationSelectTableItemSelected() const;

private:
  QScrollArea* scrollArea_;
  CorrelationSelectionTable* correlationSelectTable_;
  CorrelationWidget* correlationWidget_;
};

} // namespace ua

} // namespace casaWizard
