#include "mcmcController.h"

#include "control/casaScriptWriter.h"
#include "control/scriptRunController.h"
#include "model/calibrationTargetManager.h"
#include "model/functions/rmseCalibrationTargets.h"
#include "model/input/dataFileParser.h"
#include "model/input/mcDataCreator.h"
#include "model/input/projectReader.h"
#include "model/logger.h"
#include "model/output/runCaseSetFileManager.h"
#include "model/scenarioBackup.h"
#include "model/script/optimalCaseScript.h"
#include "model/script/runOptimalCaseScript.h"
#include "model/script/mcmcScript.h"
#include "model/stagesUA.h"
#include "model/uaScenario.h"
#include "view/mcmcTab.h"
#include "view/uaTabIDs.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDir>
#include <QMessageBox>
#include <QPushButton>
#include <QString>
#include <QTableWidgetItem>

namespace casaWizard
{

namespace ua
{

MCMCController::MCMCController(MCMCTab* mcmcTab,
                               UAScenario& casaScenario,
                               ScriptRunController& scriptRunController,
                               QObject* parent) :
  QObject(parent),
  mcmcTab_{mcmcTab},
  casaScenario_{casaScenario},
  scriptRunController_{scriptRunController}
{
  connect(mcmcTab_->pushButtonUArunCASA(),             SIGNAL(clicked()),            this, SLOT(slotPushButtonMCMCrunCasaClicked()));
  connect(mcmcTab_->tablePredictionTargets(),          SIGNAL(cellClicked(int,int)), this, SLOT(slotTablePredictionTargetClicked(int, int)));
  connect(mcmcTab_->pushButtonExportOptimalCase(),     SIGNAL(clicked()),            this, SLOT(slotPushButtonExportOptimalCasesClicked()));
  connect(mcmcTab_->pushButtonRunOptimalCase(),        SIGNAL(clicked()),            this, SLOT(slotPushButtonRunOptimalCasesClicked()));
  connect(mcmcTab_->pushButtonAddOptimalDesignPoint(), SIGNAL(clicked()),            this, SLOT(slotPushButtonAddOptimalDesignPointClicked()));
  connect(mcmcTab_->sliderHistograms(),                SIGNAL(valueChanged(int)),    this, SLOT(slotSliderHistogramsChanged(int)));
  connect(mcmcTab_->checkBoxHistoryPlotsMode(),        SIGNAL(stateChanged(int)),    this, SLOT(slotCheckBoxHistoryPlotModeChanged(int)));
}

void MCMCController::slotRefresh()
{
  const PredictionTargetManager& predictionTargetManager = casaScenario_.predictionTargetManager();
  const MonteCarloDataManager& monteCarloData = casaScenario_.monteCarloDataManager();

  if (casaScenario_.isStageComplete(StageTypesUA::mcmc))
  {
    mcmcTab_->setL2norm(monteCarloData.rmseOptimalRunCase());
    mcmcTab_->setL2normRS(casaScenario_.responseSurfacesL2NormBestMC());
    mcmcTab_->fillPredictionTargetTable(predictionTargetManager.predictionTargets());
  }
  else
  {
    mcmcTab_->setL2norm(-9999);
    mcmcTab_->setL2normRS(-9999);
    mcmcTab_->fillPredictionTargetTable({});
  }
  mcmcTab_->updateHistogram(nullptr, {0});
}

void MCMCController::slotUpdateTabGUI(int tabID)
{
  if (tabID != static_cast<int>(TabID::MCMC))
  {
    return;
  }

  mcmcTab_->setEnabled(casaScenario_.isStageComplete(StageTypesUA::doe) &&
                       casaScenario_.isStageComplete(StageTypesUA::qc));

  if (!casaScenario_.isStageComplete(StageTypesUA::qc))
  {
    Logger::log() << "QC data is not available! Complete QC stage in QC tab first." << Logger::endl();
  }
  else if (!casaScenario_.isStageComplete(StageTypesUA::mcmc))
  {
    Logger::log() << "MCMC data creation stage is not completed! Run MCMC to complete it." << Logger::endl();
  }

  slotRefresh();
}

void MCMCController::slotPushButtonMCMCrunCasaClicked()
{
  scenarioBackup::backup(casaScenario_);
  McmcScript mcmc{casaScenario_};
  if (!casaScriptWriter::writeCasaScript(mcmc))
  {
    return;
  }
  if (scriptRunController_.runScript(mcmc))
  {
    try
    {
      mcDataCreator::setData(casaScenario_);
      const PredictionTargetManager& manager = casaScenario_.predictionTargetManager();
      mcmcTab_->fillPredictionTargetTable(manager.predictionTargets());
      mcmcTab_->setL2normRS(casaScenario_.responseSurfacesL2NormBestMC());
      scenarioBackup::backup(casaScenario_);
    }
    catch (const std::exception& e)
    {
      Logger::log() << "Failed to import MCMC data file: " << e.what() << Logger::endl();
    }
    catch (...)
    {
      Logger::log() << "Failed to import MCMC data file with unknown cause." << Logger::endl();
    }
  }

  if (QFile::copy(casaScenario_.workingDirectory() + "/" + casaScenario_.stateFileNameMCMC() ,
                  casaScenario_.workingDirectory() + "/" + casaScenario_.runLocation() + "/" + casaScenario_.iterationDirName() + "/" + casaScenario_.stateFileNameMCMC()))
  {
    if (!QFile::remove( casaScenario_.workingDirectory() + "/" + casaScenario_.stateFileNameMCMC()))
    {
      Logger::log() << "There was a problem while moving " << casaScenario_.stateFileNameMCMC() << " file to " << casaScenario_.iterationDirName() << " Folder." << Logger::endl();
    }
  }

  casaScenario_.setStageComplete(StageTypesUA::mcmc);

  scenarioBackup::backup(casaScenario_);

  slotUpdateTabGUI(static_cast<int>(TabID::MCMC));
}

void MCMCController::slotPushButtonExportOptimalCasesClicked()
{
  scenarioBackup::backup(casaScenario_);

  OptimalCaseScript optimal{casaScenario_};
  const QString directory{optimal.optimalCaseDirectory()};

  if (QDir(directory).exists())
  {
    QMessageBox messageBox;
    messageBox.setText("Directory: \"" + directory + "\" exists.");
    messageBox.setInformativeText("Do you want to override it?");
    messageBox.setStandardButtons(QMessageBox::Cancel | QMessageBox::Ok);
    if (messageBox.exec() == QMessageBox::Cancel)
    {
      return;
    }
  }

  const RunCaseSetFileManager& rcsFileManager = casaScenario_.runCaseSetFileManager();
  if (casaScriptWriter::writeCasaScriptFilterOutDataDir(optimal, rcsFileManager.caseSetDirPath()))
  {
    scriptRunController_.runScript(optimal);
    scenarioBackup::backup(casaScenario_);
  }
}

void MCMCController::slotPushButtonRunOptimalCasesClicked()
{
  scenarioBackup::backup(casaScenario_);

  RunOptimalCaseScript optimal{casaScenario_};
  if (!casaScriptWriter::writeCasaScript(optimal))
  {
    return;
  }
  if (scriptRunController_.runScript(optimal))
  {
    DataFileParser<double> fileParser(optimal.absoluteDirectory() + casaScenario_.runCasesObservablesTextFileName());
    const double L2norm = functions::rmseCalibrationTargets(fileParser.rowDominantMatrix()[0], casaScenario_.calibrationTargetManager());
    MonteCarloDataManager& manager = casaScenario_.monteCarloDataManager();
    manager.setRmseOptimalRunCase(L2norm);
    mcmcTab_->setL2norm(L2norm);
    scenarioBackup::backup(casaScenario_);
  }
}

void MCMCController::slotPushButtonAddOptimalDesignPointClicked()
{
  ManualDesignPointManager& manualDesignPointManager = casaScenario_.manualDesignPointManager();
  const MonteCarloDataManager& monteCarloDataManager = casaScenario_.monteCarloDataManager();

  QVector<double> optimalMCPoint = monteCarloDataManager.getPoint(0);
  manualDesignPointManager.addDesignPoint(optimalMCPoint);

  Logger::log() << "Adding the design point ( ";
  for (const double d : optimalMCPoint)
  {
    Logger::log() << QString::number(d) << " ";
  }
  Logger::log() << ") as a manual design point. The additional case needs to be run in design of experiment." << Logger::endl();

  casaScenario_.setNumberOfManualDesignPoints();
  casaScenario_.changeUserDefinedPointStatus(true);
  casaScenario_.setStageComplete(StageTypesUA::doe, false);
  casaScenario_.setStageComplete(StageTypesUA::qc, false);
  casaScenario_.setStageComplete(StageTypesUA::mcmc, false);

  scenarioBackup::backup(casaScenario_);
}

void MCMCController::slotTablePredictionTargetClicked(int row, int /*column*/)
{
  if (row < 0)
  {
    return;
  }

  const MonteCarloDataManager& monteCarloData = casaScenario_.monteCarloDataManager();

  if (!monteCarloData.predictionTargetMatrix().empty())
  {
    const PredictionTargetManager& predictionTargetManager = casaScenario_.predictionTargetManager();
    const PredictionTarget* const targetTimeSeries = predictionTargetManager.predictionTargetInTimeSeries(row)[0];

    if (mcmcTab_->checkBoxHistoryPlotsMode()->checkState() == Qt::CheckState::Checked)
    {
      const ProjectReader& projReader = casaScenario_.projectReader();
      const QVector<QVector<double>>& predTargetMatrix = monteCarloData.predictionTargetMatrix();
      const QVector<QVector<double>> currentPredTargetMatrix = predTargetMatrix.mid(predictionTargetManager.indexCumulativePredictionTarget(row)
                                                                                    , predictionTargetManager.sizeOfPredictionTargetWithTimeSeries(row));
      QVector<double> bestMatchedValues;
      for (const QVector<double>& predTarget : currentPredTargetMatrix)
      {
        bestMatchedValues.push_back(predTarget[0]);
      }

      mcmcTab_->updateTimeSeriesPlot(*targetTimeSeries, projReader.agesFromMajorSnapshots(), currentPredTargetMatrix, bestMatchedValues);
    }
    else
    {
      mcmcTab_->updateHistogram(targetTimeSeries, monteCarloData.predictionTargetMatrix()[predictionTargetManager.indexCumulativePredictionTarget(row)]);
      mcmcTab_->updateSliderHistograms(predictionTargetManager.sizeOfPredictionTargetWithTimeSeries(row));
    }
    scenarioBackup::backup(casaScenario_);
  }
}

void MCMCController::slotSliderHistogramsChanged(int indexTime)
{
  const MonteCarloDataManager& monteCarloData = casaScenario_.monteCarloDataManager();

  if (!monteCarloData.predictionTargetMatrix().empty())
  {
    const int row = mcmcTab_->tablePredictionTargets()->currentRow();
    const PredictionTargetManager& predictionTargetManager = casaScenario_.predictionTargetManager();
    const PredictionTarget* const targetTimeSeries = predictionTargetManager.predictionTargetInTimeSeries(row)[indexTime];

    mcmcTab_->updateHistogram(targetTimeSeries, monteCarloData.predictionTargetMatrix()[predictionTargetManager.indexCumulativePredictionTarget(row) + indexTime]);
  }
}

void MCMCController::slotCheckBoxHistoryPlotModeChanged(const int checkState)
{
  mcmcTab_->setEnableTimeSeries(checkState);
  slotTablePredictionTargetClicked(mcmcTab_->tablePredictionTargets()->currentRow(), 0);
}

} // namespace ua

} // namespace casaWizard
