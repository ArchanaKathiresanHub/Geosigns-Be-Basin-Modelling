//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// Data class for the SAC Wizard
#pragma once

#include "model/lithofractionManager.h"
#include "model/SacScenario.h"

namespace casaWizard
{

class VectorVectorMap;

namespace sac
{

class SacLithologyScenario : public SacScenario
{
public:
  SacLithologyScenario(ProjectReader* projectReader);

  int t2zReferenceSurface() const;
  void setT2zReferenceSurface(int t2zReferenceSurface);

  int t2zLastSurface() const;
  void updateT2zLastSurface();

  int t2zNumberCPUs() const;
  void setT2zNumberCPUs(int t2zNumberCPUs);

  int t2zSubSampling() const;
  void setT2zSubSampling(int t2zSubSampling);

  bool t2zRunOnOriginalProject() const;
  void setT2zRunOnOriginalProject(bool t2zRunOnOriginalProject);

  bool showSurfaceLines() const;
  void setShowSurfaceLines(const bool showSurfaceLines);


  LithofractionManager& lithofractionManager();
  const LithofractionManager& lithofractionManager() const;

  void writeToFile(ScenarioWriter& writer) const override;
  void readFromFile(const ScenarioReader& reader) override;
  void clear() override;

  void getVisualisationData(QVector<OptimizedLithofraction>& optimizedLithoFractions,
                            QVector<const Well*>& activeAndIncludedWells,
                            const QString& activeLayer) const;
  void exportOptimizedLithofractionMapsToZycor(const QString& targetPath);

  QVector<int> getHighlightedWells(const QVector<int>& selectedWells, const QString& activeLayer);
  bool getLithologyTypesAndMaps(const QString& layerName, std::vector<VectorVectorMap>& lithologyMaps, QStringList& lithologyTypes);
  std::vector<double> getLithopercentagesOfClosestWell(const double xInKm, const double yInKm, const QString& activeLayer, int& closestWellID) const;

private:
  LithofractionManager lithofractionManager_;

  int t2zLastSurface_;
  int t2zReferenceSurface_;
  int t2zSubSampling_;
  bool t2zRunOnOriginalProject_;
  int t2zNumberCPUs_;

  bool showSurfaceLines_;

  QVector<OptimizedLithofraction> getOptimizedLithoFractionsInLayer(const QString& layer) const;
  bool wellHasDataInActiveLayer(const Well* well, const QString& activeLayer) const;
};

} // namespace sac

} // namespace casaWizard
