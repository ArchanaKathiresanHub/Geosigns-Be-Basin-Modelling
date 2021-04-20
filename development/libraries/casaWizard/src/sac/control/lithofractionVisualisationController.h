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
class MapReader;
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
  QStringList obtainAvailableLayers();
  std::vector<VectorVectorMap> obtainLithologyMaps(const MapReader& mapReader, int layerID);
  QStringList obtainLithologyTypes(const int layerID);
  bool openMaps(MapReader& mapReader, const int layerID);

private slots:
  void slotUpdatePlots(const QString& layerName);
  void toolTipCreated(const QPointF& point, const int plotID);
};

} // namespace sac

} // namespace casaWizard
