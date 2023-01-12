//
// Copyright (C) 2012 - 2023 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "ThermalInputTabValidator.h"
#include "GuiTestUtilities.h"
#include "ThermalGuiTester.h"

#include <QWidget>
#include <QTimer>
#include <QAbstractButton>
#include <QLineEdit>
#include <QComboBox>
#include <QFileDialog>

#include "view/ThermalInputTab.h"

#include "view/importWellPopupXlsx.h"
#include "view/workspaceDialog.h"
#include "view/calibrationTargetTable.h"

#include "view/assets/objectiveFunctionTableSAC.h"

using namespace guiUtilities;

namespace casaWizard
{

namespace sac
{

namespace thermal
{

ThermalInputTabValidator::ThermalInputTabValidator(ThermalController& controller, ThermalGuiTester* tester):
   SacInputTabValidator(),
   m_controller(controller),
   m_tester(tester)
{

}

void ThermalInputTabValidator::validateInputTab()
{
   m_controller.gui().tabWidget()->setCurrentIndex(0);

   //Select & load in project3D file (button & path & validation)
   //Select a workspace path (dialog & path)
   std::cout << " - Validating Import Project3D File: " << std::endl;
   validateImportProject3DFile();

   //Import wells (button & path & validation)
   std::cout << " - Validating Importing wells: " << std::endl;
   validateImportWells();

   std::cout << " - Validating Calibration Targets: " << std::endl;
   validateCalibrationTargets();

   //Validation of selections & subsequent changes:
   std::cout << " - Validating run options: " << std::endl;
   validateRunOptions();

   //Validation of run Modes:
   std::cout << " - Validating 1D Optimization: " << std::endl;
   validateRun1DOptimization();

   std::cout << " - Validating 1D Original: " << std::endl;
   validateRun1DOriginal();

   std::cout << " - Validating 3D Original: " << std::endl;
   validateRun3DOriginal();
}

void ThermalInputTabValidator::validateImportProject3DFile()
{
   //validate cancelling project import
   QTimer::singleShot( timeDelay, [this](){processSelectProject(false);} );

   getQChild<QAbstractButton>("Select", m_controller.gui().inputTab())->click();
   m_tester->reportStep(m_controller.gui().inputTab()->lineEditProject3D()->text() == "", "Validate cancel loading the project3d file");

   //validate importing project
   QTimer::singleShot( timeDelay, [this](){processSelectProject(true);} );
   getQChild<QAbstractButton>("Select", m_controller.gui().inputTab())->click();
   if (m_controller.gui().inputTab()->lineEditProject3D()->text() == "")
   {
      throw std::runtime_error("Failed to load in the project3d file");
   }

   m_tester->reportStep(!m_controller.scenario().project3dFilename().isEmpty(), "Validate loading in the project3d file");
}

void ThermalInputTabValidator::validateImportWells()
{
   //validate cancelling importing wells, by selecting a file then escaping/cancelling the import wells dialog & validate no targets or properties have been set
   QTimer::singleShot( timeDelay, [this](){processSelectTargets(false);} );
   getQChild<QAbstractButton>("Import Targets", m_controller.gui().inputTab()->subLayoutActivationWidget())->click();

   m_tester->reportStep( (m_controller.scenario().calibrationTargetManager().wells().isEmpty()) &&
                         (m_controller.scenario().objectiveFunctionManager().values().isEmpty()),
                         "Validating cancelling importing wells" );

   //validate importing wells
   std::cout.rdbuf(m_tester->m_logStream.rdbuf()); //mute imported wells messages

   QTimer::singleShot( timeDelay,[this](){processSelectTargets(true);} );
   getQChild<QAbstractButton>("Import Targets", m_controller.gui().inputTab()->subLayoutActivationWidget())->click();

   std::cout.rdbuf(m_tester->m_testBuffer);

   if (m_controller.scenario().calibrationTargetManager().wells().isEmpty() ||
       m_controller.scenario().objectiveFunctionManager().values().isEmpty())
   {
      throw std::runtime_error("Failed to load in wells");
   }
   m_tester->reportStep(true, "Validate importing wells");

   //validate overriding wells
   std::cout.rdbuf(m_tester->m_logStream.rdbuf());

   QTimer::singleShot(timeDelay, [this](){processSelectTargets(true);} );
   getQChild<QAbstractButton>("Import Targets", m_controller.gui().inputTab()->subLayoutActivationWidget())->click();

   std::cout.rdbuf(m_tester->m_testBuffer);

   if (m_controller.scenario().calibrationTargetManager().wells().size() != 4 ||
       m_controller.scenario().objectiveFunctionManager().values().size() != 2)
   {
      throw std::runtime_error("Failed to override wells");
   }
   m_tester->reportStep(true, "Validate overriding existing wells with imported wells");
}

void ThermalInputTabValidator::validateCalibrationTargets()
{
   auto setAllPropertiesState = [this](bool state)
   {
      std::cout.rdbuf(m_tester->m_logStream.rdbuf()); //mute messages
      for (int i = 0; i < m_controller.gui().inputTab()->objectiveFunctionTable()->rowCount(); ++i )
      {
         dynamic_cast<QAbstractButton*> (m_controller.gui().inputTab()->objectiveFunctionTable()->cellWidget(i, 0)->children()[0])->setChecked(state);
      }
      std::cout.rdbuf(m_tester->m_testBuffer);
   };

   CalibrationTargetTable* table = m_controller.gui().inputTab()->calibrationTargetTable();

   //validate targets are imported
   if(table->rowCount() != 4)
   {
      throw std::runtime_error("Incorrect number of targets expected, expected 4");
   }

   std::vector<std::vector<QString>> expectedTableEntries = {{"AME_1", "192870", "609000"},
                                                             {"AMN_1", "191689", "615179"},
                                                             {"N07_3", "199340", "613950"},
                                                             {"OutOfAOI",   "0",      "0"}}; //should be disabled

   for(int i = 0; i < expectedTableEntries.size(); ++i)
   {
      for(int j = 0; j < expectedTableEntries[i].size(); ++j)
      {
         if(expectedTableEntries[i][j] != table->item(i, j+1)->text() )
         {
            throw std::runtime_error("Table entry " + std::to_string(i) + "," + std::to_string(j) + " doesn't match expected values");
         }

         if(i == 3 && table->item(i, j+1)->flags().testFlag(Qt::ItemIsEnabled))
         {
            throw std::runtime_error("Table entry " + std::to_string(i) + "," + std::to_string(j) + " should be disabled");
         }
      }
   }

   m_tester->reportStep(true, "Validate entries match expected data");

   //Validate disabling targets trough objective funtions
   setAllPropertiesState(false);

   bool success = true;
   for(int i = 0; i < table->rowCount(); ++i)
   {
      for(int j = 1; j < table->columnCount(); ++j)
      {
         if(table->item(i, j)->flags().testFlag(Qt::ItemIsEnabled))
         {
            success = false;
         }
      }
   }

   m_tester->reportStep(success, "Validating disabling targets through properties (objective functions) done");

   //Reenable the properties, so that there are wells available for the rest of the workflow
   setAllPropertiesState(true);
}

void ThermalInputTabValidator::validateRunOptions()
{
   m_controller.gui().inputTab()->lineEditMinTCHP()->setText("0.5");
   m_controller.gui().inputTab()->lineEditMaxTCHP()->setText("4.5");

   m_tester->reportStep((m_controller.scenario().TCHPmanager().minTCHP() == 0.5) &&
                        (m_controller.scenario().TCHPmanager().maxTCHP() == 4.5),
                        "Validate editability of min/max TCHP.");

   m_controller.gui().inputTab()->comboBoxApplication()->setCurrentText("Hydrostatic");
   m_tester->reportStep(m_controller.scenario().applicationName() == "fastcauldron \"-temperature\"",
                        "Validate run applications");

   m_controller.gui().inputTab()->comboBoxCluster()->setCurrentText("LOCAL");
   m_tester->reportStep(m_controller.scenario().clusterName() == "LOCAL" , "Validating run locations");

   QApplication::processEvents(); //allow qt to show changes
}

void ThermalInputTabValidator::validateRun1DOptimization()
{
   std::cout.rdbuf(m_tester->m_logStream.rdbuf());

   getQChild<QAbstractButton>("Run 1D optimization", m_controller.gui().inputTab()->subLayoutActivationWidget())->click(); //hangs till dialog is closed

   std::cout.rdbuf(m_tester->m_testBuffer);
   if (m_controller.scenario().wellTrajectoryManager().trajectories()[TrajectoryType::Optimized1D].first().depth().empty() ||
       m_controller.scenario().wellTrajectoryManager().trajectories()[TrajectoryType::Optimized1D].first().value().empty())
   {
      throw std::runtime_error("Failed to generate Optimized 1D Results");
   }

   m_tester->reportStep(true, "Validating generation of optimized 1D Results");
}

void ThermalInputTabValidator::validateRun1DOriginal()
{
   std::cout.rdbuf(m_tester->m_logStream.rdbuf());

   getQChild<QAbstractButton>("Run original 1D", m_controller.gui().inputTab()->subLayoutActivationWidget())->click();

   std::cout.rdbuf(m_tester->m_testBuffer);

   if (m_controller.scenario().wellTrajectoryManager().trajectories()[TrajectoryType::Original1D].first().depth().empty() ||
       m_controller.scenario().wellTrajectoryManager().trajectories()[TrajectoryType::Original1D].first().value().empty())
   {
      throw std::runtime_error("Failed to generate original 1D Results");
   }
   m_tester->reportStep(true, "Validating generation of original 1D Results");
}

void ThermalInputTabValidator::validateRun3DOriginal()
{
   std::cout.rdbuf(m_tester->m_logStream.rdbuf());

   QTimer::singleShot(timeDelay,   [this](){processDestinationFolderPopup() ;} );
   QTimer::singleShot(2*timeDelay, [this](){processCpuPopup()               ;} );
   QTimer::singleShot(3*timeDelay, [this](){processSubSamplingPopup()       ;} );

   getQChild<QAbstractButton>("Run original 3D", m_controller.gui().inputTab()->subLayoutActivationWidget())->click();

   std::cout.rdbuf(m_tester->m_testBuffer);

   if (m_controller.scenario().wellTrajectoryManager().trajectories()[TrajectoryType::Original3D].first().depth().empty() ||
       m_controller.scenario().wellTrajectoryManager().trajectories()[TrajectoryType::Original3D].first().value().empty())
   {
      throw std::runtime_error("Failed to generate Original 3D Results");
   }
   m_tester->reportStep(true, "Validating generation of original 3d results");
}

void ThermalInputTabValidator::processSelectProject(bool proceed)
{
   QTimer::singleShot(timeDelay,   [this](){openExistingFileInDialog(QDir::current().absolutePath() + "/Project.project3d"); } );
   QTimer::singleShot(2*timeDelay, [this](){pressButtonInDialog<QFileDialog>(true);                                          } );
   QTimer::singleShot(3*timeDelay, [this, proceed](){pressButtonInDialog<WorkspaceDialog>(proceed);                          } );
}

void ThermalInputTabValidator::processSelectTargets(bool proceed)
{
   QTimer::singleShot(timeDelay,   [this](){openExistingFileInDialog("wells.xlsx");                                                                         } );
   QTimer::singleShot(2*timeDelay, [this](){if(!m_controller.scenario().calibrationTargetManager().wells().isEmpty()){pressButtonInDialog<QDialog>(false);} } );
   QTimer::singleShot(3*timeDelay, [this, proceed](){pressButtonInDialog<ImportWellPopupXlsx>(proceed);                                                     } );
}

void ThermalInputTabValidator::processDestinationFolderPopup()
{
   pressButtonInDialog<QDialog>(true);
}

void ThermalInputTabValidator::processCpuPopup()
{
   setIntInInputDialog(12);
}

void ThermalInputTabValidator::processSubSamplingPopup()
{
   setIntInInputDialog(10);
}

} // thermal

} // sac

} // casaWizard
