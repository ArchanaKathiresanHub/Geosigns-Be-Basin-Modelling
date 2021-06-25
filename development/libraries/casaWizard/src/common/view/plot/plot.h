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

struct PlotSettings
{
  int markerSize = 3;
  int lineWidth = 3;
  int lineWidthErrorBar = 1;
  int lineWidthSymbol = 2;

  QVector<QColor> lineColors = {QColor(222,24,31),  QColor(0, 96, 174), QColor(34,140,34),
                                      QColor(24,222,215), QColor(174,78,0),   QColor(140, 34, 140),
                                      QColor(174,78,0, 50)};
  int nLineColors = lineColors.size();
};

enum class SymbolType
{
  Cross,
  Circle
};

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
  QString seriesName;

  PlotData( const LineType& lineType = {}, const SymbolType& symbolType = SymbolType::Cross, const QVector<double>& xValues = {}, const QVector<double>& yValues = {}, const QVector<double>& zValues = {}, const QColor& color = {},
            const QString& seriesName = "") :
    lineType{lineType},
    symbolType{symbolType},
    xValues{xValues},
    yValues{yValues},
    zValues{zValues},
    color{color},
    seriesName{seriesName}
  {
  }
};

class Plot : public PlotBase
{
  Q_OBJECT


public:

  explicit Plot(QWidget* parent = 0);

  void addLine(const QVector<double>& xValues, const QVector<double>& yValues, int colorIndex = -1, const QString& seriesName = "");
  void addErrorBarHorizontal(const QVector<QVector<double>>& xValuesMatrix, const QVector<double>& yValues);
  void addXYscatter(const QVector<double>& xValues, const QVector<double>& yValues, const QVector<double>& zValues);
  void addXYscatter(const QVector<double>& xValues, const QVector<double>& yValues, const QColor color = Qt::black);
  void addXYscatter(const QVector<double>& xValues, const QVector<double>& yValues, int colorIndex);
  void addXYscatter(const QVector<double>& xValues, const QVector<double>& yValues, const SymbolType symbolType, int colorIndex = -1);
  void setMinMaxZvalues(const double minValue, const double maxValue);
  void setLegend(const QStringList& legend);
  void addHorizontalLine(const QString& seriesName, const double yValue);
  void setSeparateLegend(const bool separateLegend);
  static void plotSymbol(QPainter& painter, const QPointF& point, const int lineWidthSymbol, const int markerSize, const SymbolType symbolType = SymbolType::Cross);
  const PlotSettings& plotSettings() const;
  const QVector<PlotData>& plotDataForLegend() const;
  const QStringList& legend() const;


signals:
  void pointSelectEvent(int, int);

protected:
  void clearData() override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void drawData(QPainter& painter) override;
  void updateMinMaxData() override;

private:
  void drawLegend(QPainter& painter);
  int amountPerLineType(LineType type) const;
  QColor getColor(double value, double minValue, double maxValue);

  QVector<PlotData> plotData_;
  QStringList legend_;
  QLabel* label_;
  double minZvalue_;
  double maxZvalue_;
  bool separateLegend_;
  PlotSettings plotSettings_;
};

} // namespace casaWizard
