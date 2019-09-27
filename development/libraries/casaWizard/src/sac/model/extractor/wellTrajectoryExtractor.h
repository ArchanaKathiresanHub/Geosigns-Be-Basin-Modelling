// Function to read the well trajectory data using track1d
#pragma once

#include "caseExtractor.h"
#include "model/input/wellTrajectoryDataCreator.h"
#include "model/script/track1dScript.h"

#include <QString>
#include <QVector>

namespace casaWizard
{

class Well;

namespace sac
{

class SACScenario;

class WellTrajectoryExtractor : public CaseExtractor
{
public:
  explicit WellTrajectoryExtractor(SACScenario& scenario);
  RunScript& script() override;
  CaseDataCreator& dataCreator() override;

private:
  void updateCaseScript(const int wellIndex, const QString relativeDataFolder) override;
  Track1DScript script_;
  WellTrajectoryDataCreator dataCreator_;
};

} // namespace sac

} // namespace casaWizard
