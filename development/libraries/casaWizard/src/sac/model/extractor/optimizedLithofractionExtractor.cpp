#include "optimizedLithofractionExtractor.h"

namespace casaWizard
{

namespace sac
{

OptimizedLithofractionExtractor::OptimizedLithofractionExtractor(SACScenario& scenario) :
  CaseExtractor(scenario),
  script_{iterationPath()},
  dataCreator_{scenario, iterationPath()}
{  
}

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
