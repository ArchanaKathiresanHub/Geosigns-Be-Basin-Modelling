//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

class QString;

namespace casaWizard
{

class ScriptRunController;

namespace sac
{

class SacScenario;

class Run3dCaseController
{
public:
  explicit Run3dCaseController(SacScenario& scenario,
                               ScriptRunController& scriptRunController);

  bool run3dCase(const QString& directory, const bool isOptimized);

private:
  SacScenario& scenario_;
  ScriptRunController& scriptRunController_;  

  bool import3dWellData(const QString& baseDirectory, const bool isOptimized);
};

} // namespace sac

} // namespace casaWizard
