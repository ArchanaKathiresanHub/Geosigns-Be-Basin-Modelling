// Class to extract the optimized lithofractions from 1D runs using casa
#pragma once

#include "model/input/optimizedLithofractionDataCreator.h"
#include "model/script/optimizedLithofractionScript.h"
#include "model/extractor/caseExtractor.h"

namespace casaWizard
{

namespace sac
{

class SacLithologyScenario;

class OptimizedLithofractionExtractor : public CaseExtractor
{
public:
  explicit OptimizedLithofractionExtractor(SacLithologyScenario& scenario);
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
