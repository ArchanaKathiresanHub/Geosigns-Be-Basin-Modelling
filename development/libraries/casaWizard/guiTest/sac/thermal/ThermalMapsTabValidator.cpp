//
// Copyright (C) 2012 - 2023 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "ThermalMapsTabValidator.h"
#include "GuiTestUtilities.h"
#include "ThermalGuiTester.h"

#include "view/ThermalMapTab.h"
#include "view/assets/TCHPVisualisation.h"
#include "view/assets/activeWellsTable.h"

#include "view/sharedComponents/customtitle.h"
#include "view/plots/ThermalGrid2DPlot.h"
#include "view/plots/plot.h"

#include <QPushButton>
#include <QComboBox>
#include <QSpinBox>
#include <QPointF>
#include <QString>
#include <QAbstractButton>
#include <QTimer>

#include "model/scenarioReader.h"
#include "NumericFunctions.h"

using namespace guiUtilities;

namespace casaWizard
{

namespace sac
{

namespace thermal
{

ThermalMapsTabValidator::ThermalMapsTabValidator(ThermalController& controller, ThermalGuiTester* tester):
   SacMapsTabValidator(),
   m_controller(controller),
   m_tester(tester)
{

}

void ThermalMapsTabValidator::validateMapsTab()
{
   m_controller.gui().tabWidget()->setCurrentIndex(2);

   //Validation of selections & subsequent changes:
   //Selection/states of wells (checkboxes & amount & drawn in image)
   std::cout << " - Validating initial maps tab options: " << std::endl;
   validateInitialMapsTabOptions();

   //Gridding:
   //Set interpolation method (combobox selection & validation)
   //Set P (if enabled) (Read default & Set new & Read changed)
   //Set smoothing method (combobox selection & validation)
   //Set radius (if enabled) (Read default & Set new & Read changed)
   //Create 2D Grid (button & validation)
   std::cout << " - Validating gridding options: " << std::endl;
   validateGridding();

   std::cout <<" - Validating map options after gridding:" << std::endl;
   validateMapOptionsAfterGridding();

   //Optimized 3D run:
   //Select run optimized 3D (Dialog CPU's & Dialog SS & Waiting & validation)
   std::cout << "Validating 3D optimization: " << std::endl;
   validateRun3DOptimization();

   //Exports:
   //validate export optimized (button & zip comparison)
   //validate export Zycor (button & file comparison)
   std::cout << " - Validating export options: " << std::endl;
   validateExports();
}

void ThermalMapsTabValidator::validateInitialMapsTabOptions()
{
   validateTitleBeforeGridding();
   validateNumberOfPlottedWells();
   validateTooltipWithWellsVisible();
}

void ThermalMapsTabValidator::validateGridding()
{
   m_controller.gui().mapsTab()->pValue()->setValue(5);
   m_controller.gui().mapsTab()->interpolationType()->setCurrentText("Natural Neighbor");
   m_controller.gui().mapsTab()->smoothingType()->setCurrentText("Gaussian");
   m_controller.gui().mapsTab()->smoothingRadius()->setValue(10000);

   m_tester->reportStep(m_controller.scenario().mapsManager().pIDW() == 5, "Validate P value is set correctly");
   m_tester->reportStep(m_controller.scenario().mapsManager().interpolationMethod() == 1, "Validate interpolation method is set correctly");
   m_tester->reportStep(!m_controller.gui().mapsTab()->pValue()->isVisible(), "Validate spinbox for IDW value should be invisible when selecting Natural Neighbor algorithm");
   m_tester->reportStep(m_controller.scenario().mapsManager().smoothingOption() == 1, "Validate smoothing option is set correctly");
   m_tester->reportStep(m_controller.scenario().mapsManager().radiusSmoothing() == 10000, "Validate smoothing radius is set correctly");

   std::cout.rdbuf(m_tester->m_logStream.rdbuf()); //remove cout buffer so we wont get spammed with run results
   m_controller.gui().mapsTab()->createGridsButton()->click();
   std::cout.rdbuf(m_tester->m_testBuffer); //restore buffer

   const double expectedValue = 4.293109;
   const double actualValue = m_controller.gui().mapsTab()->mapsVisualisation()->TCHPPlot()->get2DView()->getValue(QPointF(180, 604.5));

   const bool correctMapValue = NumericFunctions::isEqual(actualValue, expectedValue, Utilities::Numerical::DefaultNumericalTolerance);
   m_tester->reportStep(correctMapValue, "Validate that the Expected map value (" + std::to_string(expectedValue) + ") is equal to the actual map value (" + std::to_string(actualValue) + ")");

}

void ThermalMapsTabValidator::validateRun3DOptimization()
{
   std::cout.rdbuf(m_tester->m_logStream.rdbuf());

   QTimer::singleShot(timeDelay,   [this](){processCpuPopup()         ;} );
   QTimer::singleShot(2*timeDelay, [this](){processSubSamplingPopup() ;} );
   getQChild<QAbstractButton>("Run optimized 3D", m_controller.gui().mapsTab())->click();

   bool stepPassed = !m_controller.scenario().wellTrajectoryManager().trajectories()[TrajectoryType::Optimized3D].first().depth().empty() &&
                     !m_controller.scenario().wellTrajectoryManager().trajectories()[TrajectoryType::Optimized3D].first().value().empty();

   m_tester->reportStep(stepPassed, "Validate generation of Optimized 3D Results");

   std::cout.rdbuf(m_tester->m_testBuffer);
}

void ThermalMapsTabValidator::validateMapOptionsAfterGridding()
{
   validateTitleAfterGridding();
   validateNumberOfPlottedWells();
   validateTooltipWithWellsVisible();
   validateTooltipWhenWellsInvisible();
   validateMapsTabPlottingOptions();
   validateWellHighlighting();
}

void ThermalMapsTabValidator::validateExports()
{
   validateOptimizedProjectZipExport();
   validateOptimizedMapZycorExport();
}

void ThermalMapsTabValidator::validateTitleBeforeGridding()
{
   const QString title = m_controller.gui().mapsTab()->mapsVisualisation()->TCHPPlot()->title()->text();
   m_tester->reportStep(title.startsWith("INPUT MAP"), "Validate that the title starts with 'INPUT MAP' before gridding is done");
}

void ThermalMapsTabValidator::validateTitleAfterGridding()
{
   const QString title = m_controller.gui().mapsTab()->mapsVisualisation()->TCHPPlot()->title()->text();
   m_tester->reportStep(!title.startsWith("INPUT MAP") && title != "", "Validate title after gridding is done");
}

void ThermalMapsTabValidator::validateNumberOfPlottedWells()
{
   auto isNumberOfPlottedWellsIsEqualTo = [this](int expectedNumber)
   {
      const int numberOfActualWells = m_controller.gui().mapsTab()->mapsVisualisation()->TCHPPlot()->get2DView()->getOptimizedTCHPs().size();
      const bool stepSucceeded = numberOfActualWells == expectedNumber;
      m_tester->reportStep(stepSucceeded, "Validate number of wells displayed, expected = " + std::to_string(expectedNumber) + ", actual = " + std::to_string(numberOfActualWells));
   };

   isNumberOfPlottedWellsIsEqualTo(3);

   dynamic_cast<QAbstractButton*>(m_controller.gui().mapsTab()->activeWellsTable()->cellWidget(1,0)->children()[1])->click();
   isNumberOfPlottedWellsIsEqualTo(2);
   getQChild<QAbstractButton>("Deselect All", m_controller.gui().mapsTab())->click();
   isNumberOfPlottedWellsIsEqualTo(0);
   getQChild<QAbstractButton>("Select All", m_controller.gui().mapsTab())->click();
   isNumberOfPlottedWellsIsEqualTo(3);
}

void ThermalMapsTabValidator::validateTooltipWithWellsVisible()
{
   bool stepSucceeded;

   TCHPMapToolTip* tooltip = getTooltipFromMap();
   if (tooltip == nullptr)
   {
      throw std::runtime_error("Tooltip object is not available");
   }
   for (const auto well : m_controller.scenario().calibrationTargetManager().activeAndIncludedWells())
   {
      createTooltipOnPoint(QPointF(well->x()/1000,well->y()/1000));

      stepSucceeded = tooltip->isVisible();
      m_tester->reportStep(stepSucceeded, "Validate tooltip is displayed, after creating at well location for well " + well->name().toStdString());

      validateTooltipHasLabelWithText(well->name(), tooltip);
      validateTooltipHasLabelWithText("(" + QString::number(well->x()/1000,'f', 1) + ", " + QString::number(well->y()/1000,'f', 1) + ")", tooltip);

      const auto selectedItems = m_controller.gui().mapsTab()->activeWellsTable()->selectedItems();
      const QString selectedWellName = m_controller.gui().mapsTab()->activeWellsTable()->item(selectedItems[0]->row(), 1)->text();
      stepSucceeded = selectedItems.size() == 1 && selectedWellName == well->name();
      m_tester->reportStep(stepSucceeded, "Validate highlighting of well " + well->name().toStdString() + " in the well list");
   }

   createTooltipOnPoint(QPointF(-9999, -9999)); // emulating a click at a 'non-well' location
   m_tester->reportStep(!tooltip->isVisible(), "Validate tooltip is invisble after clicking at a 'non-well' location");
}

void ThermalMapsTabValidator::validateTooltipWhenWellsInvisible()
{
   TCHPMapToolTip* tooltip = getTooltipFromMap();
   if (tooltip == nullptr)
   {
      throw std::runtime_error("Tooltip object is not available");
   }

   const QPointF pointWithinDomain(180, 605);
   tooltip->setDomainPosition(pointWithinDomain);

   createTooltipOnPoint(pointWithinDomain);
   m_tester->reportStep(!tooltip->isVisible(), "Validate tooltip is INVISIBLE when clicking at a non-well location within the domain and the wells visible checkbox IS checked");

   getQChild<QAbstractButton>("wellsVisible", getPlotOptions())->click();
   createTooltipOnPoint(pointWithinDomain);
   m_tester->reportStep(tooltip->isVisible(), "Validate tooltip is VISIBLE when clicking at a non-well location within the domain and the wells visible checkbox IS NOT checked");

   validateTooltipHasLabelWithText( "(" + QString::number(pointWithinDomain.x(),'f', 1) + ", " + QString::number(pointWithinDomain.y(),'f', 1) + ")", tooltip);
   validateTooltipHasLabelWithText("Map Value", tooltip);
   const double TCHP =  m_controller.gui().mapsTab()->mapsVisualisation()->TCHPPlot()->get2DView()->getValue(pointWithinDomain);
   validateTooltipHasLabelWithText("Value: " + QString::number(TCHP, 'f', 2), tooltip);
}

void ThermalMapsTabValidator::validateMapsTabPlottingOptions()
{
   QColor actualColor = m_controller.gui().mapsTab()->mapsVisualisation()->colormap().getColor(0.5, 0, 1);
   ColorMap expectedColorMap(ColorMapType::VIRIDIS);
   QColor expectedColor = expectedColorMap.getColor(0.5, 0, 1);
   m_tester->reportStep(actualColor == expectedColor, "Validate Viridis colormap");

   getPlotOptions()->colorMapSelection()->setCurrentIndex(1);
   actualColor = m_controller.gui().mapsTab()->mapsVisualisation()->colormap().getColor(0.5, 0, 1);
   expectedColorMap.setColorMapType("Gray scale");
   expectedColor = expectedColorMap.getColor(0.5, 0, 1);
   m_tester->reportStep(actualColor == expectedColor, "Validate Gray scale colormap");

   getPlotOptions()->colorMapSelection()->setCurrentIndex(2);
   actualColor = m_controller.gui().mapsTab()->mapsVisualisation()->colormap().getColor(0.5, 0, 1);

   expectedColorMap.setColorMapType("Rainbow");
   expectedColor = expectedColorMap.getColor(0.5, 0, 1);
   m_tester->reportStep(actualColor == expectedColor, "Validate Rainbow colormap");
}

void ThermalMapsTabValidator::validateWellHighlighting()
{
   auto activeWellsTable = m_controller.gui().mapsTab()->activeWellsTable();
   activeWellsTable->setItemSelected(activeWellsTable->item(0,1), true);
   activeWellsTable->setItemSelected(activeWellsTable->item(2,1), true);
   QVector<int> wellsHighlightedInPlot = m_controller.gui().mapsTab()->mapsVisualisation()->TCHPPlot()->get2DView()->selectedWells();
   m_tester->reportStep(wellsHighlightedInPlot.size() == 2, "Validate 2 wells are highlighted in the plot when selecting them in the table");

}

void ThermalMapsTabValidator::validateTooltipHasLabelWithText(const QString& labelText, const TCHPMapToolTip* tooltip)
{
   bool tooltipHasLabelWithText = false;
   for (const auto object : tooltip->children())
   {
      QLabel* label = dynamic_cast<QLabel*>(object);
      if (label != nullptr)
      {
         if (label->text() == labelText)
         {
            tooltipHasLabelWithText = true;
         }
      }
   }

   m_tester->reportStep(tooltipHasLabelWithText, "Validate tooltip contains the following text: " + labelText.toStdString());

}

void ThermalMapsTabValidator::validateOptimizedProjectZipExport()
{
   QTimer::singleShot(timeDelay, [this](){processDestinationFolderPopup();} );

   std::cout.rdbuf(m_tester->m_logStream.rdbuf());
   getQChild<QAbstractButton>("Export optimized", m_controller.gui().mapsTab())->click();
   std::cout.rdbuf(m_tester->m_testBuffer);

   QDir dir(m_controller.scenario().workingDirectory());
   QStringList zipList = dir.entryList({"*.zip"}, QDir::Files);
   m_tester->reportStep(zipList.size() == 1, "Validate exporting the optimized project generates a zip file");

   QStringList infoFileList = dir.entryList({"*info.txt"});
   m_tester->reportStep(infoFileList.size() == 1, "Validate exporting the optimized project generates an info text file");

}

void ThermalMapsTabValidator::validateOptimizedMapZycorExport()
{
   QTimer::singleShot(timeDelay, [this](){processDestinationFolderPopup();} );

   std::cout.rdbuf(m_tester->m_logStream.rdbuf());
   getQChild<QAbstractButton>("Export to Zycor", m_controller.gui().mapsTab())->click();
   std::cout.rdbuf(m_tester->m_testBuffer);

   QDir dir(m_controller.scenario().workingDirectory());
   QStringList zycorFileList = dir.entryList({"*.zyc"}, QDir::Files);
   m_tester->reportStep(zycorFileList.size() == 1, "Validate exporting the optimized maps generates a zycor file");

}

void ThermalMapsTabValidator::createTooltipOnPoint(const QPointF& point)
{
   emit m_controller.gui().mapsTab()->mapsVisualisation()->TCHPPlot()->get2DView()->toolTipCreated(point);
}

TCHPMapToolTip* ThermalMapsTabValidator::getTooltipFromMap()
{
   for (QObject* object : m_controller.gui().mapsTab()->mapsVisualisation()->TCHPPlot()->get2DView()->children())
   {
      TCHPMapToolTip* tooltip = dynamic_cast<TCHPMapToolTip*>(object);
      if (tooltip != nullptr)
      {
         return tooltip;
      }
   }

   return nullptr;
}

ThermalMapPlotOptions* ThermalMapsTabValidator::getPlotOptions()
{
   for (QObject* object : m_controller.gui().mapsTab()->mapsVisualisation()->children())
   {
      ThermalMapPlotOptions* plotOptions = dynamic_cast<ThermalMapPlotOptions*>(object);
      if (plotOptions != nullptr)
      {
         return plotOptions;
      }
   }

   return nullptr;
}

void ThermalMapsTabValidator::processDestinationFolderPopup()
{
   pressButtonInDialog<QDialog>(true);
}

void ThermalMapsTabValidator::processCpuPopup()
{
   setIntInInputDialog(12);
}

void ThermalMapsTabValidator::processSubSamplingPopup()
{
   setIntInInputDialog(10);
}

} // thermal

} // sac

} // casaWizard
