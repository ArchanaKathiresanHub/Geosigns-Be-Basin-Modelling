// Read data files which contain the optimized TCHP
#pragma once

#include "model/input/caseDataCreator.h"

namespace casaWizard
{

namespace sac
{

namespace thermal
{

class ThermalScenario;

class TCHPManager;

class OptimizedTCHPDataCreator : public CaseDataCreator
{
public:
  explicit OptimizedTCHPDataCreator(ThermalScenario& scenario, const QString& iterationPath);

private:
  void readCase(const int wellIndex, const int caseIndex);

  TCHPManager& m_TCHPManager;
};

} // namespace thermal

} // namespace sac

} // namespace casaWizard
