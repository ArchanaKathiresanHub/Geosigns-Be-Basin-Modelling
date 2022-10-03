//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// Script to run the design of experiment
#pragma once

#include "model/script/casaScript.h"
#include "model/SacScenario.h"

namespace casaWizard
{

namespace sac
{

class SACScript : public CasaScript
{
public:
   explicit SACScript(const QString& baseDirectory, bool doOptimization = true);
   virtual ~SACScript() override = default;

   virtual const SacScenario& scenario() const override = 0;
   QString scriptFilename() const override;
   QString workingDirectory() const override;

protected:  
   bool createStopExecFile() const override;
   QString writeWellTrajectory(const QString& wellName, const int wellIndex, const QString& propertyUserName) const;
   bool doOptimization() const;

private:
   const bool doOptimization_;
   bool validateScenario() const override;
};

} // sac

} // namespace casaWizard
