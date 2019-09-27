// Birds eye view for the wells, with the option to select those
#pragma once

#include "view/plot/plot.h"

namespace casaWizard
{

namespace sac
{

class WellBirdsView : public Plot
{
  Q_OBJECT

public:
  explicit WellBirdsView(QWidget* parent = 0);

  void setWellLocations(const QVector<double> x, const QVector<double> y);
  void setActiveWells(const QVector<int> activeWells);

private:
  QVector<double> x_;
  QVector<double> y_;
};

}  // namespace sac

}  // namespace casaWizard
