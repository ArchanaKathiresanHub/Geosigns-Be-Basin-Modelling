//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "UAResultsController.h"

#include "control/casaScriptWriter.h"
#include "control/scriptRunController.h"
#include "control/UAResultsTargetTableController.h"

#include "model/calibrationTargetManager.h"
#include "model/functions/folderOperations.h"
#include "model/functions/rmseCalibrationTargets.h"
#include "model/input/dataFileParser.h"
#include "model/input/mcDataCreator.h"
#include "model/input/projectReader.h"
#include "model/doeOption.h"
#include "model/logger.h"
#include "model/output/McmcOutputDataCollector.h"
#include "model/output/McmcOutputWriter.h"
#include "model/output/McmcTargetExportData.h"
#include "model/output/runCaseSetFileManager.h"
#include "model/output/OptimalCaseExporter.h"
#include "model/scenarioBackup.h"
#include "model/script/addCasesScript.h"
#include "model/script/optimalCaseScript.h"
#include "model/script/runOptimalCaseScript.h"
#include "model/script/mcmcScript.h"
#include "model/stagesUA.h"
#include "model/uaScenario.h"
#include "view/UAResultsTab.h"
#include "view/UAResultsTargetTable.h"
#include "view/uaTabIDs.h"


#include "model/script/qcScript.h"
#include "model/input/targetQCdataCreator.h"


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

UAResultsController::UAResultsController(UAResultsTab* uaResultsTab,
                                         UAScenario& casaScenario,
                                         ScriptRunController& scriptRunController,
                                         QObject* parent) :
   QObject(parent),
   m_uaResultsTab{uaResultsTab},
   m_casaScenario{casaScenario},
   m_scriptRunController{scriptRunController},
   m_targetTableController{m_uaResultsTab->targetTable()}
{
   connect(parent, SIGNAL(signalUpdateTabGUI(int)), this, SLOT(slotUpdateTabGUI(int)));

   connect(m_uaResultsTab->pushButtonUArunCASA(),             SIGNAL(clicked()),            this, SLOT(slotPushButtonMCMCrunCasaClicked()));
   connect(&m_targetTableController,                          SIGNAL(targetClicked(int)),   this, SLOT(slotPredictionTargetClicked(int)));
   connect(m_uaResultsTab->pushButtonExportOptimalCase(),     SIGNAL(clicked()),            this, SLOT(slotPushButtonExportOptimalCasesClicked()));
   connect(m_uaResultsTab->pushButtonRunOptimalCase(),        SIGNAL(clicked()),            this, SLOT(slotPushButtonRunOptimalCasesClicked()));
   connect(m_uaResultsTab->pushButtonExportMcmcResults(),     SIGNAL(clicked()),            this, SLOT(slotPushButtonExportMcmcOutputClicked()));
   connect(m_uaResultsTab->pushButtonRecalcResponseSurfaces(), SIGNAL(clicked()),           this, SLOT(slotPushButtonRecalculateResponseSurfacesClicked()));
   connect(m_uaResultsTab->sliderHistograms(),                SIGNAL(valueChanged(int)),    this, SLOT(slotSliderHistogramsChanged(int)));
   connect(m_uaResultsTab->checkBoxHistoryPlotsMode(),        SIGNAL(stateChanged(int)),    this, SLOT(slotCheckBoxHistoryPlotModeChanged(int)));

   //Optimal case is only defined if there are calibration targets.
   if (m_casaScenario.calibrationTargetManager().amountOfActiveCalibrationTargets() == 0)
   {
      m_uaResultsTab->setOptimalCaseButtonsDisabled();
   }
}

void UAResultsController::refreshGUI()
{
   const PredictionTargetManager& predictionTargetManager = m_casaScenario.predictionTargetManager();
   if (m_casaScenario.isStageComplete(StageTypesUA::mcmc))
   {
      m_uaResultsTab->disableAll(false);
      bool buttonEnabled = !(m_casaScenario.isStageUpToDate(StageTypesUA::responseSurfaces) && m_casaScenario.isStageUpToDate(StageTypesUA::mcmc));
      m_uaResultsTab->setRecalcResponseSurfacesButtonEnabled(buttonEnabled);
      m_uaResultsTab->setUArunCASAButtonEnabled(false);

      if (!m_targetTableController.hasData()) //To make sure it only updates the data after loading from autosave.
      {
         m_targetTableController.setTableData(predictionTargetManager.predictionTargets());
      }
      slotPredictionTargetClicked(m_targetTableController.currentTarget());

      if (m_casaScenario.calibrationTargetManager().amountOfActiveCalibrationTargets() == 0)
      {
         m_uaResultsTab->setOptimalCaseButtonsDisabled();
      }
      else
      {
         showOptimalCaseQuality();

         QStringList parameterNames;
         QVector<double> optimalValues = getOptimalSettings(parameterNames);
         m_uaResultsTab->displayOptimalParameterValues(optimalValues,parameterNames);

         m_uaResultsTab->setOptimalCaseButtonsEnabled();
      }
   }
   else
   {
      m_uaResultsTab->disableAll();
      m_uaResultsTab->clearOptimalResultsLayout();
      m_uaResultsTab->clearOptimalCaseQuality();
      m_uaResultsTab->setUArunCASAButtonEnabled();
      m_targetTableController.setTableData({});
      m_uaResultsTab->updateHistogram(nullptr, {{0}}, {});
   }

}

void UAResultsController::loadObservablesOptimalRun()
{
   QVector<QString> colNames;
   QVector<double> observableValues;

   QString filePath = m_casaScenario.workingDirectory() + "/" + m_casaScenario.runCasesObservablesTextFileName();
   observableValues = DataFileParser<double>::parseMatrixFileWithHeaderRowDominant(filePath,colNames).back();
   m_casaScenario.setOptimalValuesTargetQCs(observableValues,colNames);

   QVector<int> calibrationTargetIndices = m_casaScenario.calibrationDataObservablesIndexRange();
   if (calibrationTargetIndices.size() == 0 || calibrationTargetIndices.back()-1 > observableValues.size())
   {
      Logger::log() << "Calculation of RMSE failed due to insufficient calibration targets." << Logger::endl();
      return;
   }

   QVector<double> calibrationTargets;
   for (int idx : calibrationTargetIndices)
   {
      calibrationTargets.push_back(observableValues.at(idx));
   }

   const double rmse = functions::rmseCalibrationTargets(calibrationTargets, m_casaScenario.calibrationTargetManager());
   MonteCarloDataManager& manager = m_casaScenario.monteCarloDataManager();
   manager.setRmseOptimalRunCase(rmse);
}

void UAResultsController::showOptimalCaseQuality()
{
   QStringList parNames{"RMSE response surface"};
   QVector<double> rmses{m_casaScenario.responseSurfacesL2NormBestMC()};
   double rsmeOptimalRun = m_casaScenario.monteCarloDataManager().rmseOptimalRunCase();
   if (rsmeOptimalRun > 0.0)
   {
      rmses.push_back(rsmeOptimalRun);
      parNames.append("RMSE optimal simulation");
   }

   m_uaResultsTab->displayOptimalCaseQuality(rmses,parNames);
}

QVector<double> UAResultsController::getOptimalSettings(QStringList& parameterNames) const
{
   const InfluentialParameterManager& influentialParameterManager = m_casaScenario.influentialParameterManager();
   parameterNames = influentialParameterManager.nameList();
   return influentialParameterManager.optimalParameterValues();
}

void UAResultsController::slotUpdateTabGUI(int tabID)
{
   if (tabID != static_cast<int>(TabID::MCMC))
   {
      return;
   }

   m_uaResultsTab->setEnabled(m_casaScenario.isStageComplete(StageTypesUA::doe) &&
                              m_casaScenario.isStageComplete(StageTypesUA::responseSurfaces));

   if (!m_casaScenario.isStageComplete(StageTypesUA::responseSurfaces))
   {
      Logger::log() << "QC data is not available! Complete QC stage in QC tab first." << Logger::endl();
   }
   else if (!m_casaScenario.isStageComplete(StageTypesUA::mcmc))
   {
      Logger::log() << "MCMC data creation stage is not completed! Run MCMC to complete it." << Logger::endl();
   }

   refreshGUI();
}

void UAResultsController::slotProjectOpened()
{
   m_targetTableController.setTableData({}); //Data in table should be loaded when opening another project or autosave.
}

void UAResultsController::slotPushButtonMCMCrunCasaClicked()
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
         m_targetTableController.setTableData(manager.predictionTargets());
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

void UAResultsController::slotPushButtonExportOptimalCasesClicked()
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

   if (!casaScriptWriter::writeCasaScript(optimal) ||
       !m_scriptRunController.runScript(optimal))
   {
      return;
   }

   optimalCaseExporter::exportOptimalCase(optimal.optimalCaseDirectory(),m_casaScenario.workingDirectory());

   scenarioBackup::backup(m_casaScenario);
}

void UAResultsController::slotPushButtonRunOptimalCasesClicked()
{
   scenarioBackup::backup(m_casaScenario);

   //Qc options selected is reset when changing the doe's retain the selected cases to reselect upon recalculation of response surfaces.
   QStringList selectedQcOptions = m_casaScenario.qcDoeOptionSelectedNames();

   ManualDesignPointManager& manualDesignPointManager = m_casaScenario.manualDesignPointManager();
   const MonteCarloDataManager& monteCarloDataManager = m_casaScenario.monteCarloDataManager();

   QVector<double> optimalMCPoint = monteCarloDataManager.getPoint(0);
   manualDesignPointManager.addDesignPoint(optimalMCPoint);

   Logger::log() << "Adding the design point ( ";
   for (const double d : optimalMCPoint)
   {
      Logger::log() << QString::number(d) << " ";
   }
   Logger::log() << ") as a manual design point." << Logger::endl();

   m_casaScenario.setNumberOfManualDesignPoints();
   m_casaScenario.changeUserDefinedPointStatus(true);

   AddCasesScript addCasesScript{m_casaScenario};
   if (!casaScriptWriter::writeCasaScript(addCasesScript) ||
       !m_scriptRunController.runScript(addCasesScript))
   {
      return;
   }
   m_casaScenario.manualDesignPointManager().completeAll();

   try
   {
      loadObservablesOptimalRun();
   }
   catch (const std::exception& e)
   {
      Logger::log() << e.what() << Logger::endl();
   }

   showOptimalCaseQuality();

   QStringList selectedDoEs = m_casaScenario.doeOptionSelectedNames();
   for (int i = 0; i < selectedDoEs.size(); i++)
   {
      if (selectedDoEs[i] == "UserDefined" || selectedQcOptions.contains(selectedDoEs[i]))
      {
         m_casaScenario.setIsQcDoeOptionSelected(i, true);
      }
   }

   m_casaScenario.setStageUpToDate(StageTypesUA::responseSurfaces,false);
   m_casaScenario.setStageUpToDate(StageTypesUA::mcmc,false);

   refreshGUI();

   scenarioBackup::backup(m_casaScenario);
}

void UAResultsController::slotPushButtonRecalculateResponseSurfacesClicked()
{
   scenarioBackup::backup(m_casaScenario);

   //Keep the optimal values, as these will get lost when the targetQcs are recalculated:
   QVector<double> optimalValues;
   QVector<QString> identifiers;
   QVector<TargetQC> targetQCs = m_casaScenario.targetQCs();
   for (const auto& qc : targetQCs)
   {
      optimalValues.append(qc.yOptimalSim());
      identifiers.append(qc.identifier());
   }

   QCScript qc{m_casaScenario};
   if (!casaScriptWriter::writeCasaScript(qc) ||
       !m_scriptRunController.runScript(qc))
   {
      return;
   }

   try
   {
      targetQCdataCreator::readTargetQCs(m_casaScenario);
   }
   catch (const std::exception& e)
   {
      Logger::log() << "Failed to read the targets for the quality check: " << e.what() << Logger::endl();
   }

   m_casaScenario.setOptimalValuesTargetQCs(optimalValues,identifiers);

   m_casaScenario.copyToIterationDir(m_casaScenario.stateFileNameQC());

   slotPushButtonMCMCrunCasaClicked();

   m_casaScenario.setStageUpToDate(StageTypesUA::responseSurfaces,true);
   m_casaScenario.setStageUpToDate(StageTypesUA::mcmc,true);

   refreshGUI();

   scenarioBackup::backup(m_casaScenario);
}

void UAResultsController::slotPushButtonExportMcmcOutputClicked()
{
   McmcTargetExportData exportData = McmcOutputDataCollector::collectMcmcOutputData(m_casaScenario);

   if (exportData.targetData.empty())
   {
      Logger::log() << "No data to export." << Logger::endl();
      return;
   }
   QString fileName = QFileDialog::getSaveFileName(m_uaResultsTab, "Save as", QDir::currentPath(), "comma separated file (*.csv)");
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

void UAResultsController::slotPredictionTargetClicked(int row)
{
   if (row < 0)
   {
      return;
   }

   const MonteCarloDataManager& monteCarloData = m_casaScenario.monteCarloDataManager();

   if (!monteCarloData.predictionTargetMatrix().empty())
   {
      if (m_uaResultsTab->checkBoxHistoryPlotsMode()->checkState() == Qt::CheckState::Checked)
      {
         const PredictionTargetManager& predictionTargetManager = m_casaScenario.predictionTargetManager();
         if (!predictionTargetManager.targetHasTimeSeries()[row])
         {
            Logger::log() << "This target does not have a time series, so the time series plot is empty";
            m_uaResultsTab->clearTimeSeriesPlots();
            return;
         }

         QVector<double> snapshotAges;
         QMap<QString,QVector<double>> bestMatchedValuesPerProperty; // Initialize one best matched values vector per property
         QMap<QString,QVector<QVector<double>>> currentPredTargetMatrixPerProperty; // Initialize one prediction matrix per property, indexed in a map
         m_casaScenario.obtainTimeSeriesMonteCarloData(row, snapshotAges, bestMatchedValuesPerProperty, currentPredTargetMatrixPerProperty);

         m_uaResultsTab->updateTimeSeriesPlot(snapshotAges, currentPredTargetMatrixPerProperty, bestMatchedValuesPerProperty, predictionTargetManager.predictionTargetOptions());
      }
      else
      {
         QVector<QVector<double>> data;
         m_casaScenario.obtainMonteCarloDataForTimeStep(row, 0.0, data);

         const PredictionTargetManager& predictionTargetManager = m_casaScenario.predictionTargetManager();
         const PredictionTarget* const targetTimeSeries = predictionTargetManager.predictionTargetInTimeSeries(row)[0];
         m_uaResultsTab->updateHistogram(targetTimeSeries, data, predictionTargetManager.predictionTargetOptions());
         m_uaResultsTab->updateSliderHistograms(predictionTargetManager.targetHasTimeSeries()[row] ? m_casaScenario.projectReader().agesFromMajorSnapshots().size() : 0);
      }
   }
}

void UAResultsController::slotSliderHistogramsChanged(int indexTime)
{
   const MonteCarloDataManager& monteCarloData = m_casaScenario.monteCarloDataManager();

   if (!monteCarloData.predictionTargetMatrix().empty())
   {
      const int targetIdx = m_targetTableController.currentTarget();
      QVector<QVector<double>> data;
      m_casaScenario.obtainMonteCarloDataForTimeStep(targetIdx, indexTime, data);

      const PredictionTargetManager& predictionTargetManager = m_casaScenario.predictionTargetManager();
      const PredictionTarget* const targetTimeSeries = predictionTargetManager.predictionTargetInTimeSeries(targetIdx)[indexTime];

      m_uaResultsTab->updateHistogram(targetTimeSeries, data, predictionTargetManager.predictionTargetOptions());
   }
}

void UAResultsController::slotCheckBoxHistoryPlotModeChanged(const int checkState)
{
   m_uaResultsTab->setEnableTimeSeries(checkState);
   slotPredictionTargetClicked(m_targetTableController.currentTarget());
}

} // namespace ua

} // namespace casaWizard
