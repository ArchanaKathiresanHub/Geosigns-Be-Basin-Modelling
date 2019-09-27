// Table to define depth target, i.e., x, y, z and checkbox for time series
#pragma once

#include <QWidget>

class QPushButton;
class QTableWidget;

namespace casaWizard
{

namespace ua
{

class PredictionTargetDepth;

class DepthTargetTable : public QWidget
{
  Q_OBJECT

public:
  explicit DepthTargetTable(QWidget* parent = 0);

  void updateTable(const QVector<PredictionTargetDepth> depthTargets,
                   const QStringList& predictionTargetOptions,
                   const QVector<bool>& hasTimeSeriesForPredictionTargets);

  const QTableWidget* tableWidgetDepthTargets() const;
  const QPushButton* pushButtonAddDepthTarget() const;
  const QPushButton* pushButtonDelDepthTarget() const;
  const QPushButton* pushButtonCopyDepthTarget() const;

  int checkBoxColumnNumber() const;

private:
  QTableWidget* tableWidgetPredictionTargets_;
  QPushButton* pushButtonAddPredictionTarget_;
  QPushButton* pushButtonDelPredictionTarget_;
  QPushButton* pushButtonCopyPredictionTarget_;
  const int checkBoxColumnNumber_;
};

} // namespace ua

} // namespace casaWizard
