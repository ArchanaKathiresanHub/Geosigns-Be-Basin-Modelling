// Base class for generating data (using a RunScript) and collecting the generated data (using a CaseDataCreator).
// The class will iterate over the Case_X directories in the last Iteration_N directory.
#pragma once

#include <QString>

namespace casaWizard
{

class RunScript;
class Well;

namespace sac
{

class SACScenario;
class CaseDataCreator;

class CaseExtractor
{
public:
  explicit CaseExtractor(const SACScenario& scenario, const QString& iterationPath = "");
  virtual ~CaseExtractor();

  void extract();
  void readCaseData();
  QString iterationPath() const;
  const SACScenario& scenario();
  virtual RunScript& script() = 0;
  virtual CaseDataCreator& dataCreator() = 0;

private:
  void extractCase(const Well* well, const int caseIndex);
  void obtainIterationPath();
  virtual void updateCaseScript(const int wellIndex, const QString relativeDataFolder) = 0;

  QString iterationPath_;
  const SACScenario& scenario_;
};

} // namespace sac

} // namespace casaWizard
