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

class LithofractionVisualisation;
class OptimizedLithofraction;
class SacLithologyScenario;

class LithofractionVisualisationController : public SacVisualisationController
{
  Q_OBJECT
public:
  LithofractionVisualisationController(LithofractionVisualisation* lithofractionVisualisation,
                                       SacLithologyScenario& scenario,
                                       QObject* parent);

  void updateAvailableLayers();
  void updateBirdsView() override;
  void updateSelectedWells(QVector<int> selectedWells) override;
  void hideAllTooltips() override;

private slots:
  void slotRefresh() override;

private:
  QString activeLayer_;
  bool singleMapLayout_;
  LithofractionVisualisation* lithofractionVisualisation_;
  SacLithologyScenario& scenario_;

  void connectToolTipSlots();  
  QString currentlyDisplayedLithotype() const;
  QVector<OptimizedLithofraction> getOptimizedLithoFractionsInLayer(const QString& layer) const;
  std::vector<double> getLithopercentagesOfClosestWell(const QPointF& point, int& closestWellID) const;
  std::vector<double> getLithopercentagesAtLocation(const QPointF& point) const;
  bool lithofractionIsValid(const double lithofractionAtPoint) const;
  QStringList obtainAvailableLayers() const;

private slots:
  void slotUpdatePlots(const QString& layerName);
  void toolTipCreated(const QPointF& point, const int plotID);
  void slotUpdateSingleMapLayout(int state);
};

} // namespace sac

} // namespace casaWizard
