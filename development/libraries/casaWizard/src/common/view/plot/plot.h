// Plot widget for lines and scatter
#pragma once

#include "plotBase.h"

#include <QVector>
#include <QStringList>

class QColor;
class QLabel;
class QPointF;

namespace casaWizard
{

enum class SymbolType
{
  Cross,
  Circle
};

class Plot : public PlotBase
{
  Q_OBJECT

  enum class LineType
  {
    line = 0,
    xyScatter = 1,
    errorBarHorizontal = 2
  };

  struct PlotData
  {
    LineType lineType;
    SymbolType symbolType;
    QVector<double> xValues;
    QVector<double> yValues;
    QVector<double> zValues;
    QColor color;

    PlotData( const LineType& lineType = {}, const SymbolType& symbolType = SymbolType::Cross, const QVector<double>& xValues = {}, const QVector<double>& yValues = {}, const QVector<double>& zValues = {}, const QColor& color = {}) :
      lineType{lineType},
      symbolType{symbolType},
      xValues{xValues},
      yValues{yValues},
      zValues{zValues},
      color{color}
    {
    }
  };

public:
  explicit Plot(QWidget* parent = 0);

  void addLine(const QVector<double>& xValues, const QVector<double>& yValues, int colorIndex = -1);
  void addErrorBarHorizontal(const QVector<QVector<double>>& xValuesMatrix, const QVector<double>& yValues);
  void addXYscatter(const QVector<double>& xValues, const QVector<double>& yValues, const QVector<double>& zValues);
  void addXYscatter(const QVector<double>& xValues, const QVector<double>& yValues, const QColor color = Qt::black);
  void addXYscatter(const QVector<double>& xValues, const QVector<double>& yValues, int colorIndex);
  void addXYscatter(const QVector<double>& xValues, const QVector<double>& yValues, const SymbolType symbolType, int colorIndex = -1);
  void setMinMaxZvalues(const double minValue, const double maxValue);
  void setLegend(const QStringList& legend);

signals:
  void pointSelectEvent(int, int);

protected:
  void clearData() override;
  void mousePressEvent(QMouseEvent* event) override;
  void drawData(QPainter& painter) override;

private:
  void updateMinMaxData() override;
  void drawLegend(QPainter& painter);
  void plotSymbol(QPainter& painter, const QPointF& point, const SymbolType symbolType = SymbolType::Cross);
  int amountPerLineType(LineType type) const;
  QColor getColor(double value, double minValue, double maxValue);

  QVector<PlotData> plotData_;
  QStringList legend_;
  QLabel* label_;
  double minZvalue_;
  double maxZvalue_;
};

} // namespace casaWizard
