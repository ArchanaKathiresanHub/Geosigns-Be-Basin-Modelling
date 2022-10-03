//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// Function to read the well trajectory data using track1d
#pragma once

#include "caseExtractor.h"
#include "model/input/wellTrajectoryDataCreator.h"
#include "model/script/track1dScript.h"

#include <QString>

namespace casaWizard
{

namespace sac
{

class SacScenario;

class WellTrajectoryExtractor : public CaseExtractor
{
public:
  explicit WellTrajectoryExtractor(SacScenario& scenario, const QString& projectName, const QString& iterationPath = "");
  ~WellTrajectoryExtractor() override;
  RunScript& script() override;
  CaseDataCreator& dataCreator() override;

private:
  void updateCaseScript(const int wellIndex, const QString relativeDataFolder) override;

  Track1DScript m_script;
  WellTrajectoryDataCreator m_dataCreator;
};

} // namespace sac

} // namespace casaWizard
