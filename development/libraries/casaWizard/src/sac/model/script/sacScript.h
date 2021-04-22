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

class QFile;

namespace casaWizard
{

struct Well;

namespace sac
{

class SACScenario;
struct Lithofraction;
struct WellTrajectory;

class SACScript : public CasaScript
{
public:
  explicit SACScript(const SACScenario& scenario, const QString& baseDirectory, bool doOptimization = true);
  virtual ~SACScript() override = default;

  const CasaScenario& scenario() const override;
  bool prepareKill() const override;
  QString scriptFilename() const override;
  QString workingDirectory() const override;

private:
  const SACScenario& scenario_;
  const bool doOptimization_;

  void writeScriptContents(QFile& file) const override;
  bool validateScenario() const override;

  QString writeWellTrajectory(const WellTrajectory& trajectory, const QString& wellName) const;
  QString writeLithofraction(const Lithofraction& lithofraction) const;
};

} // sac

} // namespace casaWizard
