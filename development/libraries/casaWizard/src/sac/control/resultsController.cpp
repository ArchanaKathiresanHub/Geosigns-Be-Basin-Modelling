#include "resultsController.h"

#include "control/casaScriptWriter.h"
#include "control/functions/copyCaseFolder.h"
#include "control/scriptRunController.h"
#include "model/case3DTrajectoryConvertor.h"
#include "model/input/case3DTrajectoryReader.h"
#include "model/logger.h"
#include "model/sacScenario.h"
#include "model/scenarioBackup.h"
#include "model/script/cauldronScript.h"
#include "model/script/Generate3DScenarioScript.h"
#include "model/script/track1dAllWellScript.h"
#include "view/plot/wellBirdsView.h"
#include "view/plot/wellScatterPlot.h"
#include "view/plotOptions.h"
#include "view/resultsTab.h"
#include "view/sacTabIDs.h"

#include <QFileDialog>
#include <QInputDialog>
#include <QListWidget>
#include <QPushButton>

namespace casaWizard
{

namespace sac
{

ResultsController::ResultsController(ResultsTab* resultsTab,
                                     SACScenario& scenario,
                                     ScriptRunController& scriptRunController,
                                     QObject* parent) :
  QObject(parent),
  resultsTab_{resultsTab},
  scenario_{scenario},
  scriptRunController_{scriptRunController},
  activeWell_{0},
  activeProperty_{""}
{
  connect(resultsTab_->wellsList(), SIGNAL(currentTextChanged(const QString&)), this, SLOT(updateWell(const QString&)));
  connect(resultsTab_->wellsList(), SIGNAL(itemSelectionChanged()), this, SLOT(refreshPlot()));

  connect(resultsTab_->buttonExportOptimized(), SIGNAL(clicked()), this, SLOT(exportOptimized()));
  connect(resultsTab_->buttonRunOptimized(),  SIGNAL(clicked()), this, SLOT(runOptimized()));
  connect(resultsTab_->buttonBaseCase(),      SIGNAL(clicked()), this, SLOT(runBaseCase()));

  connect(resultsTab_->plotOptions(), SIGNAL(activeChanged()), this, SLOT(refreshPlot()));
  connect(resultsTab_->plotOptions(), SIGNAL(plotTypeChange(int)), this, SLOT(togglePlotType(int)));
  connect(resultsTab_->plotOptions(), SIGNAL(propertyChanged(QString)), this, SLOT(updateProperty(QString)));

  connect(resultsTab_->wellBirdsView(), SIGNAL(pointSelectEvent(int,int)), this, SLOT(updateWellFromBirdView(int, int)));
  connect(resultsTab_->wellScatterPlot(), SIGNAL(selectedWell(int)), this, SLOT(selectedWellFromScatter(int)));
}

void ResultsController::refreshGUI()
{
  const CalibrationTargetManager& ctManager = scenario_.calibrationTargetManager();
  resultsTab_->updateWellList(ctManager.activeWells());
  resultsTab_->updateBirdsView(ctManager.activeWells());
  resultsTab_->plotOptions()->setActivePlots(scenario_.activePlots());
}

void ResultsController::slotUpdateTabGUI(int tabID)
{
  if (tabID != static_cast<int>(TabID::Results))
  {
    return;
  }

  refreshGUI();
  setDefaultWellSelection();
}

void ResultsController::setDefaultWellSelection()
{
  if (noValidWellSelected())
  {
    resultsTab_->wellsList()->setCurrentRow(0);
  }
}

bool ResultsController::noValidWellSelected()
{
  const CalibrationTargetManager& ctManager = scenario_.calibrationTargetManager();
  const int nrOfActiveWells = ctManager.activeWells().size();
  return ( nrOfActiveWells > 0 && ( resultsTab_->wellsList()->currentRow() < 0 ||
                                    resultsTab_->wellsList()->currentRow() >= nrOfActiveWells) );
}

void ResultsController::updateWell(const QString& name)
{
  activeWell_ = -1;
  const CalibrationTargetManager& ctManager = scenario_.calibrationTargetManager();
  for (const Well* well : ctManager.activeWells())
  {
    if (well->name() == name)
    {
      activeWell_ = well->id();
      break;
    }
  }

  Logger::log() << "Selected well " << activeWell_ << Logger::endl();
  updateOptimizedTable();
  refreshPlot();
}

void ResultsController::refreshPlot()
{
  QVector<bool> activePlots = resultsTab_->plotOptions()->activePlots();
  scenario_.setActivePlots(activePlots);
  updateWellPlot();
  updateScatterPlot();
  updateBirdView();
}

void ResultsController::exportOptimized()
{
  QDir sourceDir(scenario_.calibrationDirectory() + "/ThreeDFromOneD");
  if (!sourceDir.exists())
  {
    Logger::log() << "Optimized case is not available" << Logger::endl();
    return;
  }
  QDir targetDir(QFileDialog::getExistingDirectory(nullptr, "Save optimized case to directory", scenario_.workingDirectory(),
                                                   QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks));
  if (!targetDir.exists())
  {
    Logger::log() << "Target directory is not set" << Logger::endl();
    return;
  }

  const bool filesCopied = functions::copyCaseFolder(sourceDir, targetDir);
  QString projectTextFile("/" + scenario_.project3dFilename().replace(QString("project3d"), QString("txt")));
  QFile::copy(scenario_.workingDirectory() + projectTextFile, targetDir.absolutePath() + projectTextFile);

  scenarioBackup::backup(scenario_);

  Logger::log() << (filesCopied ? "Finished saving optimized case" :
                                  "Failed saving optimized case, no files were copied") << Logger::endl();
}

void ResultsController::runOptimized()
{
  scenarioBackup::backup(scenario_);

  const QString baseDirectory{scenario_.calibrationDirectory() + "/ThreeDFromOneD"};

  if (run3dCase(baseDirectory))
  {
    const bool isOptimized{true};
    import3dWellData(baseDirectory, isOptimized);
    scenarioBackup::backup(scenario_);
  }
}

void ResultsController::runBaseCase()
{
  const QString runDirectory{scenario_.calibrationDirectory() + "/ThreeDBase"};

  const QDir sourceDir(scenario_.workingDirectory());
  const QDir targetDir(runDirectory);
  if (!sourceDir.exists())
  {
    Logger::log() << "Source directory " + sourceDir.absolutePath() + " not found" << Logger::endl();
    return;
  }

  const bool filesCopied = functions::copyCaseFolder(sourceDir, targetDir);
  if (!filesCopied)
  {
    Logger::log() << "Failed to create the 3D base case"
                  << "\nThe base case is not run" << Logger::endl();
    return;
  }

  if (run3dCase(runDirectory))
  {
    const bool isOptimized{false};
    import3dWellData(runDirectory, isOptimized);
  }
}

void ResultsController::togglePlotType(const int currentIndex)
{
  resultsTab_->setPlotType(currentIndex);
  refreshPlot();
}

void ResultsController::updateProperty(const QString property)
{
  activeProperty_ = property;
  updateScatterPlot();
}

void ResultsController::updateWellFromBirdView(const int lineIndex, const int pointIndex)
{
  // Disable when multiple wells are selected
  if (selectedWells().size() > 1)
  {
    return;
  }
  QSignalBlocker blocker(resultsTab_->wellsList());
  if (lineIndex != 1)
  {
    return;
  }

  if (pointIndex<activeWell_)
  {
    activeWell_ = pointIndex;
  }
  else
  {
    activeWell_ = pointIndex + 1;
  }
  resultsTab_->wellsList()->setCurrentRow(activeWell_);
  refreshPlot();
}

void ResultsController::selectedWellFromScatter(const int wellIndex)
{
  const CalibrationTargetManager& mgr = scenario_.calibrationTargetManager();
  const QVector<const Well*> wells = mgr.activeWells();
  if (wellIndex>=wells.size())
  {
    return;
  }

  Logger::log() << "Selected point is in well " << wells[wellIndex]->name() << Logger::endl();

}

void ResultsController::updateOptimizedTable()
{
  const LithofractionManager& manager{scenario_.lithofractionManager()};
  resultsTab_->updateOptimizedLithoTable(manager.optimizedInWell(activeWell_), manager.lithofractions(), scenario_.projectReader());
}

void ResultsController::updateWellPlot()
{
  QStringList properties;
  const CalibrationTargetManager& calibrationManager = scenario_.calibrationTargetManager();
  QVector<QVector<CalibrationTarget>> targetsInWell = calibrationManager.extractWellTargets(properties, activeWell_);

  const WellTrajectoryManager& wellTrajectoryManager = scenario_.wellTrajectoryManager();
  const QVector<QVector<WellTrajectory>> allTrajectories = wellTrajectoryManager.trajectoriesInWell({activeWell_}, properties);
  QVector<bool> activePlots{scenario_.activePlots()};



  resultsTab_->updateWellPlot(targetsInWell,
                              properties,
                              allTrajectories,
                              activePlots);
}

void ResultsController::updateScatterPlot()
{
  QSignalBlocker blocker(resultsTab_->plotOptions());
  QStringList properties;
  QVector<int> wellIndices = selectedWells();
  const CalibrationTargetManager& calibrationManager = scenario_.calibrationTargetManager();
  QVector<QVector<CalibrationTarget>> targetsInWell = calibrationManager.extractWellTargets(properties, wellIndices);

  const WellTrajectoryManager& wellTrajectoryManager = scenario_.wellTrajectoryManager();
  QVector<QVector<WellTrajectory>> allTrajectories = wellTrajectoryManager.trajectoriesInWell(wellIndices, properties);
  QVector<bool> activePlots{scenario_.activePlots()};

  resultsTab_->updateScatterPlot(targetsInWell,
                                 properties,
                                 allTrajectories,
                                 activePlots,
                                 activeProperty_);
}

void ResultsController::updateBirdView()
{
  resultsTab_->updateActiveWells(selectedWells());
}

bool ResultsController::run3dCase(const QString directory)
{
  bool ok = true;
  const int cores = QInputDialog::getInt(0, "Number of cores", "Cores", scenario_.numberCPUs(), 1, 48, 1, &ok);
  if (!ok)
  {
    return false;
  }

  scenario_.setNumberCPUs(cores);
  scenarioBackup::backup(scenario_);
  CauldronScript cauldron{scenario_, directory};
  if (!casaScriptWriter::writeCasaScript(cauldron) ||
      !scriptRunController_.runScript(cauldron))
  {
    return false;
  }

  scenarioBackup::backup(scenario_);
  return true;
}

void ResultsController::import3dWellData(const QString baseDirectory, const bool isOptimized)
{
  Logger::log() << "Extracting data from the 3D case using track1d" << Logger::endl();

  QVector<double> xCoordinates;
  QVector<double> yCoordinates;
  const CalibrationTargetManager& calibrationTargetManager = scenario_.calibrationTargetManager();
  const QVector<const Well*> wells = calibrationTargetManager.activeWells();

  for (const Well* well : wells)
  {
    xCoordinates.push_back(well->x());
    yCoordinates.push_back(well->y());
  }
  QStringList properties = scenario_.calibrationTargetManager().activeProperties();

  Track1DAllWellScript import{baseDirectory,
                              xCoordinates,
                              yCoordinates,
                              properties,
                              scenario_.project3dFilename()};
  if (!scriptRunController_.runScript(import))
  {
    Logger::log() << "Failed to run well import" << Logger::endl();
    return;
  }

  Case3DTrajectoryReader reader{baseDirectory + "/welldata.csv"};
  try
  {
    reader.read();
  }
  catch(const std::exception& e)
  {
    Logger::log() << "Failed to read file" << e.what() << Logger::endl();
    return;
  }

  case3DTrajectoryConvertor::convertToScenario(reader, scenario_, isOptimized);

  Logger::log() << "Finished extracting data from the 3D case" << Logger::endl();
}

QVector<int> ResultsController::selectedWells()
{
  QModelIndexList indices = resultsTab_->wellsList()->selectionModel()->selectedIndexes();

  QVector<int> wellIndices;
  for(const QModelIndex& index : indices)
  {
    wellIndices.push_back(index.row());
  }
  std::sort(wellIndices.begin(), wellIndices.end());
  return wellIndices;
}

} // namespace sac

} // namespace casaWizard
