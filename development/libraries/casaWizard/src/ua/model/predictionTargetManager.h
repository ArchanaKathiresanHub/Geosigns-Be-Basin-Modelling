// Manager for the prediction targets
#pragma once

#include "predictionTargetDepth.h"
#include "predictionTargetSurface.h"
#include "model/writable.h"

#include <QVector>

namespace casaWizard
{

class ProjectReader;

namespace ua
{

class PredictionTargetManager : public Writable
{
public:
  explicit PredictionTargetManager(const ProjectReader& projectReader);
  ~PredictionTargetManager();

  QVector<PredictionTargetDepth> depthTargets() const;
  QVector<PredictionTargetSurface> surfaceTargets() const;

  QVector<const PredictionTarget*> predictionTargets() const;
  QVector<const PredictionTarget*> predictionTargetsIncludingTimeSeries() const;
  QVector<const PredictionTarget*> predictionTargetInTimeSeries(const int row) const;

  void addDepthTarget(const QString& property, const double x, const double y, const double z, const double age = 0.0);
  void setDepthTarget(int row, int column, const QString& text);
  void copyDepthTarget(int index);
  void removeDepthTarget(int index);

  void addSurfaceTarget(const QString& property, const double x, const double y, QString layer = "", const double age = 0.0);
  void setSurfaceTarget(int row, int column, const QString& text);
  void copySurfaceTarget(int index);
  void removeSurfaceTarget(int index);

  static QStringList predictionTargetOptions();
  void writeToFile(ScenarioWriter& writer) const override;
  void readFromFile(const ScenarioReader& reader) override;
  void clear() override;

  QVector<bool> depthTargetHasTimeSeries() const;  
  QVector<bool> surfaceTargetHasTimeSeries() const;
  void setDepthTargetHasTimeSeries(const int row, const bool isSelected);
  void setSurfaceTargetHasTimeSeries(const int row, const bool isSelected);

  int amountAtAge0() const;
  int amountIncludingTimeSeries() const;
  int sizeOfPredictionTargetWithTimeSeries(const int row) const;
  int indexCumulativePredictionTarget(const int row) const;

  QStringList validLayerNames() const;

private:
  PredictionTargetManager(const PredictionTargetManager&) = delete;
  PredictionTargetManager& operator=(PredictionTargetManager) = delete;

  QVector<int> sizeOfPredictionTargetsWithTimeSeries() const;
  void setPredictionTargetsAllTimes();
  void clearMemory();

  QVector<PredictionTargetDepth> depthTargets_;
  QVector<PredictionTargetSurface> surfaceTargets_;
  QVector<const PredictionTarget*> predictionTargetsAllTimes_;
  static QStringList targetVariables_;
  QVector<bool> depthTargetHasTimeSeries_;
  QVector<bool> surfaceTargetHasTimeSeries_;
  const ProjectReader& projectReader_;
};

} // namespace ua

} // namespace casaWizard

