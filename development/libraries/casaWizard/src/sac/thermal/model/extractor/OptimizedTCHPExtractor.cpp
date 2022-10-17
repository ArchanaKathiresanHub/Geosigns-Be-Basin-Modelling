#include "OptimizedTCHPExtractor.h"
#include "model/ThermalScenario.h"

namespace casaWizard
{

namespace sac
{

namespace thermal
{

OptimizedTCHPExtractor::OptimizedTCHPExtractor(ThermalScenario& scenario) :
   CaseExtractor(scenario),
   m_script{iterationPath()},
   m_dataCreator{scenario, iterationPath()}
{}

RunScript& OptimizedTCHPExtractor::script()
{
   return m_script;
}

CaseDataCreator& OptimizedTCHPExtractor::dataCreator()
{
   return m_dataCreator;
}

void OptimizedTCHPExtractor::updateCaseScript(const int /*wellIndex*/, const QString relativeDataFolder)
{
   m_script.addCase(relativeDataFolder);
}

} // namespace thermal

} // namespace sac

} // namespace casaWizard
