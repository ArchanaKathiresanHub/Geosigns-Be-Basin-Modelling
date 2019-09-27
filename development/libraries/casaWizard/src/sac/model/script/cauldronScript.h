// Class for running fastcauldron on a specific folder
#pragma once

#include "model/script/casaScript.h"

namespace casaWizard
{

namespace sac
{

class SACScenario;

class CauldronScript : public CasaScript
{
public:
  explicit CauldronScript(const SACScenario& scenario, const QString& baseDirectory);
  const CasaScenario& scenario() const override;
  QString scriptFilename() const override;
  QString workingDirectory() const override;

private:
  void writeScriptContents(QFile& file) const override;
  bool validateScenario() const override;

  const SACScenario& scenario_;
};

}  // namespace sac

}  // namespace casaWizard
