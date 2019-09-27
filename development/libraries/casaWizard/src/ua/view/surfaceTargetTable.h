// Table for surface target, i.e., x,y, name of layer and checkbox for time series
#pragma once

#include <QWidget>

class QPushButton;
class QTableWidget;

namespace casaWizard
{

namespace ua
{

class PredictionTargetSurface;

class SurfaceTargetTable : public QWidget
{
  Q_OBJECT

public:
  explicit SurfaceTargetTable(QWidget* parent = 0);

  void updateTable(const QVector<PredictionTargetSurface> depthTargets,
                   const QStringList predictionTargetOptions,
                   const QVector<bool>& hasTimeSeriesForPredictionTargets,
                   const QStringList validLayerNames);

  const QTableWidget* tableWidgetSurfaceTargets() const;
  const QPushButton* pushButtonAddSurfaceTarget() const;
  const QPushButton* pushButtonDelSurfaceTarget() const;
  const QPushButton* pushButtonCopySurfaceTarget() const;

  int checkBoxColumnNumber() const;

private:
  QTableWidget* tableWidgetTargets_;
  QPushButton* pushButtonAddTarget_;
  QPushButton* pushButtonDelTarget_;
  QPushButton* pushButtonCopyTarget_;
  const int checkBoxColumnNumber_;
};

} // namespace ua

} // namespace casaWizard
