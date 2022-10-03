//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// Abstract class for data creators. This class iterates over the Case_X directories a specific Iteration_N
// The data is stored in the SACScenario.

#pragma once

#include <QString>

namespace casaWizard
{

class CalibrationTargetManager;
class Well;

namespace sac
{

class CaseDataCreator
{
public:
  explicit CaseDataCreator(const CalibrationTargetManager& calibrationTargetManager, const QString& iterationPath);
  virtual ~CaseDataCreator() = default;
  void read();

protected:
  const CalibrationTargetManager& calibrationTargetManager() const;
  const QString& iterationPath() const;

private:
  virtual void readCase(const int wellIndex, const int caseIndex) = 0;

  const CalibrationTargetManager& m_calibrationTargetManager;
  const QString m_iterationPath;
};

} // namespace sac

} // namespace casaWizard
