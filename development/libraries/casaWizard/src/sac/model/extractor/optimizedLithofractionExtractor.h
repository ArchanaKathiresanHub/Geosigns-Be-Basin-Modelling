// Class to extract the optimized lithofractions from 1D runs using casa
#pragma once

#include "caseExtractor.h"
#include "model/input/optimizedLithofractionDataCreator.h"
#include "model/script/optimizedLithofractionScript.h"

namespace casaWizard
{

namespace sac
{

class SACScenario;

class OptimizedLithofractionExtractor : public CaseExtractor
{
public:
  explicit OptimizedLithofractionExtractor(SACScenario& scenario);
  ~OptimizedLithofractionExtractor() override;
  RunScript& script() override;
  CaseDataCreator& dataCreator() override;

private:
  void updateCaseScript(const int wellIndex, const QString relativeDataFolder) override;

  OptimizedLithofractionScript script_;
  OptimizedLithofractionDataCreator dataCreator_;
};

} // namespace sac

} // namespace casaWizard
