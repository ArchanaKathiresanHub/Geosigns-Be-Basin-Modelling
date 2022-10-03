//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// Script to run the design of experiment
#pragma once

#include "model/script/sacScript.h"
#include "model/sacLithologyScenario.h"

class QFile;

namespace casaWizard
{

namespace sac
{

class Lithofraction;

class SACLithologyScript : public SACScript
{
public:
   explicit SACLithologyScript(const SacLithologyScenario& scenario, const QString& baseDirectory, bool doOptimization = true);
   const SacLithologyScenario& scenario() const override;

private:
   void writeScriptContents(QFile& file) const override;
   QString writeLithofraction(const Lithofraction& lithofraction) const;

   const SacLithologyScenario& m_scenario;
};

} // sac

} // namespace casaWizard
