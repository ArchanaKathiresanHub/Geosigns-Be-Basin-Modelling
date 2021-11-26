// Script to run only the uncompleted design points
#pragma once

#include "model/script/uaScript.h"

namespace casaWizard
{

namespace ua
{

class AddCasesScript : public UAScript
{
public:
  explicit AddCasesScript(const UAScenario& scenario); 
  QString scriptFilename() const final;

private:
  void writeScriptContents(QFile& file) const final;
};

} // namespace ua

} // namespace casaWizard
