//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#pragma once

#include "uaScript.h"

namespace casaWizard
{

namespace ua
{

class CollectObservablesScript : public UAScript
{
public:
   CollectObservablesScript(UAScenario& scenario);
   QString scriptFilename() const final;

private:
   void writeScriptContents(QFile& file) const final;
};

} // namespace ua

} // namespace casaWizard
