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

class StageStateUA : public Writable
{
public:
  StageStateUA(const QString stateName = "isStageComplete", bool state = false);

  void writeToFile(ScenarioWriter& writer) const override;
  void readFromFile(const ScenarioReader& reader) override;
  void clear() override;

  bool isTrue(const StageTypesUA& stageType) const;
  void setAllToTrue();
  void setStageState(const StageTypesUA& stageType, const bool isTrue = true);

private:
  QVector<bool> m_stageStates;
  QString m_stateName;
};

} // namespace ua

} // namespace casaWizard
