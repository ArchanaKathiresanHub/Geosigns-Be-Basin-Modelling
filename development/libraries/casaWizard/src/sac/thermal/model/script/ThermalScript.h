//
// Copyright (C) 2012-2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// Script to run the design of experiment
#pragma once

#include "model/script/sacScript.h"
#include "model/ThermalScenario.h"

class QFile;

namespace casaWizard
{

class Well;

namespace sac
{

class WellTrajectory;

namespace thermal
{

class ThermalScript : public SACScript
{
public:
   explicit ThermalScript(const ThermalScenario& scenario,
                          const QString& baseDirectory,
                          bool doOptimization = true);
   const SacScenario& scenario() const override;

private:
   void writeParameters(QTextStream& stream) const override;

   const ThermalScenario& m_scenario;
};

} // thermal

} // sac

} // namespace casaWizard
