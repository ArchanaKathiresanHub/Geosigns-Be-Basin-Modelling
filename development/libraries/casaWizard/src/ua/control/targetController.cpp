#include "targetController.h"

#include "control/calibrationTargetController.h"
#include "control/importWellPopupXlsxController.h"
#include "control/objectiveFunctionController.h"
#include "control/PredictionTargetController.h"
#include "model/input/extractWellDataXlsx.h"
#include "model/input/calibrationTargetCreator.h"
#include "model/logger.h"
#include "model/uaScenario.h"
#include "view/calibrationTargetTable.h"
#include "view/targetTab.h"
#include "view/uaTabIDs.h"
#include "control/importWellPopupController.h"

#include <QFileDialog>
#include <QLineEdit>
#include <QPushButton>

namespace casaWizard
{

namespace ua
{

TargetController::TargetController(TargetTab* targetTab,
                                   UAScenario& scenario,
                                   QObject* parent) :
   QObject(parent),
   scenario_{scenario},
   targetTab_{targetTab},
   predictionTargetController_{new PredictionTargetController(targetTab_->surfaceTargetTable(),
                                                              scenario_.predictionTargetManager(),
                                                              scenario_.calibrationTargetManager(),
                                                              this)},
   calibrationTargetController_{new CalibrationTargetController(targetTab_->calibrationTargetTable(), scenario_, this)},
   objectiveFunctionController_{new ObjectiveFunctionController(targetTab_->objectiveFunctionTable(), scenario_, this)}
{
   connect(parent, SIGNAL(signalUpdateTabGUI(int)), this, SLOT(slotUpdateTabGUI(int)));

   connect(targetTab_->pushSelectCalibration(), SIGNAL(clicked()),                   this, SLOT(slotPushSelectCalibrationClicked()));
   connect(targetTab_->pushSelectAllVRe(), SIGNAL(clicked()),                   this, SLOT(slotPushSelectAllVReClicked()));
   connect(targetTab_->pushSelectAllTemperatures(), SIGNAL(clicked()),                   this, SLOT(slotPushSelectAllTemperaturesClicked()));

   connect(targetTab_->pushSelectAllTargetVRe(), SIGNAL(clicked()),                   this, SLOT(slotPushSelectAllVRePredictionTargetsClicked()));
   connect(targetTab_->pushSelectAllTargetTemperatures(), SIGNAL(clicked()),                   this, SLOT(slotPushSelectAllTemperaturePredictionTargetsClicked()));
   connect(targetTab_->pushSelectAllTargetTimeSeries(), SIGNAL(clicked()),                   this, SLOT(slotPushSelectAllTimeSeriesPredictionTargetsClicked()));

   connect(predictionTargetController_, SIGNAL(dataChanged()), this, SLOT(slotDataChanged()));
   connect(calibrationTargetController_, SIGNAL(wellSelectionChanged()), this, SLOT(slotDataChanged()));
   connect(objectiveFunctionController_, SIGNAL(dataChanged()), this, SLOT(slotDataChanged()));

   connect(targetTab_->lineEditCalibration(),   SIGNAL(textChanged(const QString&)), this, SLOT(slotLineEditCalibrationTextChanged(const QString&)));
}

void TargetController::refreshGUI()
{
   QSignalBlocker blockCalibration{targetTab_->lineEditCalibration()};
   targetTab_->lineEditCalibration()->setText("");

   bool buttonEnabled = scenario_.calibrationTargetManager().wells().size() > 0;
   predictionTargetController_->enableTargetsAtWellLocationsButton(buttonEnabled);

   emit signalRefreshChildWidgets();
}

void TargetController::slotUpdateTabGUI(int tabID)
{
   if (tabID != static_cast<int>(TabID::Targets))
   {
      return;
   }

   if (scenario_.isStageComplete(StageTypesUA::doe))
   {
      targetTab_->setEnabled(scenario_.iterationDirExists());
   }
   else
   {
      targetTab_->setEnabled(false);
      Logger::log() << "DoE data is not available! Complete DoE stage in DoE tab first." << Logger::endl();
   }

   refreshGUI();
}

void TargetController::slotPushSelectAllVReClicked()
{
   scenario_.calibrationTargetManager().setPropertyActiveForAllWells("VRe");
   calibrationTargetController_->refreshAndEmitDataChanged();
}

void TargetController::slotPushSelectAllTemperaturesClicked()
{
   scenario_.calibrationTargetManager().setPropertyActiveForAllWells("Temperature");
   calibrationTargetController_->refreshAndEmitDataChanged();
}

void TargetController::slotPushSelectAllVRePredictionTargetsClicked()
{
   scenario_.predictionTargetManager().setPropertyActiveForAllTargets("VRe");
   predictionTargetController_->refreshAndEmitDataChanged();
}

void TargetController::slotPushSelectAllTemperaturePredictionTargetsClicked()
{
   scenario_.predictionTargetManager().setPropertyActiveForAllTargets("Temperature");
   predictionTargetController_->refreshAndEmitDataChanged();
}

void TargetController::slotPushSelectAllTimeSeriesPredictionTargetsClicked()
{
   scenario_.predictionTargetManager().setTimeSeriesActiveForAllTargets();
   predictionTargetController_->refreshAndEmitDataChanged();
}

void TargetController::slotDataChanged()
{
   scenario_.setStageComplete(StageTypesUA::responseSurfaces, false);
   scenario_.setStageComplete(StageTypesUA::mcmc, false);
}

void TargetController::slotPushSelectCalibrationClicked()
{
   const bool oneFolderHigherThanWorkingDir = true;
   QString fileName = QFileDialog::getOpenFileName(targetTab_,
                                                   "Select calibration targets",
                                                   scenario_.defaultFileDialogLocation(oneFolderHigherThanWorkingDir),
                                                   "Spreadsheet (*.xlsx)");
   targetTab_->lineEditCalibration()->setText(fileName);
   emit signalRefreshChildWidgets();
}

void TargetController::slotLineEditCalibrationTextChanged(const QString& calibrationTargetsFilename)
{
   if (!scenario_.calibrationTargetManager().wells().empty())
   {
      QMessageBox overwriteData(QMessageBox::Icon::Information,
                                "UA already has wells",
                                "Would you like to overwrite or append the new wells?");
      QPushButton* appendButton = overwriteData.addButton("Append", QMessageBox::RejectRole);
      QPushButton* overwriteButton =overwriteData.addButton("Overwrite", QMessageBox::AcceptRole);
      connect(appendButton, SIGNAL(clicked()), &overwriteData, SLOT(reject()));
      connect(overwriteButton, SIGNAL(clicked()), &overwriteData, SLOT(accept()));

      if (overwriteData.exec() == QDialog::Accepted)
      {
         scenario_.clearWellsAndCalibrationTargets();
      }
   }

   ImportWellPopupXlsxController controller(this, scenario_, {"Temperature", "VRe", "Unknown"});
   controller.importWellsToCalibrationTargetManager({calibrationTargetsFilename}, scenario_.calibrationTargetManager());

   bool buttonEnabled = scenario_.calibrationTargetManager().wells().size() > 0;
   predictionTargetController_->enableTargetsAtWellLocationsButton(buttonEnabled);

   scenario_.updateObjectiveFunctionFromTargets();

   emit signalRefreshChildWidgets();
}

} // namespace ua

} // namespace casaWizard
