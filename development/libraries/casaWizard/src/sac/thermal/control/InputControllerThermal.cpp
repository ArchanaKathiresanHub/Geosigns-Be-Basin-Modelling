#include "InputControllerThermal.h"

#include "view/InputTab.h"
#include "model/SacScenario.h"
#include "model/script/sacScript.h"
#include "control/importWellPopupXlsxController.h"

#include <QPushButton>
#include <QMessageBox>
#include <QFileDialog>

namespace casaWizard
{

namespace sac
{

namespace thermal
{

InputControllerThermal::InputControllerThermal(InputTab* inputTab,
                                 SacScenario& casaScenario,
                                 ScriptRunController& scriptRunController,
                                 QObject* parent) :
   SacInputController(inputTab, casaScenario, scriptRunController, parent),
   m_inputTab{inputTab},
   m_scenario(casaScenario)
{
   connect(m_inputTab->pushButtonImportTargets(), SIGNAL(clicked()), this, SLOT(slotImportTargetsClicked()));
}

void InputControllerThermal::readOptimizedResults()
{
   //Placeholder function
}

void InputControllerThermal::slotImportTargetsClicked()
{
   QString fileName = QFileDialog::getOpenFileName(m_inputTab,
                                                   "Select calibration targets",
                                                   "",
                                                   "Spreadsheet (*.xlsx)");

   if (!scenario().calibrationTargetManager().wells().empty())
   {
      QMessageBox overwriteData(QMessageBox::Icon::Information,
                                "SAC already has wells",
                                "Would you like to overwrite or append the new wells?");
      QPushButton* appendButton = overwriteData.addButton("Append", QMessageBox::RejectRole);
      QPushButton* overwriteButton =overwriteData.addButton("Overwrite", QMessageBox::AcceptRole);
      connect(appendButton, SIGNAL(clicked()), &overwriteData, SLOT(reject()));
      connect(overwriteButton, SIGNAL(clicked()), &overwriteData, SLOT(accept()));

      if (overwriteData.exec() == QDialog::Accepted)
      {
         scenario().clearWellsAndCalibrationTargets();
      }
   }

   ImportWellPopupXlsxController controller(this, scenario(), {"Temperature", "VRe", "Unknown"});
   controller.importWellsToCalibrationTargetManager({fileName}, scenario().calibrationTargetManager());

   scenario().updateObjectiveFunctionFromTargets();

   emit signalRefreshChildWidgets();
}

void InputControllerThermal::slotUpdateTabGUI(int tabID)
{

}

std::unique_ptr<SACScript> InputControllerThermal::optimizationScript(const QString& baseDirectory, bool doOptimization)
{
   return nullptr;
}

SacScenario& InputControllerThermal::scenario()
{
   return m_scenario;
}

SacScenario& InputControllerThermal::scenario() const
{
   return m_scenario;
}

} //namespace thermal

} //namespace sac

} //namespace casaWizard
