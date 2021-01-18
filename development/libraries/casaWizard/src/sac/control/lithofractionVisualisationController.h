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

class LithofractionMap;
class MapReader;

namespace sac
{

class LithofractionVisualisation;
class SACScenario;

class LithofractionVisualisationController : public QObject
{
  Q_OBJECT
public:
  LithofractionVisualisationController(LithofractionVisualisation* lithofractionVisualisation,
                                       SACScenario& scenario,
                                       QObject* parent);

  void updateAvailableLayers();

private:
  SACScenario& scenario_;
  LithofractionVisualisation* lithofractionVisualisation_;

  QVector<QVector<double> > convertToQData(const std::vector<std::vector<double> >& mapData);
  std::vector<LithofractionMap> obtainLithologyMaps(const MapReader& mapReader, int layerID);
  bool openMaps(MapReader& mapReader, const int layerID);
  QStringList obtainAvailableLayers();
  QStringList obtainLithologyTypes(const int layerID);

private slots:
  void slotUpdatePlots(const QString& layerName);
};

} // namespace sac

} // namespace casaWizard
