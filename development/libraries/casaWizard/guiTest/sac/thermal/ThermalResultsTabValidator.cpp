//
// Copyright (C) 2012 - 2023 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "ThermalResultsTabValidator.h"
#include "GuiTestUtilities.h"
#include "ThermalGuiTester.h"

#include <QAbstractButton>
#include <QTableWidget>
#include <QListWidget>
#include <QComboBox>
#include <QCheckBox>

#include "view/ThermalResultsTab.h"
#include "view/assets/PlotOptions.h"
#include "view/assets/wellCorrelationPlotLayout.h"
#include "view/plots/legend.h"
#include "view/plots/wellBirdsView.h"

using namespace guiUtilities;

namespace casaWizard
{

namespace sac
{

namespace thermal
{

ThermalResultsTabValidator::ThermalResultsTabValidator(ThermalController& controller, ThermalGuiTester* tester):
   SacResultsTabValidator(),
   m_controller(controller),
   m_tester(tester)
{

}

void ThermalResultsTabValidator::validateResultsTab()
{
   std::cout.rdbuf(m_tester->m_logStream.rdbuf()); //ignore well is selected message
   m_controller.gui().tabWidget()->setCurrentIndex(1);
   std::cout.rdbuf(m_tester->m_testBuffer);

   std::cout << " - Validating depth plots: " << std::endl;
   validateDepthPlots();

   std::cout << " - Validating correlation plots: " << std::endl;
   validateCorrelationPlots();

   std::cout << " - Validating birds view: " << std::endl;
   validateBirdsView();

   std::cout << " - Validating top crust heat production table: " << std::endl;
   validateTable();
}

void ThermalResultsTabValidator::validateDepthPlots()
{
   auto checkActivePlots = [this](const bool& expectedStates) -> bool
   {
      for (bool state : m_controller.gui().resultsTab()->plotOptions()->activePlots())
      {
         if (state != expectedStates)
         {
            return false;
         }
      }
      return true;
   };

   auto* org1D = getQChild<QCheckBox>("Original 1d", m_controller.gui().resultsTab()->plotOptions());
   auto* opt1D = getQChild<QCheckBox>("Optimized 1d", m_controller.gui().resultsTab()->plotOptions());
   auto* org3D = getQChild<QCheckBox>("Original 3d", m_controller.gui().resultsTab()->plotOptions());
   auto* opt3D = getQChild<QCheckBox>("Optimized 3d", m_controller.gui().resultsTab()->plotOptions());

   getQChild<QAbstractButton>("Depth plot", m_controller.gui().resultsTab()->plotOptions())->click();

   //set all trajectories inactive (set defaults false)
   org1D->click();
   opt1D->click();
   m_tester->reportStep(checkActivePlots(false), "Validate setting trajectories inactive");

   //set all trajectories active
   org1D->click();
   opt1D->click();
   org3D->click();
   opt3D->click();
   m_tester->reportStep(checkActivePlots(true), "Validate setting all trajectories active");
}

void ThermalResultsTabValidator::validateCorrelationPlots()
{
   getQChild<QAbstractButton>("Correlation plot", m_controller.gui().resultsTab()->plotOptions())->click();

   //check legend
   if (Legend* legend = getQChild<Legend>("", m_controller.gui().resultsTab()->wellCorrelationPlotLayout()))
   {
      m_tester->reportStep(legend->getEntries()->size() == 4, "Validate legend contains all properties");
   }
   else
   {
      m_tester->reportStep(false, "Failed to acquire legend");
   }

   //Check available properties to view
   if (QComboBox* cbox = m_controller.gui().resultsTab()->wellCorrelationPlotLayout()->properties())
   {
      m_tester->reportStep(cbox->count() == m_controller.scenario().objectiveFunctionManager().values().size(),
                           "Validating if properties combobox contains all properties");
   }
}

void ThermalResultsTabValidator::validateTable()
{
   auto compareTables = [this](QVector<QVector<QString>>& expected, QTableWidget* actual) -> bool
   {
      bool succes = true;
      for (int i = 0; i < expected.size(); ++i)
      {
         for (int j = 0; j < expected.size(); ++j)
         {
            if ( actual->item(i, j)->text() != expected[i][j])
            {
               succes = false;
               m_tester->reportStep(false, "Table entry " + std::to_string(i) + "," + std::to_string(j) + " doesn't match expected values");
            }
         }
      }
      return succes;
   };

   // go to Table view
   getQChild<QAbstractButton>("Table", m_controller.gui().resultsTab()->plotOptions())->click();

   QTableWidget* guiTable = m_controller.gui().resultsTab()->optimizedResultsTable();

   m_controller.gui().resultsTab()->wellsList()->setCurrentRow(0);

   QVector<QVector<QString>> expectedTable = {{"AME_1", "2.50", "4.33", "2.06"}};
   m_tester->reportStep(compareTables(expectedTable, guiTable), "Validating table with single well done");

   m_controller.gui().resultsTab()->wellsList()->selectAll();
   expectedTable = {{"AME_1", "2.50", "4.33", "2.06"},
                    {"AMN_1", "2.50", "4.13", "1.63"},
                    {"N07_3", "2.50", "4.42", "1.92"}};
   m_tester->reportStep(compareTables(expectedTable, guiTable), "Validating table with all wells done");
}

void ThermalResultsTabValidator::validateBirdsView()
{
   //test birdsview by selecting trough welllist
   //this step only works when on correlation or on table view
   m_controller.gui().resultsTab()->wellsList()->selectAll();
   m_tester->reportStep(m_controller.gui().resultsTab()->wellBirdsView()->selectedWells().size() == 3,
                        "Validate birdsview selection by selecting all wells is done. Expected 3 got " +
                        std::to_string(m_controller.gui().resultsTab()->wellBirdsView()->selectedWells().size()));

   std::cout.rdbuf(m_tester->m_logStream.rdbuf()); //mute "selected x well" message
   m_controller.gui().resultsTab()->wellsList()->setCurrentRow(1);
   std::cout.rdbuf(m_tester->m_testBuffer);

   m_tester->reportStep( (m_controller.gui().resultsTab()->wellBirdsView()->selectedWells().size() == 1) &&
                         (m_controller.gui().resultsTab()->wellBirdsView()->selectedWells().last() == 1),
                         "Validate birdsview selection by selecting a single well is done. Expected 1 well with index 1 got " +
                         std::to_string(m_controller.gui().resultsTab()->wellBirdsView()->selectedWells().size()) +
                         " well(s) with index " + std::to_string(m_controller.gui().resultsTab()->wellBirdsView()->selectedWells().first()));
}

} // thermal

} // sac

} // casaWizard
