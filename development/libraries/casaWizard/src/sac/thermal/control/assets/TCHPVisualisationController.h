//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include "control/assets/SacVisualisationController.h"

namespace casaWizard
{

class CalibrationTargetManager;
class CMBMapReader;
class VectorVectorMap;
class Well;

namespace sac
{

namespace thermal
{

class TCHPVisualisation;
class OptimizedTCHP;
class ThermalScenario;

class TCHPVisualisationController : public SacVisualisationController
{
   Q_OBJECT
public:
   TCHPVisualisationController(TCHPVisualisation* TCHPVisualisation,
                               ThermalScenario& scenario,
                               QObject* parent);

   void updateBirdsView() override;
   void updateSelectedWells(QVector<int> selectedWells) override;
   void hideAllTooltips() override;

private:
   TCHPVisualisation* TCHPVisualisation_;
   ThermalScenario& scenario_;

   QString currentlyDisplayedLithotype() const;
   QVector<OptimizedTCHP> getOptimizedTCHPsInLayer(const QString& layer) const;
   double getTCHPOfClosestWell(const QPointF& point, int& closestWellID) const;
   double getTCHPAtLocation(const QPointF& point) const;
   bool TCHPIsValid(const double TCHPAtPoint) const;

private slots:
   void slotRefresh() override;
   void slotUpdatePlot();
   void createToolTip(const QPointF& point);
};

} // namespace thermal

} // namespace sac

} // namespace casaWizard
