#include "legend.h"
#include "plot.h"

#include <QPainter>

namespace casaWizard
{

Legend::Legend(const casaWizard::PlotSettings& plotSettings, const QVector<PlotData>& plotData, const QStringList& entries, QWidget* parent) :
  QWidget(parent),
  plotSettings_{plotSettings},
  entries_{entries},
  plotData_{plotData}
{

}

void Legend::paintEvent(QPaintEvent* /*event*/)
{
  QPainter painter(this);
  const double horizontalSpacing = 20;
  const int nNames = entries_.size();

  QPen pen = painter.pen();
  pen.setWidth(plotSettings_.lineWidth);

  QFontMetrics fm(painter.font());
  double maxLegendWidth = 0;
  for( int i = 0; i<nNames; ++i)
  {
    const double w = fm.horizontalAdvance(entries_[i]);
    maxLegendWidth = (w>maxLegendWidth)?w:maxLegendWidth;
  }

  const QPointF h(horizontalSpacing, 0);
  QPointF previousPoint(0, height()/2);
  int iEntry = 0;
  for (const PlotData& data : plotData_)
  {
    if (data.lineType == LineType::errorBarHorizontal || data.seriesName != "")
    {
      continue;
    }
    QPointF y = previousPoint + h;

    pen.setColor(Qt::black);
    painter.setPen(pen);
    if (iEntry<entries_.size())
    {
      painter.drawText(y + h*3/2 + QPointF(0, fm.height()/4), entries_[iEntry]);
    }

    pen.setColor(data.color);
    painter.setPen(pen);
    switch (data.lineType)
    {
      case LineType::line:
        painter.drawLine(y, y + h);
        break;
      case LineType::xyScatter:
        Plot::plotSymbol(painter, y + h/2, plotSettings_.lineWidthSymbol, plotSettings_.markerSize, data.symbolType);
        break;
    }

    previousPoint = y + h*3/2 + QPointF(fm.horizontalAdvance(entries_[iEntry]),0);

    ++iEntry;
  }
}

}
