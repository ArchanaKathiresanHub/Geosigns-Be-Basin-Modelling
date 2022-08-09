// Contains enum class for thermal UA analysis stages including doe, qc, and mcmc, and
// struct for data showing whether or not UA stages are completed.
#pragma once

#include "model/writable.h"

#include <QVector>

namespace casaWizard
{

namespace ua
{

enum class StageTypesUA
{
  doe = 0,
  responseSurfaces = 1,
  mcmc = 2,
  Count = 3 // holds number of stages
};

class StageCompletionUA : public Writable
{
public:
  StageCompletionUA();

  void writeToFile(ScenarioWriter& writer) const override;
  void readFromFile(const ScenarioReader& reader) override;
  void clear() override;

  bool isComplete(const StageTypesUA& stageType) const;
  void setAllToCompleted();
  void setStageIsComplete(const StageTypesUA& stageType, const bool isComplete = true);

private:
  QVector<bool> isComplete_;
};

} // namespace ua

} // namespace casaWizard
