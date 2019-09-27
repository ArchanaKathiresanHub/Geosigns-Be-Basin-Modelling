#include "correlationPlot.h"

#include "model/influentialParameter.h"

#include <QPainter>
#include <QPointF>
#include <QRectF>

namespace casaWizard
{

namespace ua
{

const int symbolSize = 5;

CorrelationPlot::CorrelationPlot(QWidget* parent) :
  Plot(parent),
  hasData_{false}
{
}

void CorrelationPlot::setCorrelations(const QVector<double>& parameterX,
                                      const QVector<double>& parameterY,
                                      const QVector<double>& parameterZ)
{
  if (parameterZ.isEmpty())
  {
    hasData_ = false;
    return;
  }

  clearData();
  addXYscatter(parameterX, parameterY, parameterZ);

  setMinMaxZvalues(*std::min_element(parameterZ.begin(), parameterZ.end()),
                   *std::max_element(parameterZ.begin(), parameterZ.end()));

  hasData_ = true;
}

void CorrelationPlot::paintEvent(QPaintEvent* event)
{
  QPainter painter(this);
  if (!hasData_)
  {
    painter.drawText(rect(), QString("No data found, please run casa in UA tab"), QTextOption(Qt::AlignCenter));
    return;
  }

  // Also execute the parent paint event
  Plot::paintEvent(event);
}

} // namespace ua

} // namespace casaWizard
