// Read data files which contain the optimized lithofractions
#pragma once

#include "model/input/caseDataCreator.h"

namespace casaWizard
{

namespace sac
{

class LithofractionManager;
class SacLithologyScenario;

class OptimizedLithofractionDataCreator : public CaseDataCreator
{
public:
  explicit OptimizedLithofractionDataCreator(SacLithologyScenario& scenario, const QString& iterationPath);

private:
  void readCase(const int wellIndex, const int caseIndex);

  LithofractionManager& lithofractionManager_;
};

} // namespace sac

} // namespace casaWizard
