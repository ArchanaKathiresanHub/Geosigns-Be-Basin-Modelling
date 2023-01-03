//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "SacMapsController.h"

#include "assets/SacVisualisationController.h"
#include "control/assets/activeWellsController.h"
#include "control/assets/run3dCaseController.h"
#include "control/casaScriptWriter.h"
#include "control/ScriptRunController.h"

#include "model/functions/folderOperations.h"
#include "model/logger.h"
#include "model/output/SacInfoGenerator.h"
#include "model/SacMapsManager.h"
#include "model/SacScenario.h"
#include "model/scenarioBackup.h"
#include "model/script/Generate3DScenarioScript.h"

#include "view/assets/activeWellsTable.h"
#include "view/SacMapsTab.h"

#include <QComboBox>
#include <QFileDialog>
#include <QModelIndexList>
#include <QPushButton>
#include <QSpinBox>
#include <QDir>

namespace casaWizard
{

namespace sac
{

SacMapsController::SacMapsController(ScriptRunController& scriptRunController,
                                     QObject* parent) :
   QObject(parent),
   m_scriptRunController{scriptRunController},
   m_activeWellsController{nullptr}
{}

void SacMapsController::init()
{
   m_activeWellsController = new ActiveWellsController(mapsTab()->activeWellsTable(), scenario(), this);

   connect(mapsTab()->activeWellsTable(), SIGNAL(checkBoxSelectionChanged()), this, SLOT(slotUpdateWellSelection()));
   connect(&visualisationController(), SIGNAL(wellClicked(const QString&)), this, SLOT(slotWellClicked(const QString&)));
   connect(mapsTab()->pushSelectAllWells(), SIGNAL(clicked()), this, SLOT(slotSelectAllWells()));
   connect(mapsTab()->pushClearWellSelection(), SIGNAL(clicked()), this, SLOT(slotDeselectAllWells()));
   connect(mapsTab()->pValue(),            SIGNAL(valueChanged(int)),        this, SLOT(slotPvalueChanged(int)));
   connect(mapsTab()->smoothingType(),     SIGNAL(currentIndexChanged(int)), this, SLOT(slotSmoothingTypeCurrentIndexChanged(int)));
   connect(mapsTab()->smoothingRadius(),   SIGNAL(valueChanged(int)),        this, SLOT(slotSmoothingRadiusValueChanged(int)));
   connect(mapsTab()->createGridsButton(), SIGNAL(clicked()),                this, SLOT(slotGenerateMaps()));
   connect(mapsTab()->interpolationType(), SIGNAL(currentIndexChanged(int)), this, SLOT(slotInterpolationTypeCurrentIndexChanged(int)));
   connect(mapsTab()->buttonExportOptimized(), SIGNAL(clicked()), this, SLOT(slotExportOptimized()));
   connect(mapsTab()->buttonRunOptimized(),    SIGNAL(clicked()), this, SLOT(slotRunOptimized()));
   connect(mapsTab()->buttonExportOptimizedToZycor(), SIGNAL(clicked()), this, SLOT(slotExportOptimizedToZycor()));
   connect(&visualisationController(), SIGNAL(clearWellListHighlightSelection()), m_activeWellsController, SLOT(slotClearWellListHighlightSelection()));
   connect(mapsTab()->activeWellsTable(), SIGNAL(itemSelectionChanged()), this, SLOT(slotUpdateBirdView()));
}

void SacMapsController::refreshGUI()
{
   mapsTab()->interpolationType()->setCurrentIndex(mapsManager().interpolationMethod());
   mapsTab()->pValue()->setValue(mapsManager().pIDW());
   mapsTab()->smoothingType()->setCurrentIndex(mapsManager().smoothingOption());
   mapsTab()->smoothingRadius()->setValue(mapsManager().radiusSmoothing());
   visualisationController().updateSelectedWells({});
}

void SacMapsController::slotUpdateBirdView()
{
   QVector<int> selectedWells = scenario().getIncludedWellIndicesFromSelectedWells(getSelectedWellIndices());
   visualisationController().updateSelectedWells(selectedWells);
}

void SacMapsController::slotInterpolationTypeCurrentIndexChanged(int interpolationType)
{
   mapsManager().setInterpolationMethod(interpolationType);
}

void SacMapsController::slotPvalueChanged(int pIDW)
{
   mapsManager().setPIDW(pIDW);
}

void SacMapsController::slotSmoothingTypeCurrentIndexChanged(int smoothingType)
{
   mapsManager().setSmoothingOption(smoothingType);
}

void SacMapsController::slotSmoothingRadiusValueChanged(int smoothingRadius)
{
   mapsManager().setRadiusSmoothing(smoothingRadius);
}

void SacMapsController::slotUpdateWellSelection()
{
   visualisationController().updateBirdsView();
}

void SacMapsController::slotWellClicked(const QString& wellName)
{
   mapsTab()->highlightWell(wellName);
}

void SacMapsController::slotExportOptimized()
{
   if (scenario().project3dPath().isEmpty())
   {
      return;
   }

   QDir sourceDir(scenario().optimizedProjectDirectory());
   functions::exportScenarioToZip(sourceDir, scenario().workingDirectory(), scenario().project3dFilename(), *infoGenerator());
}

void SacMapsController::slotExportOptimizedToZycor()
{
   QDir sourceDir(scenario().optimizedProjectDirectory());
   if (!sourceDir.exists())
   {
      Logger::log() << "No optimized project is available for export" << Logger::endl();
      return;
   }

   QString targetPath = QFileDialog::getExistingDirectory(nullptr, "Choose a location for the exported zycor maps", scenario().defaultFileDialogLocation(),
                                                          QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

   QDir targetDir(targetPath);
   if (!targetDir.exists() || targetPath == "")
   {
      Logger::log() << "Target directory is not set" << Logger::endl();
      return;
   }

   scenario().exportOptimizedMapsToZycor(targetPath);

   Logger::log() << "Finished exporting optimized map(s) to Zycor" << Logger::endl();
}

void SacMapsController::slotRunOptimized()
{
   if (scenario().project3dPath().isEmpty())
   {
      return;
   }

   const QString baseDirectory{scenario().optimizedProjectDirectory()};
   Run3dCaseController run3dCaseController(scenario(), scriptRunController());
   if (run3dCaseController.run3dCase(baseDirectory, true))
   {
      scenarioBackup::backup(scenario());
   }
}

void SacMapsController::slotSelectAllWells()
{
   m_activeWellsController->slotSelectAllWells();
   slotUpdateWellSelection();
}

void SacMapsController::slotDeselectAllWells()
{
   m_activeWellsController->slotDeselectAllWells();
   slotUpdateWellSelection();
}

void SacMapsController::slotGenerateMaps()
{
   Logger::log() << "Start saving optimized case" << Logger::endl();

   scenarioBackup::backup(scenario());
   std::unique_ptr<Generate3DScenarioScript> saveOptimized = generate3DScenarioScript();
   if (!casaScriptWriter::writeCasaScript(*saveOptimized) ||
       !m_scriptRunController.runScript(*saveOptimized))
   {
      refreshGUI();
      return;
   }
   scenarioBackup::backup(scenario());

   Logger::log() << "Done!" << Logger::endl();
   refreshGUI();
}

void SacMapsController::validateWellsHaveOptimized()
{
   for (int i = 0; i < mapsTab()->numberOfActiveWells(); i++)
   {
      if (!scenario().hasOptimizedSuccessfully(i))
      {
         mapsTab()->disableWellAtIndex(i);
      }
   }
}

QVector<int> SacMapsController::getSelectedWellIndices()
{
   QModelIndexList indices = mapsTab()->activeWellsTable()->selectionModel()->selectedIndexes();

   QVector<int> wellIndices;
   for(const QModelIndex& index : indices)
   {
      wellIndices.push_back(index.row());
   }

   return wellIndices;
}

ScriptRunController& SacMapsController::scriptRunController()
{
   return m_scriptRunController;
}

ActiveWellsController* SacMapsController::activeWellsController()
{
   return m_activeWellsController;
}

} // namespace sac

} // namespace casaWizard
