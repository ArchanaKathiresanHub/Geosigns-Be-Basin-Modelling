//
// Copyright (C) 2012-2023 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// Automated regression test for the SACThermal tool

#include "ThermalGuiTester.h"
#include "GuiTestUtilities.h"

#include <QTimer>
#include <QLineEdit>

#include "view/ThermalInputTab.h"
#include "view/ThermalResultsTab.h"
#include "view/ThermalMapTab.h"
#include "view/calibrationTargetTable.h"
#include "view/menuBar.h"

#include "view/assets/objectiveFunctionTableSAC.h"

#include "model/scenarioReader.h"

#include <iostream>
#include <string>
#include <cmath>

static const QString saveFile = "TesterSave.dat";

namespace casaWizard
{

namespace sac
{

namespace thermal
{

ThermalGuiTester::ThermalGuiTester(ThermalController& controller) :
   GuiTester(),
   m_controller(controller),
   m_inputValidator(new ThermalInputTabValidator(controller, this)),
   m_resultsValidator(new ThermalResultsTabValidator(controller, this)),
   m_mapsValidator(new ThermalMapsTabValidator(controller, this))
{

}

void ThermalGuiTester::runValidations()
{
   std::cout << "Validating the input tab: " << std::endl;
   m_inputValidator->validateInputTab();

   std::cout << "Validating the results tab: " << std::endl;
   m_resultsValidator->validateResultsTab();

   std::cout << "Validating the maps tab: " << std::endl;
   m_mapsValidator->validateMapsTab();

   std::cout << "Validating saving & loading: " << std::endl;
   validateLoadingSaves();
}

void ThermalGuiTester::validateLoadingSaves()
{
   m_workingDir = m_controller.scenario().workingDirectory(); // need to know for loading, get cleared during validation of clearing
   if (m_failedSteps > 0)
   {
      std::cout << "* WARNING: Previous tests steps have failed and can influence the results of the following tests" << std::endl;
   }

   std::cout << " - Validating saving: " << std::endl;
   validateSaving();

   std::cout << " - Validating clearing: " << std::endl;
   validateClearing();

   std::cout << " - Validating loading saved scenario: " << std::endl;
   validateLoading();
}

void ThermalGuiTester::openSave(QString filepath)
{
   ScenarioReader reader{filepath};
   m_controller.scenario().readFromFile(reader);
}

void ThermalGuiTester::validateSaving()
{
   QTimer::singleShot(guiUtilities::timeDelay, [this](){processSaveDialog();} );
   std::cout.rdbuf(m_logStream.rdbuf()); //mute reading from file message
   m_controller.gui().menu()->actionSave()->trigger();
   std::cout.rdbuf(m_testBuffer);

   reportStep(QFile::exists( m_controller.scenario().workingDirectory() + "/" + saveFile), "Validate save file is created");
}

void ThermalGuiTester::validateClearing()
{
   std::cout.rdbuf(m_logStream.rdbuf()); //mute clearing message
   m_controller.gui().menu()->actionNew()->trigger();
   std::cout.rdbuf(m_testBuffer);

   //lineEdit project3d
   reportStep(m_controller.gui().inputTab()->lineEditProject3D()->text().isEmpty(), "Validate project3d line edit is empty");
   reportStep(m_controller.scenario().project3dPath().isEmpty(), "Validate scenario stored project3d path is empty");

   //calibrationtable
   reportStep(m_controller.gui().inputTab()->calibrationTargetTable()->rowCount() == 0, "Validate target table is empty");
   reportStep(m_controller.scenario().calibrationTargetManager().wells().empty(), "Validate scenario stored wells are empty");

   //properties table
   reportStep(m_controller.gui().inputTab()->objectiveFunctionTable()->rowCount() == 0, "Validate property table is empty");
   reportStep(m_controller.scenario().objectiveFunctionManager().values().empty(), "Validate scenario stored properties are empty");
   reportStep(m_controller.scenario().calibrationTargetManager().userNameToCauldronNameMapping().empty(), "Validate username to cauldron name mapping is cleared");

   //run options
   reportStep(m_controller.gui().inputTab()->lineEditMinTCHP()->text() == "0", "Validate minTCHP is restored to default");
   reportStep(m_controller.gui().inputTab()->lineEditMaxTCHP()->text() == "5", "Validate maxTCHP is restored to default");

   //results
   bool succes = true;
   for (auto trajectory : m_controller.scenario().wellTrajectoryManager().trajectories())
   {
      succes &= trajectory.empty();
   }
   reportStep(succes, "Validate well trajectories are cleared");
   reportStep(m_controller.scenario().TCHPmanager().optimizedTCHPs().empty(), "Validate optimized TCHP's are cleared");
}

void ThermalGuiTester::validateLoading()
{
   QTimer::singleShot(guiUtilities::timeDelay, [this](){processLoadDialog();} );
   std::cout.rdbuf(m_logStream.rdbuf()); //mute reading from file message
   m_controller.gui().menu()->actionOpen()->trigger();
   std::cout.rdbuf(m_testBuffer);

   if (!h_bool)
   {
      throw std::runtime_error(("Failed to find save file in " + m_controller.scenario().workingDirectory().toStdString() + "/" + saveFile.toStdString()));
   }

   //lineEdit project3d
   reportStep(!m_controller.gui().inputTab()->lineEditProject3D()->text().isEmpty(), "Validate project3d filepath is shown");
   reportStep(!m_controller.scenario().project3dPath().isEmpty(), "Validate scenario stored project3d path is loaded");

   //calibrationtable
   reportStep(m_controller.gui().inputTab()->calibrationTargetTable()->rowCount() == 4, "Validate target table loaded 4 entries");
   reportStep(!m_controller.scenario().calibrationTargetManager().wells().empty(), "Validate scenario stored wells are loaded");

   //properties table
   reportStep(m_controller.gui().inputTab()->objectiveFunctionTable()->rowCount() == 2, "Validate property table loaded 2 entries");
   reportStep(!m_controller.scenario().objectiveFunctionManager().values().empty(), "Validate scenario stored properties isn't empty");
   reportStep(!m_controller.scenario().calibrationTargetManager().userNameToCauldronNameMapping().empty(), "Validate username to cauldron name mapping is loaded");

   //run options
   reportStep(m_controller.gui().inputTab()->lineEditMinTCHP()->text() == "0.5", "Validate minTCHP loads stored value");
   reportStep(m_controller.gui().inputTab()->lineEditMaxTCHP()->text() == "4.5", "Validate maxTCHP loads stored value");

   bool succes = true;
   for (auto trajectory : m_controller.scenario().wellTrajectoryManager().trajectories())
   {
      succes &= !trajectory.empty();
   }
   reportStep(succes, "Validate well trajectories are loaded");
   reportStep(!m_controller.scenario().TCHPmanager().optimizedTCHPs().empty(), "Validate optimized TCHP's are loaded");
}

void ThermalGuiTester::processSaveDialog()
{
   //clear old save if present (should never happen but still)
   if(QFile::exists( m_controller.scenario().workingDirectory() + "/" + saveFile) )
   {
      QFile(m_controller.scenario().workingDirectory() + "/" + saveFile).remove();
   }
   guiUtilities::openNewFileInDialog(saveFile); //this will get dumped in the working directory (casaworkspace...) not in the project folder
}

void ThermalGuiTester::processLoadDialog()
{
   if(!QFile::exists( m_workingDir + "/" + saveFile) )
   {
      guiUtilities::openExistingFileInDialog("", false); //leave dialog
      h_bool = false; //notifiy that this process failed
   }
   guiUtilities::openExistingFileInDialog(m_workingDir + "/" + saveFile);
   h_bool = true;
}

} // namespace thermal

} // namespace sac

} // namespace casawizard
