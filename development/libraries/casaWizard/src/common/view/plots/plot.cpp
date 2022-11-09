#include "plot.h"

#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QPointF>
#include <QRectF>

#include <algorithm>
#include <cmath>

namespace casaWizard
{

Plot::Plot(QWidget* parent) :
  PlotBase(parent),
  plotData_{},
  label_{new QLabel("", this)},
  minZvalue_{0.0},
  maxZvalue_{1.0},
  separateLegend_{false}
{
  label_->setVisible(false);
  label_->setBackgroundRole(QPalette::Base);
  label_->setAutoFillBackground(true);
  label_->setFrameShape(QFrame::Shape::Box);
  dataChanged();
}

void Plot::clearData()
{
  plotData_.clear();
}

int Plot::amountPerLineType(LineType type) const
{
  int amount = 0;
  for (const PlotData& data : plotData_)
  {
    if (data.lineType == type)
    {
      amount++;
    }
  }
  return amount;
}

void Plot::addLine(const QVector<double>& xValues, const QVector<double>& yValues, int colorIndex, const QString& seriesName)
{
  PlotData data;
  data.lineType = LineType::line;
  data.xValues = xValues;
  data.yValues = yValues;
  data.seriesName = seriesName;
  if (colorIndex < 0)
  {
    colorIndex = amountPerLineType(LineType::line);
  }
  colorIndex = colorIndex%plotSettings_.nLineColors;
  data.color = plotSettings_.lineColors[colorIndex];

  plotData_.append(data);

  dataChanged();
}

void Plot::addErrorBarHorizontal(const QVector<QVector<double>>& xValuesMatrix, const QVector<double>& yValues)
{
  int i = 0;
  for (const QVector<double>& xValues : xValuesMatrix)
  {
    PlotData data;
    data.lineType = LineType::errorBarHorizontal;
    data.xValues = xValues;
    data.yValues = {yValues[i], yValues[i]};
    data.color = Qt::black;
    plotData_.append(data);
    ++i;
  }

  dataChanged();
}

void Plot::addXYscatter(const QVector<double>& xValues,
                        const QVector<double>& yValues,
                        const QVector<double>& zValues)
{
  PlotData data;
  data.lineType = LineType::xyScatter;
  data.xValues = xValues;
  data.yValues = yValues;
  data.zValues = zValues;
  data.color = Qt::black;
  plotData_.append(data);

  dataChanged();
}

void Plot::addXYscatter(const QVector<double>& xValues,
                        const QVector<double>& yValues,
                        const QColor color)
{
  PlotData data;
  data.lineType = LineType::xyScatter;
  data.xValues = xValues;
  data.yValues = yValues;
  data.zValues = {};
  data.color = color;
  plotData_.append(data);

  dataChanged();
}

void Plot::addXYscatter(const QVector<double>& xValues, const QVector<double>& yValues, int colorIndex)
{
  if (colorIndex < 0)
  {
    colorIndex = amountPerLineType(LineType::xyScatter);
  }
  colorIndex = colorIndex%plotSettings_.nLineColors;
  addXYscatter(xValues, yValues, plotSettings_.lineColors[colorIndex]);
}

void Plot::addXYscatter(const QVector<double>& xValues, const QVector<double>& yValues, const SymbolType symbolType, int colorIndex)
{
  if (colorIndex < 0)
  {
    colorIndex = amountPerLineType(LineType::xyScatter);
  }
  colorIndex = colorIndex%plotSettings_.nLineColors;
  PlotData data;
  data.lineType = LineType::xyScatter;
  data.symbolType = symbolType;
  data.xValues = xValues;
  data.yValues = yValues;
  data.zValues = {};
  data.color = plotSettings_.lineColors[colorIndex];
  plotData_.append(data);
  dataChanged();
}

void Plot::addHorizontalLine(const QString& seriesName, const double yValue)
{
  addLine(QVector<double>{xAxisMinValue(), xAxisMaxValue()}, QVector<double>{yValue, yValue}, 6, seriesName);
}

void Plot::setSeparateLegend(const bool separateLegend)
{
  separateLegend_ = separateLegend;
}

const PlotSettings& Plot::plotSettings() const
{
  return plotSettings_;
}

const QVector<PlotData>& Plot::plotDataForLegend() const
{
  return plotData_;
}

const QStringList& Plot::legend() const
{
  return legend_;
}

void Plot::setMinMaxZvalues(const double minValue, const double maxValue)
{
  minZvalue_ = minValue;
  maxZvalue_ = maxValue;
}

void Plot::setLegend(const QStringList& legend)
{
  legend_ = legend;
}

void Plot::updateMinMaxData()
{
  double xAxisMinValue = 0.0;
  double xAxisMaxValue = 1.0;
  double yAxisMinValue = 0.0;
  double yAxisMaxValue = 1.0;
  bool first = true;
  for (const PlotData& lineData : plotData_)
  {
    if (lineData.xValues.empty() || lineData.yValues.empty())
    {
      continue;
    }
    else if (first)
    {
      xAxisMinValue = *std::min_element(lineData.xValues.constBegin(), lineData.xValues.constEnd());
      xAxisMaxValue = *std::max_element(lineData.xValues.constBegin(), lineData.xValues.constEnd());
      yAxisMinValue = *std::min_element(lineData.yValues.constBegin(), lineData.yValues.constEnd());
      yAxisMaxValue = *std::max_element(lineData.yValues.constBegin(), lineData.yValues.constEnd());
      first = false;
    }
    else
    {
      xAxisMinValue = std::min(xAxisMinValue, *std::min_element(lineData.xValues.constBegin(), lineData.xValues.constEnd()));
      xAxisMaxValue = std::max(xAxisMaxValue, *std::max_element(lineData.xValues.constBegin(), lineData.xValues.constEnd()));
      yAxisMinValue = std::min(yAxisMinValue, *std::min_element(lineData.yValues.constBegin(), lineData.yValues.constEnd()));
      yAxisMaxValue = std::max(yAxisMaxValue, *std::max_element(lineData.yValues.constBegin(), lineData.yValues.constEnd()));
    }
  }

  if (xAxisMaxValue == xAxisMinValue)
  {
    xAxisMaxValue += 0.1*std::fabs(xAxisMaxValue);
    xAxisMinValue -= 0.1*std::fabs(xAxisMinValue);
  }
  if (yAxisMaxValue == yAxisMinValue)
  {
    yAxisMaxValue += 0.1*std::fabs(yAxisMaxValue);
    yAxisMinValue -= 0.1*std::fabs(yAxisMinValue);
  }

  setMinMaxValues(xAxisMinValue, xAxisMaxValue, yAxisMinValue, yAxisMaxValue);
}

void Plot::drawData(QPainter& painter)
{
  for (const PlotData& lineData : plotData_)
  {
    painter.save();
    QPen pen = painter.pen();

    painter.setPen(pen);
    switch (lineData.lineType)
    {
      case LineType::line:
      {
        pen.setWidth(plotSettings_.lineWidth);
        pen.setColor(lineData.color);
        painter.setPen(pen);
        bool first = true;
        QPointF prevPoint;
        for (int i = 0; i < lineData.xValues.size(); ++i)
        {
          QPointF point = valToPoint(lineData.xValues[i], lineData.yValues[i]);
          if (first)
          {
            prevPoint = point;
            first = false;
            continue;
          }
          painter.drawLine(prevPoint,point);
          prevPoint = point;
        }
        break;
      }
      case LineType::errorBarHorizontal:
      {
        QPointF edgeErrorBar(0.0, 2.0);
        pen.setWidth(plotSettings_.lineWidthErrorBar);
        pen.setColor(lineData.color);
        painter.setPen(pen);
        bool first = true;
        QPointF prevPoint;
        for (int i = 0; i < lineData.xValues.size(); ++i)
        {
          QPointF point = valToPoint(lineData.xValues[i], lineData.yValues[i]);
          painter.drawLine(point + edgeErrorBar, point - edgeErrorBar);
          if (first)
          {
            prevPoint = point;
            first = false;
            continue;
          }
          painter.drawLine(prevPoint,point);
          prevPoint = point;
        }
        break;
      }
      case LineType::xyScatter:
      {
        for (int i = 0; i < lineData.xValues.size(); ++i)
        {
          if ( lineData.zValues.size() > i)
          {
            pen.setColor(getColor(lineData.zValues[i], minZvalue_, maxZvalue_));
          }
          else
          {
            pen.setColor(lineData.color);
          }
          painter.setPen(pen);
          QPointF point = valToPoint(lineData.xValues[i], lineData.yValues[i]);
          plotSymbol(painter, point, plotSettings_.lineWidthSymbol, plotSettings_.markerSize, lineData.symbolType);
        }
        break;
      }
    }

    painter.restore();
  }

  if (!separateLegend_)
  {
    drawLegend(painter);
  }
}

void Plot::plotSymbol(QPainter& painter, const QPointF& point, const int lineWidthSymbol, const int markerSize, const SymbolType symbolType)
{
  painter.save();
  QPen pen = painter.pen();
  pen.setWidth(lineWidthSymbol);
  painter.setPen(pen);
  switch (symbolType)
  {
    case SymbolType::Circle:
    {
      pen.setWidth(1);
      painter.drawEllipse(point, markerSize, markerSize);
    }
      break;

    default:
    {
      painter.drawLine(point + QPointF( markerSize, -markerSize), point + QPointF(-markerSize, markerSize));
      painter.drawLine(point + QPointF(-markerSize, -markerSize), point + QPointF( markerSize, markerSize));
    }
      break;
  }
  painter.restore();
}

void Plot::drawLegend(QPainter& painter)
{
  const double verticalSpacing = 20;
  const double horizontalSpacing = 40;

  const int nPlots = plotData_.size();
  const int nNames = legend_.size();
  const int nEntries = (nPlots<nNames)?nPlots:nNames;

  if (nEntries==0)
  {
    return;
  }

  painter.save();
  QPen pen = painter.pen();
  pen.setWidth(plotSettings_.lineWidth);

  QFontMetrics fm(painter.font());
  double maxLegendWidth = 0;
  for( int i = 0; i<nEntries; ++i)
  {
    const double w = fm.horizontalAdvance(legend_[i]);
    maxLegendWidth = (w>maxLegendWidth)?w:maxLegendWidth;
  }

  const double xMax = xAxisMaxValue();
  const double yMax = yAxisMaxValue();

  const QPointF v(0,verticalSpacing);
  const QPointF h(horizontalSpacing, 0);
  const QPointF textTopLeft = valToPoint(xMax, yMax) - QPoint(maxLegendWidth, 0) - h + v;

  int iEntry = 0;
  for (const PlotData& data : plotData_)
  {
    if (data.lineType == LineType::errorBarHorizontal || data.seriesName != "")
    {
      continue;
    }
    QPointF y = textTopLeft+iEntry*v;

    pen.setColor(Qt::black);
    painter.setPen(pen);
    if (iEntry<legend_.size())
    {
      painter.drawText(y+h, legend_[iEntry]);
    }

    pen.setColor(data.color);
    painter.setPen(pen);
    switch (data.lineType)
    {
      case LineType::line:
        painter.drawLine(y-2*h, y);
        break;
      case LineType::xyScatter:
        plotSymbol(painter, y-h, plotSettings_.lineWidthSymbol, plotSettings_.markerSize, data.symbolType);
        break;
    }
    ++iEntry;
  }
  painter.restore();
}

QColor Plot::getColor(double value, double minValue, double maxValue)
{
  int red = 0;
  int green = 0;
  int blue = 0;
  value -= minValue;
  maxValue -= minValue;
  double quarter = maxValue/4;
  if (value <= quarter)
  {
    blue = 255;
    green = value/quarter*255;
  }
  else if (value <= quarter*2)
  {
    green = 255;
    blue = (quarter*2 - value)/quarter*255;
  }
  else if (value <= quarter*3)
  {
    green = 255;
    red = (value - quarter*2)/quarter*255;
  }
  else if (value <= maxValue)
  {
    red = 255;
    green = (maxValue - value)/quarter*255;
  }
  return QColor(red, green, blue);
}

void Plot::mousePressEvent(QMouseEvent* event)
{
  label_->setVisible(false);

  QPoint mousePosition = event->pos();
  const int x = mousePosition.x();
  const int y = mousePosition.y();

  int iPlot = 0;
  for (const PlotData& plotData : plotData_)
  {
    iPlot++;
    if (plotData.lineType == LineType::xyScatter)
    {
      for (int i = 0; i < plotData.xValues.size(); ++i)
      {
        QPointF p = valToPoint(plotData.xValues[i], plotData.yValues[i]);
        if (x < p.x() + plotSettings_.markerSize && x > p.x() - plotSettings_.markerSize &&
            y < p.y() + plotSettings_.markerSize && y > p.y() - plotSettings_.markerSize)
        {
          label_->setText(QString("x: " + QString::number(plotData.xValues[i]) + "\ny: " + QString::number(plotData.yValues[i])));
          mousePosition.setX(std::max(0, std::min(x, width() - label_->width())));
          mousePosition.setY(std::max(0, std::min(y, height() - label_->height())));

          label_->move(mousePosition);
          label_->setVisible(true);
          emit pointSelectEvent(iPlot, i);
          return;
        }
      }
    }
  }
}

void Plot::mouseMoveEvent(QMouseEvent* event)
{
  label_->setVisible(false);

  QPoint mousePosition = event->pos();
  const int x = mousePosition.x();
  const int y = mousePosition.y();

  int iPlot = 0;
  QString labelText = "";
  for (const PlotData& plotData : plotData_)
  {
    iPlot++;
    if (plotData.lineType == LineType::line && plotData.seriesName != "")
    {
      QPointF p = valToPoint(plotData.xValues[0], plotData.yValues[0]);
      if (y < p.y() + plotSettings_.markerSize && y > p.y() - plotSettings_.markerSize)
      {
        labelText += labelText != "" ? "\n" : "";
        labelText += plotData.seriesName;
      }
    }
  }
  if (labelText!="")
  {
    label_->setText(labelText);
    mousePosition.setX(std::max(0, std::min(x, width() - label_->width())));
    mousePosition.setY(std::max(0, std::min(y, height() - label_->height())));

    label_->move(mousePosition);
    label_->setVisible(true);
  }
}

}

