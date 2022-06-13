#include "mcmcController.h"

#include "control/casaScriptWriter.h"
#include "control/scriptRunController.h"
#include "model/calibrationTargetManager.h"
#include "model/functions/rmseCalibrationTargets.h"
#include "model/input/dataFileParser.h"
#include "model/input/mcDataCreator.h"
#include "model/input/projectReader.h"
#include "model/logger.h"
#include "model/output/McmcOutputDataCollector.h"
#include "model/output/McmcOutputWriter.h"
#include "model/output/McmcTargetExportData.h"
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
#include <QFileDialog>
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
   m_mcmcTab{mcmcTab},
   m_casaScenario{casaScenario},
   m_scriptRunController{scriptRunController}
{
   connect(parent, SIGNAL(signalUpdateTabGUI(int)), this, SLOT(slotUpdateTabGUI(int)));

   connect(m_mcmcTab->pushButtonUArunCASA(),             SIGNAL(clicked()),            this, SLOT(slotPushButtonMCMCrunCasaClicked()));
   connect(m_mcmcTab->tablePredictionTargets(),          SIGNAL(cellClicked(int,int)), this, SLOT(slotTablePredictionTargetClicked(int, int)));
   connect(m_mcmcTab->pushButtonExportOptimalCase(),     SIGNAL(clicked()),            this, SLOT(slotPushButtonExportOptimalCasesClicked()));
   connect(m_mcmcTab->pushButtonRunOptimalCase(),        SIGNAL(clicked()),            this, SLOT(slotPushButtonRunOptimalCasesClicked()));
   connect(m_mcmcTab->pushButtonExportMcmcResults(),     SIGNAL(clicked()),            this, SLOT(slotPushButtonExportMcmcOutputClicked()));
   connect(m_mcmcTab->pushButtonAddOptimalDesignPoint(), SIGNAL(clicked()),            this, SLOT(slotPushButtonAddOptimalDesignPointClicked()));
   connect(m_mcmcTab->sliderHistograms(),                SIGNAL(valueChanged(int)),    this, SLOT(slotSliderHistogramsChanged(int)));
   connect(m_mcmcTab->checkBoxHistoryPlotsMode(),        SIGNAL(stateChanged(int)),    this, SLOT(slotCheckBoxHistoryPlotModeChanged(int)));

   //Optimal case is only defined if there are calibration targets.
   if (m_casaScenario.calibrationTargetManager().amountOfActiveCalibrationTargets() == 0)
   {
      m_mcmcTab->setOptimalCaseButtonsDisabled();
   }
}

void MCMCController::refreshGUI()
{
   const PredictionTargetManager& predictionTargetManager = m_casaScenario.predictionTargetManager();
   const MonteCarloDataManager& monteCarloData = m_casaScenario.monteCarloDataManager();

   if (m_casaScenario.calibrationTargetManager().amountOfActiveCalibrationTargets() == 0)
   {
      m_mcmcTab->setOptimalCaseButtonsDisabled();
   }
   else
   {
      m_mcmcTab->setOptimalCaseButtonsEnabled();
   }

   if (m_casaScenario.isStageComplete(StageTypesUA::mcmc))
   {
      m_mcmcTab->setL2norm(monteCarloData.rmseOptimalRunCase());
      m_mcmcTab->setL2normRS(m_casaScenario.responseSurfacesL2NormBestMC());
      m_mcmcTab->fillPredictionTargetTable(predictionTargetManager.predictionTargets());
   }
   else
   {
      m_mcmcTab->setL2norm(-9999);
      m_mcmcTab->setL2normRS(-9999);
      m_mcmcTab->fillPredictionTargetTable({});
   }
   m_mcmcTab->updateHistogram(nullptr, {{0}}, {});
}

void MCMCController::slotUpdateTabGUI(int tabID)
{
   if (tabID != static_cast<int>(TabID::MCMC))
   {
      return;
   }

   m_mcmcTab->setEnabled(m_casaScenario.isStageComplete(StageTypesUA::doe) &&
                         m_casaScenario.isStageComplete(StageTypesUA::qc));

   if (!m_casaScenario.isStageComplete(StageTypesUA::qc))
   {
      Logger::log() << "QC data is not available! Complete QC stage in QC tab first." << Logger::endl();
   }
   else if (!m_casaScenario.isStageComplete(StageTypesUA::mcmc))
   {
      Logger::log() << "MCMC data creation stage is not completed! Run MCMC to complete it." << Logger::endl();
   }

   refreshGUI();
}

void MCMCController::slotPushButtonMCMCrunCasaClicked()
{
   scenarioBackup::backup(m_casaScenario);
   McmcScript mcmc{m_casaScenario};
   if (!casaScriptWriter::writeCasaScript(mcmc))
   {
      return;
   }
   if (m_scriptRunController.runScript(mcmc))
   {
      try
      {
         mcDataCreator::setData(m_casaScenario);
         const PredictionTargetManager& manager = m_casaScenario.predictionTargetManager();
         m_mcmcTab->fillPredictionTargetTable(manager.predictionTargets());
         m_mcmcTab->setL2normRS(m_casaScenario.responseSurfacesL2NormBestMC());
         scenarioBackup::backup(m_casaScenario);
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

   if (QFile::copy(m_casaScenario.workingDirectory() + "/" + m_casaScenario.stateFileNameMCMC() ,
                   m_casaScenario.workingDirectory() + "/" + m_casaScenario.runLocation() + "/" + m_casaScenario.iterationDirName() + "/" + m_casaScenario.stateFileNameMCMC()))
   {
      if (!QFile::remove( m_casaScenario.workingDirectory() + "/" + m_casaScenario.stateFileNameMCMC()))
      {
         Logger::log() << "There was a problem while moving " << m_casaScenario.stateFileNameMCMC() << " file to " << m_casaScenario.iterationDirName() << " Folder." << Logger::endl();
      }
   }

   m_casaScenario.setStageComplete(StageTypesUA::mcmc);

   scenarioBackup::backup(m_casaScenario);

   slotUpdateTabGUI(static_cast<int>(TabID::MCMC));
}

void MCMCController::slotPushButtonExportOptimalCasesClicked()
{
   scenarioBackup::backup(m_casaScenario);

   OptimalCaseScript optimal{m_casaScenario};
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

   const RunCaseSetFileManager& rcsFileManager = m_casaScenario.runCaseSetFileManager();
   if (!casaScriptWriter::writeCasaScriptFilterOutDataDir(optimal, rcsFileManager.caseSetDirPath()) ||
       !m_scriptRunController.runScript(optimal))
   {
      return;
   }
   scenarioBackup::backup(m_casaScenario);
}

void MCMCController::slotPushButtonRunOptimalCasesClicked()
{
   scenarioBackup::backup(m_casaScenario);

   RunOptimalCaseScript optimal{m_casaScenario};
   if (!casaScriptWriter::writeCasaScript(optimal) ||
       !m_scriptRunController.runScript(optimal))
   {
      return;
   }

   QString filePath = optimal.absoluteDirectory() + m_casaScenario.runCasesObservablesTextFileName();

   QVector<QString> colNames;
   QVector<double> observableValues;
   try
   {
      observableValues = DataFileParser<double>::parseFileWithHeaderRowDominant(filePath,colNames)[0];
   }
   catch (const std::exception& e)
   {
      Logger::log() << e.what() << Logger::endl();
   }

   m_casaScenario.setOptimalValuesTargetQCs(observableValues,colNames);

   QVector<int> calibrationTargetIndices = m_casaScenario.calibrationDataObservablesIndexRange();
   if (calibrationTargetIndices.size() == 0 || calibrationTargetIndices.back()-1 > observableValues.size())
   {
      Logger::log() << "Calculation of RMSE failed due to insufficient calibration targets." << Logger::endl();
   }

   QVector<double> calibrationTargets;
   for (int idx : calibrationTargetIndices)
   {
      calibrationTargets.push_back(observableValues.at(idx));
   }

   const double L2norm = functions::rmseCalibrationTargets(calibrationTargets, m_casaScenario.calibrationTargetManager());
   MonteCarloDataManager& manager = m_casaScenario.monteCarloDataManager();
   manager.setRmseOptimalRunCase(L2norm);
   m_mcmcTab->setL2norm(L2norm);
   scenarioBackup::backup(m_casaScenario);
}

void MCMCController::slotPushButtonAddOptimalDesignPointClicked()
{
   ManualDesignPointManager& manualDesignPointManager = m_casaScenario.manualDesignPointManager();
   const MonteCarloDataManager& monteCarloDataManager = m_casaScenario.monteCarloDataManager();

   QVector<double> optimalMCPoint = monteCarloDataManager.getPoint(0);
   manualDesignPointManager.addDesignPoint(optimalMCPoint);

   Logger::log() << "Adding the design point ( ";
   for (const double d : optimalMCPoint)
   {
      Logger::log() << QString::number(d) << " ";
   }
   Logger::log() << ") as a manual design point. The additional case needs to be run in design of experiment." << Logger::endl();

   m_casaScenario.setNumberOfManualDesignPoints();
   m_casaScenario.changeUserDefinedPointStatus(true);
   m_casaScenario.setStageComplete(StageTypesUA::doe, false);
   m_casaScenario.setStageComplete(StageTypesUA::qc, false);
   m_casaScenario.setStageComplete(StageTypesUA::mcmc, false);

   scenarioBackup::backup(m_casaScenario);
}

void MCMCController::slotPushButtonExportMcmcOutputClicked()
{
   McmcTargetExportData exportData = McmcOutputDataCollector::collectMcmcOutputData(m_casaScenario);

   if (exportData.targetData.empty())
   {
      Logger::log() << "No data to export." << Logger::endl();
      return;
   }
   QString fileName = QFileDialog::getSaveFileName(m_mcmcTab, "Save as", QDir::currentPath(), "comma separated file (*.csv)");
   if (fileName == "")
   {
      return;
   }

   if (!fileName.endsWith(".csv"))
   {
      fileName += ".csv";
   }

   McmcOutputWriter::writeToFile(fileName,exportData);
}

void MCMCController::slotTablePredictionTargetClicked(int row, int /*column*/)
{
   if (row < 0)
   {
      return;
   }

   const MonteCarloDataManager& monteCarloData = m_casaScenario.monteCarloDataManager();

   if (!monteCarloData.predictionTargetMatrix().empty())
   {
      if (m_mcmcTab->checkBoxHistoryPlotsMode()->checkState() == Qt::CheckState::Checked)
      {
         const PredictionTargetManager& predictionTargetManager = m_casaScenario.predictionTargetManager();
         if (!predictionTargetManager.targetHasTimeSeries()[row])
         {
            Logger::log() << "This target does not have a time series, so the time series plot is empty";
            m_mcmcTab->clearTimeSeriesPlots();
            return;
         }

         QVector<double> snapshotAges;
         QMap<QString,QVector<double>> bestMatchedValuesPerProperty; // Initialize one best matched values vector per property
         QMap<QString,QVector<QVector<double>>> currentPredTargetMatrixPerProperty; // Initialize one prediction matrix per property, indexed in a map
         m_casaScenario.obtainTimeSeriesMonteCarloData(row, snapshotAges, bestMatchedValuesPerProperty, currentPredTargetMatrixPerProperty);

         m_mcmcTab->updateTimeSeriesPlot(snapshotAges, currentPredTargetMatrixPerProperty, bestMatchedValuesPerProperty, predictionTargetManager.predictionTargetOptions());
      }
      else
      {
         QVector<QVector<double>> data;
         m_casaScenario.obtainMonteCarloDataForTimeStep(row, 0.0, data);

         const PredictionTargetManager& predictionTargetManager = m_casaScenario.predictionTargetManager();
         const PredictionTarget* const targetTimeSeries = predictionTargetManager.predictionTargetInTimeSeries(row)[0];
         m_mcmcTab->updateHistogram(targetTimeSeries, data, predictionTargetManager.predictionTargetOptions());
         m_mcmcTab->updateSliderHistograms(predictionTargetManager.targetHasTimeSeries()[row] ? m_casaScenario.projectReader().agesFromMajorSnapshots().size() : 0);
      }
   }
}

void MCMCController::slotSliderHistogramsChanged(int indexTime)
{
   const MonteCarloDataManager& monteCarloData = m_casaScenario.monteCarloDataManager();

   if (!monteCarloData.predictionTargetMatrix().empty())
   {
      const int row = m_mcmcTab->tablePredictionTargets()->currentRow();
      QVector<QVector<double>> data;
      m_casaScenario.obtainMonteCarloDataForTimeStep(row, indexTime, data);

      const PredictionTargetManager& predictionTargetManager = m_casaScenario.predictionTargetManager();
      const PredictionTarget* const targetTimeSeries = predictionTargetManager.predictionTargetInTimeSeries(row)[indexTime];

      m_mcmcTab->updateHistogram(targetTimeSeries, data, predictionTargetManager.predictionTargetOptions());
   }
}

void MCMCController::slotCheckBoxHistoryPlotModeChanged(const int checkState)
{
   m_mcmcTab->setEnableTimeSeries(checkState);
   slotTablePredictionTargetClicked(m_mcmcTab->tablePredictionTargets()->currentRow(), 0);
}

} // namespace ua

} // namespace casaWizard
