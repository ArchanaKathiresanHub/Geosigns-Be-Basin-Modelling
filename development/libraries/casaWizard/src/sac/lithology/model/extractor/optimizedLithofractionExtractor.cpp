#include "optimizedLithofractionExtractor.h"
#include "model/sacLithologyScenario.h"

namespace casaWizard
{

namespace sac
{

OptimizedLithofractionExtractor::OptimizedLithofractionExtractor(SacLithologyScenario& scenario) :
  CaseExtractor(scenario),
  script_{iterationPath()},
  dataCreator_{scenario, iterationPath()}
{}

OptimizedLithofractionExtractor::~OptimizedLithofractionExtractor()
{}

RunScript& OptimizedLithofractionExtractor::script()
{
  return script_;
}

CaseDataCreator& OptimizedLithofractionExtractor::dataCreator()
{
  return dataCreator_;
}

void OptimizedLithofractionExtractor::updateCaseScript(const int /*wellIndex*/, const QString relativeDataFolder)
{
  script_.addCase(relativeDataFolder);
}

} // namespace sac

} // namespace casaWizard
