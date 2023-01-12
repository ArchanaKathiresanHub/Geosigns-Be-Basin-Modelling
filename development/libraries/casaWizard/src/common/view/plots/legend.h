#pragma once
#include <QWidget>

#include "plot.h"

namespace casaWizard
{

struct PlotData;

class Legend : public QWidget
{
  Q_OBJECT

public:
  Legend(const casaWizard::PlotSettings& plotSettings, const QVector<PlotData>& plotData, const QStringList& entries, QWidget* parent = nullptr);
  void paintEvent(QPaintEvent* event) override;

  const QStringList* getEntries();

private:
  const PlotSettings& plotSettings_;
  QStringList entries_;
  QVector<PlotData> plotData_;
};

}
