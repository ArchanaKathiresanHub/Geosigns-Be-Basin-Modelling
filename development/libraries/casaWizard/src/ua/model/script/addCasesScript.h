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

private:
  void writeScriptContents(QFile& file) const override;
};

} // namespace ua

} // namespace casaWizard
