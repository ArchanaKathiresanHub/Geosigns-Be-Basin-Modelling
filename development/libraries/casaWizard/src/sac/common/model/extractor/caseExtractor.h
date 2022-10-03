//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// Base class for generating data (using a RunScript) and collecting the generated data (using a CaseDataCreator).
// The class will iterate over the Case_X directories in the last Iteration_N directory.
#pragma once

#include <QString>

namespace casaWizard
{

class Well;
class RunScript;

namespace sac
{

class CaseDataCreator;
class SacScenario;

class CaseExtractor
{
public:
  explicit CaseExtractor(const SacScenario& scenario, const QString& iterationPath = "");
  virtual ~CaseExtractor();

  void extract();
  void readCaseData();
  QString iterationPath() const;
  const SacScenario& scenario();
  virtual RunScript& script() = 0;
  virtual CaseDataCreator& dataCreator() = 0;

private:
  void extractCase(const Well* well, const int caseIndex);
  void obtainIterationPath();
  virtual void updateCaseScript(const int wellIndex, const QString relativeDataFolder) = 0;

  QString m_iterationPath;
  const SacScenario& m_scenario;
};

} // namespace sac

} // namespace casaWizard
