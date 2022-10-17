// Class to extract the optimized heat crust production from 1D runs using casa
#pragma once

#include "model/input/OptimizedTCHPDataCreator.h"
#include "model/script/OptimizedTCHPScript.h"
#include "model/extractor/caseExtractor.h"

namespace casaWizard
{

namespace sac
{

class SacScenario;

namespace thermal
{

class OptimizedTCHPExtractor : public CaseExtractor
{
public:
  explicit OptimizedTCHPExtractor(ThermalScenario& scenario);
  ~OptimizedTCHPExtractor() = default;
  RunScript& script() override;
  CaseDataCreator& dataCreator() override;

private:
  void updateCaseScript(const int wellIndex, const QString relativeDataFolder) override;

  OptimizedTCHPScript m_script;
  OptimizedTCHPDataCreator m_dataCreator;
};

}

} // namespace sac

} // namespace casaWizard
