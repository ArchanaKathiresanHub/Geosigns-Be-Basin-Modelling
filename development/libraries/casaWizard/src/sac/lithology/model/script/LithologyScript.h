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
#include "model/LithologyScenario.h"

class QFile;

namespace casaWizard
{

namespace sac
{

class Lithofraction;

class LithologyScript : public SACScript
{
public:
   explicit LithologyScript(const LithologyScenario& scenario, const QString& baseDirectory, bool doOptimization = true);
   const LithologyScenario& scenario() const override;

private:
   void writeParameters(QTextStream& stream) const override;
   QString writeLithofraction(const Lithofraction& lithofraction) const;

   const LithologyScenario& m_scenario;
};

} // sac

} // namespace casaWizard
