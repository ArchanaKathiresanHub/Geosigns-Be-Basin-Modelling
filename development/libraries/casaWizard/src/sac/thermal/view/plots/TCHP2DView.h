//
// Copyright (C) 2012-2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// Widget to display 2D view of grids
#pragma once

#include "view/plots/SacMap2DView.h"
#include "view/assets/TCHPMapToolTip.h"


#include "vector"

namespace casaWizard
{

class ColorMap;

namespace sac
{

namespace thermal
{

class OptimizedTCHP;

class TCHP2DView : public SacMap2DView
{
  Q_OBJECT

public:
  explicit TCHP2DView(const ColorMap& colormap, QWidget* parent = nullptr);

  void setOptimizedTCHPs(const QVector<OptimizedTCHP>& OptimizedTCHPs);

  void setToolTipData(const double& TCHPAtPoint, const QString& wellName);
  void finalizeTooltip(const double& TCHPAtPoint, const QString& wellName);

  void drawData(QPainter &painter) override;

private:
  TCHPMapToolTip* mapToolTip() final;
  TCHPMapToolTip* m_tchpMapToolTip;

  QVector<OptimizedTCHP> m_optimizedTCHPs;

  void determineRange() final;

  void drawPoint(QPainter& painter,
                 const int size,
                 const int counter,
                 const OptimizedTCHP& optTCHP);
  void drawPointsSelectedWells(QPainter& painter);
  void drawPointsWells(QPainter& painter);
};

} // namespace thermal

} // namespace sac

} // namespace casaWizard
