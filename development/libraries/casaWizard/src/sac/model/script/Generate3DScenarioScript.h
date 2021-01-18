// Class for creating the optimized 3D case
#pragma once

#include "model/script/casaScript.h"

namespace casaWizard
{

namespace sac
{

class SACScenario;

class Generate3DScenarioScript : public CasaScript
{
public:
  explicit Generate3DScenarioScript(const SACScenario& scenario);

  const CasaScenario& scenario() const override;
  QString scriptFilename() const override;
  QString workingDirectory() const override;

private:
  void writeScriptContents(QFile& file) const override;
  bool validateScenario() const  override;
  QString setFilterOneDResults() const;
  QString generateThreeDFromOneD() const;

  const SACScenario& scenario_;
};

} // namespace sac

} // namespace casaWizard
