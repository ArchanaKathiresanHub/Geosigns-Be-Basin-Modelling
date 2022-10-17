//
// Copyright (C) 2012-2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "ThermalInputController.h"

#include "view/assets/ThermalTabIDs.h"

#include "model/script/ThermalScript.h"
#include "model/extractor/OptimizedTCHPExtractor.h"

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

const static QString defaultMinTCHP = "0.0";
const static QString defaultMaxTCHP = "5.0";

namespace casaWizard
{

namespace sac
{

namespace thermal
{

ThermalInputController::ThermalInputController(ThermalInputTab* inputTab,
                                               ThermalScenario& scenario,
                                               ScriptRunController& scriptRunController,
                                               QObject* parent) :
   SacInputController(inputTab, scenario, scriptRunController, parent),
   m_scenario(scenario),
   m_inputTab(inputTab)
{
   connect(inputTab->pushButtonImportTargets(), SIGNAL(clicked()), this, SLOT(slotImportTargetsClicked()));
   connect(inputTab->lineEditMinTCHP(), SIGNAL(textChanged(QString)), this, SLOT(slotMinTCHPChanged(QString)));
   connect(inputTab->lineEditMaxTCHP(), SIGNAL(textChanged(QString)), this, SLOT(slotMaxTCHPChanged(QString)));

   inputTab->lineEditMinTCHP()->setText(defaultMinTCHP);
   inputTab->lineEditMaxTCHP()->setText(defaultMaxTCHP);

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

   if (!m_scenario.calibrationTargetManager().wells().empty())
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
         m_scenario.clearWellsAndCalibrationTargets();
      }
   }

   ImportWellPopupXlsxController controller(this, m_scenario, {"Temperature", "VRe", "Unknown"});
   controller.importWellsToCalibrationTargetManager({fileName}, m_scenario.calibrationTargetManager());

   m_scenario.updateObjectiveFunctionFromTargets();
   m_scenario.wellTrajectoryManager().updateWellTrajectories(m_scenario.calibrationTargetManager());

   if (!m_scenario.project3dPath().isEmpty())
   {
     m_scenario.updateWellsForProject3D();
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
   return std::unique_ptr<SACScript>(new ThermalScript(m_scenario, baseDirectory, doOptimization));
}

void ThermalInputController::readOptimizedResults()
{
   OptimizedTCHPExtractor TCHPExtractor{m_scenario};
   dataExtractionController()->readOptimizedResults(TCHPExtractor);
}

void ThermalInputController::slotUpdateTabGUI(int tabID)
{
   if (tabID != static_cast<int>(TabID::Input))
   {
      return;
   }
   refreshGUI();
}

void ThermalInputController::slotMinTCHPChanged(QString text)
{
   m_scenario.TCHPmanager().setMinTCHP(std::round(text.toDouble() * 10000.0) / 10000.0);
}

void ThermalInputController::slotMaxTCHPChanged(QString text)
{
   m_scenario.TCHPmanager().setMaxTCHP(std::round(text.toDouble() * 10000.0) / 10000.0);
}

void ThermalInputController::slotPushButtonSelectProject3dClicked()
{
   if (selectWorkspace())
   {
      refreshGUI();
   }
}

ThermalScenario& ThermalInputController::scenario()
{
   return m_scenario;
}

ThermalScenario& ThermalInputController::scenario() const
{
   return m_scenario;
}

ThermalInputTab* ThermalInputController::inputTab()
{
   return m_inputTab;
}

const ThermalInputTab* ThermalInputController::inputTab() const
{
   return m_inputTab;
}

} //namespace thermal

} //namespace sac

} //namespace casaWizard
