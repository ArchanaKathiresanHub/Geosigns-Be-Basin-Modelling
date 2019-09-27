// Class for creating the optimized 3D case
#pragma once

#include "model/script/casaScript.h"

namespace casaWizard
{

namespace sac
{

class SACScenario;

class SaveOptimizedScript : public CasaScript
{
public:
  explicit SaveOptimizedScript(const SACScenario& scenario);

  const CasaScenario& scenario() const override;
  QString scriptFilename() const;
  QString workingDirectory() const override;

private:
  void writeScriptContents(QFile& file) const override;
  bool validateScenario() const  override;

  const SACScenario& scenario_;
};

} // namespace sac

} // namespace casaWizard
