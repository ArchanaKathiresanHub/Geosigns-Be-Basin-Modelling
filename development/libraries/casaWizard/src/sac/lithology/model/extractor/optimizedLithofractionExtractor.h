// Class to extract the optimized lithofractions from 1D runs using casa
#pragma once

#include "model/input/optimizedLithofractionDataCreator.h"
#include "model/script/optimizedLithofractionScript.h"
#include "model/extractor/caseExtractor.h"

namespace casaWizard
{

namespace sac
{

namespace lithology
{

class LithologyScenario;

class OptimizedLithofractionExtractor : public CaseExtractor
{
public:
  explicit OptimizedLithofractionExtractor(LithologyScenario& scenario);
  ~OptimizedLithofractionExtractor() override;
  RunScript& script() override;
  CaseDataCreator& dataCreator() override;

private:
  void updateCaseScript(const int wellIndex, const QString relativeDataFolder) override;

  OptimizedLithofractionScript script_;
  OptimizedLithofractionDataCreator dataCreator_;
};

} // namespace lithology

} // namespace sac

} // namespace casaWizard
