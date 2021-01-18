#include "plotBase.h"
#include "plotBasePrivate.h"

#include <QPainter>
#include <QPaintEvent>
#include <QPointF>
#include <QRect>

#include <algorithm>
#include <cmath>

namespace casaWizard
{

const int textSpacing = 3;
const int majorTickLength = 6;

// PlotBasePrivate

PlotBase::PlotBasePrivate::PlotBasePrivate() :
  xAxisMinValue_{0.0},
  xAxisMaxValue_{1.0},
  yAxisMinValue_{0.0},
  yAxisMaxValue_{1.0},
  maxYtickWidth_{0.0},
  aspectRatioPlotArea_{0.0},
  plotRangeTopLeft_{},
  plotRangeBottomRight_{},
  font_{"Sans",10}
{
}

QPointF PlotBase::PlotBasePrivate::valToPoint(double x, double y)
{
  QPointF origin(plotRangeTopLeft_.x(), plotRangeBottomRight_.y());
  const double px = (x - xAxisMinValue_)/(xAxisMaxValue_ - xAxisMinValue_)*(plotRangeBottomRight_.x() - plotRangeTopLeft_.x());
  const double py = (y - yAxisMinValue_)/(yAxisMaxValue_ - yAxisMinValue_)*(plotRangeTopLeft_.y() - plotRangeBottomRight_.y()); //y-direction is top down

  return origin + QPointF(px, py);
}

QFont PlotBase::PlotBasePrivate::font() const
{
  return font_;
}

void PlotBase::PlotBasePrivate::setFont(const QFont& font)
{
  font_ = font;
}

QPointF PlotBase::PlotBasePrivate::plotRangeTopLeft() const
{
  return plotRangeTopLeft_;
}

QPointF PlotBase::PlotBasePrivate::plotRangeBottomRight() const
{
  return plotRangeBottomRight_;
}

void PlotBase::PlotBasePrivate::setXLabel(const QString& xLabel)
{
  xLabel_ = xLabel;
}

void PlotBase::PlotBasePrivate::setYLabel(const QString& yLabel)
{
  yLabel_ = yLabel;
}

void PlotBase::PlotBasePrivate::setAspectRatio(const double aspectRatio)
{
  aspectRatioPlotArea_ = aspectRatio;
}

double PlotBase::PlotBasePrivate::xAxisMinValue() const
{
  return xAxisMinValue_;
}

void PlotBase::PlotBasePrivate::setXAxisMinValue(double xAxisMinValue)
{
  xAxisMinValue_ = xAxisMinValue;
}

double PlotBase::PlotBasePrivate::xAxisMaxValue() const
{
  return xAxisMaxValue_;
}

void PlotBase::PlotBasePrivate::setXAxisMaxValue(double xAxisMaxValue)
{
  xAxisMaxValue_ = xAxisMaxValue;
}

double PlotBase::PlotBasePrivate::yAxisMinValue() const
{
  return yAxisMinValue_;
}

void PlotBase::PlotBasePrivate::setYAxisMinValue(double yAxisMinValue)
{
  yAxisMinValue_ = yAxisMinValue;
}

double PlotBase::PlotBasePrivate::yAxisMaxValue() const
{
  return yAxisMaxValue_;
}

void PlotBase::PlotBasePrivate::setYAxisMaxValue(double yAxisMaxValue)
{
  yAxisMaxValue_ = yAxisMaxValue;
}

void PlotBase::PlotBasePrivate::updatePlotRange(const int width, const int height)
{
  calculateTicks();

  QFontMetrics fm(font_);
  maxYtickWidth_ = 0;
  for(const double yTick : majorYticks_)
  {
    double yTickSize = fm.width(QString::number(yTick, 'g', 4));
    maxYtickWidth_ = (maxYtickWidth_ > yTickSize) ? maxYtickWidth_ : yTickSize;
  }

  const double offsetHeight = majorTickLength + 2*fm.height() + 3*textSpacing;
  const double offsetWidth = majorTickLength + fm.height() + maxYtickWidth_ + 3*textSpacing;

  if (std::fabs(aspectRatioPlotArea_) < 1e-5)
  {
    plotRangeTopLeft_ = QPointF(offsetWidth, offsetHeight/3);
    plotRangeBottomRight_ = QPointF(width - offsetWidth/3, height - offsetHeight);
  }
  else
  {
    int widthScaled = aspectRatioPlotArea_ * height;
    int heightScaled = height;
    if (widthScaled > width)
    {
      widthScaled = width;
      heightScaled = width / aspectRatioPlotArea_;
    }

    plotRangeTopLeft_ = QPointF(offsetWidth + std::fabs(width - widthScaled)/2, offsetHeight/3 + (height - heightScaled) / 2);
    plotRangeBottomRight_ = QPointF(widthScaled + std::fabs(width - widthScaled)/2 - offsetWidth/3, heightScaled - offsetHeight + (height - heightScaled) / 2);
  }
}

void PlotBase::PlotBasePrivate::drawBorder(QPainter& painter, const int width, const int height)
{
  painter.save();

  painter.setBrush(Qt::white);
  painter.setPen(Qt::white);

  QPointF p1 = valToPoint(xAxisMinValue_, yAxisMinValue_);
  QPointF p2 = valToPoint(xAxisMaxValue_, yAxisMaxValue_);
  painter.drawRect(0, 0, width, p2.y());
  painter.drawRect(0, 0, p1.x(), height);
  painter.drawRect(0, p1.y(), width, height - p1.y());
  painter.drawRect(p2.x(), 0, width - p2.x(), height);

  painter.restore();
}

void PlotBase::PlotBasePrivate::drawAxes(QPainter& painter)
{
  painter.save();
  QPointF origin(plotRangeTopLeft_.x(), plotRangeBottomRight_.y());
  painter.drawLine(plotRangeTopLeft_, origin);
  painter.drawLine(origin, plotRangeBottomRight_);
  painter.restore();
}

void PlotBase::PlotBasePrivate::drawTicks(QPainter& painter)
{
  painter.save();

  QFontMetrics fm(painter.font());
  QString text;
  for (const double& xTick : majorXticks_)
  {
    const QPointF p1 = valToPoint(xTick, yAxisMinValue_);
    const QPointF p2 = p1 + QPointF(0.0, majorTickLength);
    painter.drawLine(p1, p2);

    text = QString::number(xTick, 'g', 4);
    painter.drawText(p2 + QPointF(-fm.width(text)/2, fm.height() + textSpacing), text);
  }
  for (const double& yTick : majorYticks_)
  {
    const QPointF p1 = valToPoint(xAxisMinValue_, yTick);
    const QPointF p2 = p1 - QPointF(majorTickLength, 0.0);
    painter.drawLine(p1, p2);

    text = QString::number(yTick,'g',4);
    painter.drawText(p2 + QPointF(-(fm.width(text) + textSpacing), fm.height()/4.0), text);
  }
  painter.restore();
}

void PlotBase::PlotBasePrivate::drawLabels(QPainter& painter, const int height)
{
  painter.save();
  QFontMetrics fm(font_);

  const QPointF plotCenter = valToPoint( (xAxisMinValue_+xAxisMaxValue_)/2, (yAxisMinValue_+yAxisMaxValue_)/2);

  const QPointF xLabelPosition( plotCenter.x() - fm.width(xLabel_)/2, plotRangeBottomRight_.y() + textSpacing + 2*fm.height() + textSpacing);
  painter.drawText(xLabelPosition, xLabel_);

  QPointF yLabelPosition( plotRangeTopLeft().x() - textSpacing - fm.height() - fm.width(QString::number(majorYticks_[0],'g',4)), plotCenter.y() + fm.width(yLabel_)/2);
  painter.translate(yLabelPosition);
  painter.rotate(-90);
  painter.drawText(QPoint(0,0), yLabel_);
  painter.restore();
}

void PlotBase::PlotBasePrivate::calculateTicks()
{
  majorXticks_.clear();
  majorYticks_.clear();

  const double xTickSeparation = std::pow(10.0, std::floor(std::log10((xAxisMaxValue_ - xAxisMinValue_)*0.9)));
  const double yTickSeparation = std::pow(10.0, std::floor(std::log10((yAxisMaxValue_ - yAxisMinValue_)*0.9)));

  addBaseTicks(xTickSeparation, yTickSeparation);

  if (majorXticks_.size() <= 1)
  {
    addExtraTicks(majorXticks_, xAxisMinValue_, xAxisMaxValue_, xTickSeparation / 2);
  }

  if (majorYticks_.size() <= 1)
  {
    addExtraTicks(majorYticks_, yAxisMinValue_, yAxisMaxValue_, yTickSeparation / 2);
  }
}

void PlotBase::PlotBasePrivate::addBaseTicks(const double xBase, const double yBase)
{
  for (int i = 0; i <= 20; ++i)
  {
    const double xValue = std::floor(xAxisMinValue_/xBase) * xBase + xBase*i;
    if (xValue >= xAxisMinValue_ && xValue <= xAxisMaxValue_)
    {
      majorXticks_.push_back(xValue);
    }

    const double yValue = std::floor(yAxisMinValue_/yBase) * yBase + yBase*i;
    if (yValue >= yAxisMinValue_ && yValue <= yAxisMaxValue_)
    {
      majorYticks_.push_back(yValue);
    }
  }
}

void PlotBase::PlotBasePrivate::addExtraTicks(QVector<double>& ticks, const double axisMinValue, const double axisMaxValue, const double tickSeparation)
{
  ticks.clear();

  for (int i = 0; i <= 20; ++i)
  {
    const double xValue = std::floor(axisMinValue/tickSeparation) * tickSeparation + tickSeparation*i;
    if (xValue >= axisMinValue && xValue <= axisMaxValue)
    {
      ticks.push_back(xValue);
    }
  }
}

void PlotBase::PlotBasePrivate::simpleTickCalculator(QVector<double>& majorTicks, const double axisMinValue, const double axisMaxValue)
{
  const int n = 5;
  const double range = axisMaxValue - axisMinValue;
  const double stepSize = range/(n-1);
  double tickValue = axisMinValue;

  for (int i = 0; i < n; ++i)
  {
    majorTicks.push_back(tickValue);
    tickValue += stepSize;
  }
}

// PlotBase

PlotBase::PlotBase(QWidget* parent) :
  QWidget(parent),
  plotBasePrivate_{new PlotBasePrivate{}}
{
  setBackgroundRole(QPalette::Base);
  setAutoFillBackground(true);
}

PlotBase::~PlotBase()
{
}

void PlotBase::resizeEvent(QResizeEvent* event)
{
  plotBasePrivate_->updatePlotRange(width(), height());
  QWidget::resizeEvent(event);
}

QSize PlotBase::minimumSizeHint() const
{
  return QSize(100, 100);
}

QSize PlotBase::sizeHint() const
{
  return QSize(800, 800);
}

void PlotBase::paintEvent(QPaintEvent* /*event*/)
{
  QPainter painter(this);
  painter.setFont(plotBasePrivate_->font());

  plotBasePrivate_->updatePlotRange(width(), height());

  plotBasePrivate_->drawBorder(painter, width(), height());
  plotBasePrivate_->drawAxes(painter);
  plotBasePrivate_->drawTicks(painter);
  plotBasePrivate_->drawLabels(painter, height());
  drawData(painter);
}

QPointF PlotBase::valToPoint(double x, double y)
{
  return plotBasePrivate_->valToPoint(x, y);
}

void PlotBase::dataChanged()
{
  updateMinMaxData();
}

void PlotBase::setMinMaxValues(double xMin, double xMax, double yMin, double yMax)
{
  plotBasePrivate_->setXAxisMinValue(xMin);
  plotBasePrivate_->setXAxisMaxValue(xMax);
  plotBasePrivate_->setYAxisMinValue(yMin);
  plotBasePrivate_->setYAxisMaxValue(yMax);
}

double PlotBase::xAxisMinValue() const
{
  return plotBasePrivate_->xAxisMinValue();
}

double PlotBase::xAxisMaxValue() const
{
  return plotBasePrivate_->xAxisMaxValue();
}

double PlotBase::yAxisMinValue() const
{
  return plotBasePrivate_->yAxisMinValue();
}

double PlotBase::yAxisMaxValue() const
{
  return plotBasePrivate_->yAxisMaxValue();
}

void PlotBase::setXLabel(const QString& label)
{
  plotBasePrivate_->setXLabel(label);
}

void PlotBase::setYLabel(const QString& label)
{
  plotBasePrivate_->setYLabel(label);
}

void PlotBase::setAspectRatio(const double aspectRatio)
{
  plotBasePrivate_->setAspectRatio(aspectRatio);
}

void PlotBase::setFontStyle(FontStyle font)
{
  switch (font)
  {
    case FontStyle::small:
      plotBasePrivate_->setFont(QFont("Sans", 8));
      break;
    case FontStyle::normal:
      plotBasePrivate_->setFont(QFont("Sans", 10));
      break;
    case FontStyle::large:
      plotBasePrivate_->setFont(QFont("Sans", 12));
      break;
    default:
      plotBasePrivate_->setFont(QFont("Sans", 10));
      break;
  }
}

} // namespace casaWizard

