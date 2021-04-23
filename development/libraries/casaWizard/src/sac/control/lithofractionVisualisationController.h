//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include <QObject>

namespace casaWizard
{

class CalibrationTargetManager;
class CMBMapReader;
class VectorVectorMap;

namespace sac
{

class LithofractionVisualisation;
class OptimizedLithofraction;
class SACScenario;

class LithofractionVisualisationController : public QObject
{
  Q_OBJECT
public:
  LithofractionVisualisationController(LithofractionVisualisation* lithofractionVisualisation,
                                       SACScenario& scenario,
                                       QObject* parent);

  void updateAvailableLayers();
  void updateBirdsView();

private slots:
  void slotRefresh();

private:
  QString activeLayer_;
  LithofractionVisualisation* lithofractionVisualisation_;
  SACScenario& scenario_;

  void connectToolTipSlots();  
  QVector<OptimizedLithofraction> getOptimizedLithoFractionsInActiveLayer(const casaWizard::CalibrationTargetManager& ctManager);
  QStringList obtainAvailableLayers() const;
  std::vector<VectorVectorMap> obtainLithologyMaps(const CMBMapReader& mapReader, int layerID) const;
  QStringList obtainLithologyTypes(const int layerID) const;
  bool openMaps(CMBMapReader& mapReader, const int layerID) const;

private slots:
  void slotUpdatePlots(const QString& layerName);
  void toolTipCreated(const QPointF& point, const int plotID);
};

} // namespace sac

} // namespace casaWizard
