// Base class for plotting widgets
#pragma once

#include <QWidget>
#include <memory>

class QPointF;

namespace casaWizard
{

enum class FontStyle
{
  small = 0,
  normal = 1,
  large = 2
};

class PlotBase : public QWidget
{
  Q_OBJECT

public:
  explicit PlotBase(QWidget* parent = 0);
  ~PlotBase();

  QSize minimumSizeHint() const override;
  QSize sizeHint() const override;

  double xAxisMinValue() const;
  double xAxisMaxValue() const;
  double yAxisMinValue() const;
  double yAxisMaxValue() const;
  void setXLabel(const QString& label);
  void setYLabel(const QString& label);
  void setAspectRatio(const double aspectRatio);
  void setFontStyle(FontStyle font);

protected:
  virtual void clearData() = 0;
  virtual void drawData(QPainter& painter) = 0;
  virtual void updateMinMaxData() = 0;

  void dataChanged();
  void setMinMaxValues(double xMin, double xMax, double yMin, double yMax);

  QPointF valToPoint(double x, double y) const;
  QPointF pointToVal(double px, double py) const;
  bool validPosition(double px, double py) const;

  void paintEvent(QPaintEvent* event) override;
  void resizeEvent(QResizeEvent* event) override;

private:
  class PlotBasePrivate;
  std::unique_ptr<PlotBasePrivate> plotBasePrivate_;
};

} // namespace casaWizard
