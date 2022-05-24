// Data class for the SAC Wizard
#pragma once

#include "model/casaScenario.h"
#include "model/lithofractionManager.h"
#include "model/MapsManager.h"
#include "model/wellTrajectoryManager.h"

namespace casaWizard
{

class VectorVectorMap;

namespace sac
{

class SACScenario : public CasaScenario
{
public:
  SACScenario(ProjectReader* projectReader);

  QString stateFileNameSAC() const;
  QString calibrationDirectory() const;
  QString optimizedProjectDirectory() const;

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

  bool fitRangeToData() const;
  void setFitRangeToData(const bool fitRangeToData);

  LithofractionManager& lithofractionManager();
  const LithofractionManager& lithofractionManager() const;

  WellTrajectoryManager& wellTrajectoryManager();
  const WellTrajectoryManager& wellTrajectoryManager() const;

  MapsManager& mapsManager();
  const MapsManager& mapsManager() const;

  void writeToFile(ScenarioWriter& writer) const override;
  void readFromFile(const ScenarioReader& reader) override;
  void clear() override;
  QString iterationDirName() const override;

  QVector<bool> activePlots() const;
  void setActivePlots(const QVector<bool>& activePlots);
  void getVisualisationData(QVector<OptimizedLithofraction>& optimizedLithoFractions,
                            QVector<const Well*>& activeAndIncludedWells,
                            const QString& activeLayer) const;
  void exportOptimizedLithofractionMapsToZycor(const QString& targetPath);
  void wellPrepToSAC();
  QVector<int> getIncludedWellIndicesFromSelectedWells(const QVector<int>& selectedWellIndices);
  bool hasOptimizedSuccessfully(const int caseIndex);
  QVector<int> getHighlightedWells(const QVector<int>& selectedWells, const QString& activeLayer);
  bool getLithologyTypesAndMaps(const QString& layerName, std::vector<VectorVectorMap>& lithologyMaps, QStringList& lithologyTypes);
  std::vector<double> getLithopercentagesOfClosestWell(const double xInKm, const double yInKm, const QString& activeLayer, int& closestWellID) const;
  void updateWellsForProject3D();

private:
  QString stateFileNameSAC_;
  QString calibrationDirectory_;
  LithofractionManager lithofractionManager_;
  MapsManager mapsManager_;
  WellTrajectoryManager wellTrajectoryManager_;

  int t2zLastSurface_;
  int t2zReferenceSurface_;
  int t2zSubSampling_;
  bool t2zRunOnOriginalProject_;
  int t2zNumberCPUs_;

  QVector<bool> activePlots_;
  bool showSurfaceLines_;
  bool fitRangeToData_;

  QVector<OptimizedLithofraction> getOptimizedLithoFractionsInLayer(const QString& layer) const;
  bool openMaps(MapReader& mapReader, const int layerID) const;
  bool wellHasDataInActiveLayer(const Well* well, const QString& activeLayer) const;
};

} // namespace sac

} // namespace casaWizard
