#include "dataExtractionController.h"

#include "control/scriptRunController.h"
#include "model/extractor/optimizedLithofractionExtractor.h"
#include "model/extractor/wellTrajectoryExtractor.h"
#include "model/logger.h"
#include "model/output/wellTrajectoryWriter.h"
#include "model/sacScenario.h"

#include <QFileInfo>

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

void DataExtractionController::readOriginalResults()
{
  const QString projectName = QFileInfo(scenario_.project3dPath()).baseName();
  const QString iterationPath = scenario_.original1dDirectory() + "/" + scenario_.runLocation() + "/Iteration_1";

  WellTrajectoryExtractor trajectoryExtractor(scenario_, projectName, iterationPath);
  readCaseData(trajectoryExtractor, "extracting original trajectories using track1d");
}

void DataExtractionController::readOptimizedResults()
{  
  WellTrajectoryExtractor trajectoryExtractor(scenario_, "bestMatchedCase");
  readCaseData(trajectoryExtractor, "extracting optimized trajectories using track1d");

  OptimizedLithofractionExtractor lithoExtractor{scenario_};

  if (lithoExtractor.iterationPath().isEmpty())
  {
    Logger::log() << "There are no run cases available! Please run 1D optimization in 'Input' tab first." << Logger::endl();
    return;
  }

  readCaseData(lithoExtractor, "extracting optimized lithofractions");
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


