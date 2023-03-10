// Read data files which contain the optimized lithofractions
#pragma once

#include "model/input/caseDataCreator.h"

namespace casaWizard
{

namespace sac
{

namespace lithology
{

class LithofractionManager;
class LithologyScenario;

class OptimizedLithofractionDataCreator : public CaseDataCreator
{
public:
  explicit OptimizedLithofractionDataCreator(LithologyScenario& scenario, const QString& iterationPath);

private:
  void readCase(const int wellIndex, const int caseIndex);

  LithofractionManager& lithofractionManager_;
};

} // namespace lithology

} // namespace sac

} // namespace casaWizard
