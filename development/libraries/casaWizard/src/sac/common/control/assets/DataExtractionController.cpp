#include "DataExtractionController.h"

#include "control/ScriptRunController.h"
#include "model/extractor/wellTrajectoryExtractor.h"
#include "model/logger.h"
#include "model/output/wellTrajectoryWriter.h"
#include "model/SacScenario.h"

#include <QFileInfo>

namespace casaWizard
{

namespace sac
{

DataExtractionController::DataExtractionController(SacScenario& scenario,
                                                   ScriptRunController& scriptRunController,
                                                   QObject* parent) :
   QObject(parent),
   scenario_(scenario),
   scriptRunController_{scriptRunController}
{}

SacScenario& DataExtractionController::scenario()
{
   return scenario_;
}

ScriptRunController& DataExtractionController::scriptRunController()
{
   return scriptRunController_;
}

void DataExtractionController::readOriginalResults()
{
   const QString projectName = QFileInfo(scenario().project3dPath()).baseName();
   const QString iterationPath = scenario().original1dDirectory() + "/" + scenario().runLocation() + "/Iteration_1";

   WellTrajectoryExtractor trajectoryExtractor(scenario(), projectName, iterationPath);
   readCaseData(trajectoryExtractor, "extracting original trajectories using track1d");
}

void DataExtractionController::readOptimizedResults(CaseExtractor& caseExtractor)
{
   WellTrajectoryExtractor trajectoryExtractor(scenario(), "bestMatchedCase");
   readCaseData(trajectoryExtractor, "extracting optimized trajectories using track1d");

   if (caseExtractor.iterationPath().isEmpty())
   {
      Logger::log() << "There are no run cases available! Please run 1D optimization in 'Input' tab first." << Logger::endl();
      return;
   }

   readCaseData(caseExtractor, "extracting optimized results");
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


