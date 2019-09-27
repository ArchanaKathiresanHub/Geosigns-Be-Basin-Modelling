#include "dataExtractionController.h"

#include "control/scriptRunController.h"
#include "model/extractor/optimizedLithofractionExtractor.h"
#include "model/extractor/wellTrajectoryExtractor.h"
#include "model/logger.h"
#include "model/output/wellTrajectoryWriter.h"
#include "model/sacScenario.h"
#include "model/scenarioBackup.h"

namespace casaWizard
{

namespace sac
{

DataExtractionController::DataExtractionController(SACScenario& scenario,
                                                   ScriptRunController& scriptRunController,
                                                   QObject* parent) :
  QObject(parent),
  scriptRunController_{scriptRunController},
  scenario_{scenario}
{
}

void DataExtractionController::readResults()
{
  scenarioBackup::backup(scenario_);
  WellTrajectoryExtractor trajectoryExtractor{scenario_};
  readCaseData(trajectoryExtractor, "extracting trajectories using track1d");

  OptimizedLithofractionExtractor lithoExtractor{scenario_};

  if (!lithoExtractor.iterationPath().isEmpty())
  {
    readCaseData(lithoExtractor, "extracting optimized lithofractions");
  }
  else
  {
    Logger::log() << "There are no run cases available! Please run CASA in SAC tab first." << Logger::endl();
  }
}

void DataExtractionController::readCaseData(CaseExtractor& extractor, const QString& message)
{
  Logger::log() << "Start " << message << Logger::endl();
  extractor.extract();

  if (!scriptRunController_.runScript(extractor.script()))
  {
    Logger::log() << "Running failed for unknown reason." << Logger::endl();
    return;
  }

  extractor.readCaseData();
  Logger::log() << "Finished " << message << Logger::endl();
}

} // namespace sac

} // namespace casaWizard


