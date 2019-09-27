// Abstract class for data creators. This class iterates over the Case_X directories a specific Iteration_N
// The data is stored in the SACScenario.

#pragma once

#include <QString>
#include <QVector>

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
  void read();

protected:
  const CalibrationTargetManager& calibrationTargetManager() const;
  const QString& iterationPath() const;

private:
  virtual void readCase(const int wellIndex, const int caseIndex) = 0;

  const CalibrationTargetManager& calibrationTargetManager_;
  const QString iterationPath_;
};

} // namespace sac

} // namespace casaWizard
