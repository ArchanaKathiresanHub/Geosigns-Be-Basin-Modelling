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
#include "model/SacScenario.h"
//#include "model/HCP.h"

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
   explicit ThermalScript(const SacScenario& scenario, const QString& baseDirectory, bool doOptimization = true);
   const SacScenario& scenario() const override;

private:
   void writeParameters(QTextStream& stream) const override;

   const SacScenario& m_scenario;
   QString writeHCP(/*const HCP& hcp*/) const;
};

} // thermal

} // sac

} // namespace casaWizard
