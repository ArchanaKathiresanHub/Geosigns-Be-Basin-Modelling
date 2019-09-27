// Read data files which contain the optimized lithofractions
#pragma once

#include "caseDataCreator.h"

namespace casaWizard
{

namespace sac
{

class LithofractionManager;
class SACScenario;

class OptimizedLithofractionDataCreator : public CaseDataCreator
{
public:
  explicit OptimizedLithofractionDataCreator(SACScenario& scenario, const QString& iterationPath);

private:
  void readCase(const int wellIndex, const int caseIndex);

  LithofractionManager& lithofractionManager_;
};

} // namespace sac

} // namespace casaWizard
