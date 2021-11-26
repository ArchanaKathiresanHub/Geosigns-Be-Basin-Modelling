// Script to run the sensitivity analysis
#pragma once

#include "uaScript.h"

class QFile;

namespace casaWizard
{

namespace ua
{

class UAScenario;

class QCScript : public UAScript
{
public:
  explicit QCScript(const UAScenario& scenario);
  QString scriptFilename() const final;

private:
  void writeScriptContents(QFile& file) const final;
};

} // namespace ua

} // namespace casaWizard
