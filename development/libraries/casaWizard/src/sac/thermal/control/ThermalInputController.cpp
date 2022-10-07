//
// Copyright (C) 2012-2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "ThermalInputController.h"

#include "view/ThermalInputTab.h"
#include "view/assets/ThermalTabIDs.h"
#include "view/SacInputTab.h"

#include "model/SacScenario.h"
#include "model/script/ThermalScript.h"

#include "model/logger.h"
#include "model/scenarioBackup.h"

#include "control/importWellPopupXlsxController.h"
#include "control/casaScriptWriter.h"
#include "control/ScriptRunController.h"
#include "control/assets/DataExtractionController.h"

#include <QPushButton>
#include <QMessageBox>
#include <QFileDialog>
#include <QLineEdit>

#include <cmath>

const static QString defaultMinHCP = "0";
const static QString defaultMaxHCP = "5";

namespace casaWizard
{

namespace sac
{

namespace thermal
{

ThermalInputController::ThermalInputController(ThermalInputTab* inputTab,
                                               SacScenario& casaScenario,
                                               ScriptRunController& scriptRunController,
                                               QObject* parent) :
   SacInputController(inputTab, casaScenario, scriptRunController, parent),
   m_scenario(casaScenario)
{
   connect(inputTab->pushButtonImportTargets(), SIGNAL(clicked()), this, SLOT(slotImportTargetsClicked()));
   connect(inputTab->lineEditMinHCP(), SIGNAL(textChanged(QString)), this, SLOT(slotMinHCPChanged(QString)));
   connect(inputTab->lineEditMaxHCP(), SIGNAL(textChanged(QString)), this, SLOT(slotMaxHCPChanged(QString)));

   inputTab->lineEditMinHCP()->setText(defaultMinHCP);
   inputTab->lineEditMaxHCP()->setText(defaultMaxHCP);

   emit signalRefreshChildWidgets();
}

void ThermalInputController::slotImportTargetsClicked()
{
   QString fileName = QFileDialog::getOpenFileName(inputTab(),
                                                   "Select calibration targets",
                                                   "",
                                                   "Spreadsheet (*.xlsx)");
   if(fileName == "")
   {
      return;
   }

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
   scenario().wellTrajectoryManager().updateWellTrajectories(scenario().calibrationTargetManager());

   if (!scenario().project3dPath().isEmpty())
   {
     scenario().updateWellsForProject3D();
   }
   emit signalRefreshChildWidgets();
}

void ThermalInputController::refreshGUI()
{
   SacInputController::refreshGUI();
   emit signalRefreshChildWidgets();
}

std::unique_ptr<SACScript> ThermalInputController::optimizationScript(const QString& baseDirectory, bool doOptimization)
{
   return std::unique_ptr<SACScript>(new ThermalScript(scenario(), baseDirectory, doOptimization));
}

void ThermalInputController::readOptimizedResults()
{
   //placeholder
}

void ThermalInputController::slotUpdateTabGUI(int tabID)
{
   if (tabID != static_cast<int>(TabID::Input))
   {
      return;
   }
   refreshGUI();
}

void ThermalInputController::slotMinHCPChanged(QString text)
{
   m_minHCP = text.toDouble();
   m_minHCP = std::round(m_minHCP * 10000.0) / 10000.0;
}

void ThermalInputController::slotMaxHCPChanged(QString text)
{
   m_maxHCP = text.toDouble();
   m_minHCP = std::round(m_maxHCP * 10000.0) / 10000.0;
}

void ThermalInputController::slotPushButtonSelectProject3dClicked()
{
   if (selectWorkspace())
   {
      refreshGUI();
   }
}

SacScenario& ThermalInputController::scenario()
{
   return m_scenario;
}

SacScenario& ThermalInputController::scenario() const
{
   return m_scenario;
}

} //namespace thermal

} //namespace sac

} //namespace casaWizard
