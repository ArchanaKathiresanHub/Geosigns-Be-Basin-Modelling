// Birds eye view for the wells, with the option to select those
#pragma once

#include "view/plot/plot.h"
#include "QtCharts/QChart"

namespace casaWizard
{

namespace sac
{


class WellBirdsView : public Plot
{
  Q_OBJECT

public:
  explicit WellBirdsView(QWidget* parent = nullptr);

  void setWellLocations(const QVector<double>& x, const QVector<double>& y);
  void setSelectedWells(const QVector<int> selectedWells);
  const QVector<int>& selectedWells();
  const QVector<double>& x() const;
  const QVector<double>& y() const;

  void updateRange(const double xMin, const double xMax, const double yMin, const double yMax);
  void updateMinMaxData() override {}
private:
  QVector<double> x_;
  QVector<double> y_;
  QVector<int> selectedWells_;

  QVector<double> convertToKm(const QVector<double>& distancesInMeter);
};

}  // namespace sac

}  // namespace casaWizard
