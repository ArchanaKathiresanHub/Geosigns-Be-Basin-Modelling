//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "t2zController.h"

#include "control/casaScriptWriter.h"
#include "control/ScriptRunController.h"

#include "model/functions/folderOperations.h"
#include "model/input/calibrationTargetCreator.h"
#include "model/input/cmbMapReader.h"
#include "model/input/cmbProjectReader.h"
#include "model/logger.h"
#include "model/output/cmbProjectWriter.h"
#include "model/output/T2ZInfoGenerator.h"
#include "model/output/workspaceGenerator.h"
#include "model/output/zycorWriter.h"
#include "model/LithologyScenario.h"
#include "model/scenarioBackup.h"
#include "model/script/depthConversionScript.h"
#include "model/script/sacScript.h"

#include "view/assets/LithoTabIDs.h"
#include "view/t2zTab.h"

#include "SDUWorkLoadManager.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QString>
#include <QTextStream>
#include <QTimer>
#include <QFileDialog>

namespace casaWizard
{

namespace sac
{

T2Zcontroller::T2Zcontroller(T2Ztab* t2zTab,
                             LithologyScenario& casaScenario,
                             ScriptRunController& scriptRunController,
                             QObject* parent) :
    QObject(parent),
    t2zTab_{t2zTab},
    casaScenario_{casaScenario},
    scriptRunController_{scriptRunController},
    t2zDir_{""},
    sourceDir_{""},
    lineNr_{0}
{
  connect(t2zTab_->pushButtonSACrunT2Z(),      SIGNAL(clicked()),                 this,   SLOT(slotPushButtonSACrunT2ZClicked()));
  connect(t2zTab_->exportT2ZScenario(),        SIGNAL(clicked()),                 this,   SLOT(slotExportT2ZScenarioClicked()));
  connect(t2zTab_->exportT2ZMapsToZycor(),     SIGNAL(clicked()),                 this,   SLOT(slotExportT2ZMapsToZycorClicked()));
  connect(t2zTab_->comboBoxReferenceSurface(), SIGNAL(currentIndexChanged(int)),  this,   SLOT(slotComboBoxReferenceSurfaceValueChanged(int)));
  connect(t2zTab_->spinBoxSubSampling(),       SIGNAL(valueChanged(int)),         this,   SLOT(slotSpinBoxSubSamplingValueChanged(int)));
  connect(t2zTab_->spinBoxNumberOfCPUs(),       SIGNAL(valueChanged(int)),         this,   SLOT(slotSpinBoxNumberOfCPUsValueChanged(int)));

  connect(t2zTab_->comboBoxProjectSelection(), SIGNAL(currentTextChanged(const QString&)),
          this,                                SLOT(slotComboBoxProjectSelectionTextChanged(const QString&)));
  connect(t2zTab_->comboBoxClusterSelection(), SIGNAL(currentTextChanged(const QString&)),
          this,                                SLOT(slotComboBoxClusterSelectionTextChanged(const QString&)));
}

void T2Zcontroller::slotPushButtonSACrunT2ZClicked()
{
  if (noProjectAvailable() || runCanceled())
  {
    return;
  }

  scenarioBackup::backup(casaScenario_);
  if (!prepareT2ZWorkSpace())
  {
    return;
  }
  casaScenario_.updateT2zLastSurface();
  runDepthConversion();
}

bool T2Zcontroller::noProjectAvailable() const
{
  if (t2zTab_->noProjectAvailable())
  {
    QMessageBox noProject(QMessageBox::Icon::Information,
                          "No project available",
                          "Select a project in the input tab, or open a saved scenario",
                          QMessageBox::Ok);
    noProject.exec();
  }

  return t2zTab_->noProjectAvailable();
}

bool T2Zcontroller::runCanceled() const
{
  if (referenceSurfaceHasNoTWTData() || projectHasNoTWTDataBelowReferenceSurface())
  {
    QMessageBox abortTimeDepth(QMessageBox::Icon::Warning,
                          "Not enough TWT data available for depth conversion",
                          "The reference surface and at least one surface below the reference surface need to have TWT data.",
                          QMessageBox::Ok);
    abortTimeDepth.exec();
    return true;
  }

  if (!casaScenario_.projectReader().basementSurfaceHasTWT())
  {
    QMessageBox continueAnyway(QMessageBox::Icon::Warning,
                          "No TWT data for Basement",
                          "Basement does not have TWT data, do you want to continue?",
                          QMessageBox::Yes | QMessageBox::No);
    if (continueAnyway.exec() != QMessageBox::Yes)
    {
      return true;
    }
  }

  return false;
}

bool T2Zcontroller::referenceSurfaceHasNoTWTData() const
{
  return !casaScenario_.projectReader().hasTWTData(casaScenario_.t2zReferenceSurface());
}

bool T2Zcontroller::projectHasNoTWTDataBelowReferenceSurface() const
{
  return casaScenario_.projectReader().lowestSurfaceWithTWTData() <= casaScenario_.t2zReferenceSurface();
}

bool T2Zcontroller::prepareT2ZWorkSpace()
{
  getSourceAndT2zDir();
  if (!casaWizard::functions::overwriteIfDirectoryExists(t2zDir_))
  {
    return false;
  }

  QDir(t2zDir_).removeRecursively();
  workspaceGenerator::copyDir(sourceDir_ , t2zDir_);
  setSubSampling();
  return true;
}

void T2Zcontroller::getSourceAndT2zDir()
{
  sourceDir_ = "";
  t2zDir_ = "";

  if (casaScenario_.t2zRunOnOriginalProject())
  {
    sourceDir_ = casaScenario_.workingDirectory();
    t2zDir_ = casaScenario_.workingDirectory() + "/T2Z_OnOriginalProject";
  }
  else
  {
    sourceDir_ = casaScenario_.optimizedProjectDirectory();
    t2zDir_ = casaScenario_.workingDirectory() + "/T2Z_step2";
  }
}

void T2Zcontroller::setSubSampling()
{
  const QString projectFilename{QDir::separator() + QFileInfo(casaScenario_.project3dPath()).fileName()};
  CMBProjectWriter writer(t2zDir_ + projectFilename);
  writer.setScaling(casaScenario_.t2zSubSampling(), casaScenario_.t2zSubSampling());
}

void T2Zcontroller::runDepthConversion()
{
  DepthConversionScript depthConversion{casaScenario_, t2zDir_, workloadmanagers::WorkLoadManagerType::AUTO};

  const char* slot = nullptr;
  if (casaScenario_.clusterName() != "LOCAL")
  {
    lineNr_ = 0;
    slot = SLOT(slotOutputFile());
  }

  if (!scriptRunController_.runScript(depthConversion, this, slot))
  {
    QDir(t2zDir_).removeRecursively();
    return;
  }
  Logger::log() << "Done!" << Logger::endl();
  scenarioBackup::backup(casaScenario_);
}

void T2Zcontroller::slotOutputFile()
{
  QFile file(t2zDir_ + "/fastdepthconversion-output-rank-0.log");
  if (file.exists())
  {
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    int i = 1;
    QTextStream stream(&file);
    while (!stream.atEnd())
    {
      QString line = stream.readLine();
      if (i > lineNr_)
      {
        lineNr_ = i;
        Logger::log() << line << Logger::endl();
      }
      ++i;
    }
    file.close();
  }
}

void T2Zcontroller::slotComboBoxReferenceSurfaceValueChanged(int referenceSurface)
{
  casaScenario_.setT2zReferenceSurface(referenceSurface);
}

void T2Zcontroller::slotSpinBoxSubSamplingValueChanged(int subSampling)
{
  casaScenario_.setT2zSubSampling(subSampling);
}

void T2Zcontroller::slotComboBoxProjectSelectionTextChanged(const QString& projectSelection)
{
  casaScenario_.setT2zRunOnOriginalProject(projectSelection == "Original Project");
}

void T2Zcontroller::slotSpinBoxNumberOfCPUsValueChanged(int numberOfCPUs)
{
  casaScenario_.setT2zNumberCPUs(numberOfCPUs);
}

void T2Zcontroller::slotComboBoxClusterSelectionTextChanged(const QString& clusterName)
{
  casaScenario_.setClusterName(clusterName);
}

void T2Zcontroller::slotExportT2ZScenarioClicked()
{
  getSourceAndT2zDir();
  const QStringList iterations = QDir(t2zDir_).entryList({"T2Zcal_*"}, QDir::Filter::Dirs, QDir::Time | QDir::Reversed);
  if (iterations.empty())
  {
    Logger::log() << "No T2Z results are available for export.\n" << Logger::endl();
    return;
  }

  QDir sourceDir(t2zDir_ + "/" + iterations.last());
  CMBProjectReader projectReader;
  T2ZInfoGenerator t2zInfoGenerator(casaScenario_, projectReader);
  functions::exportScenarioToZip(sourceDir, casaScenario_.workingDirectory(), casaScenario_.project3dFilename(), t2zInfoGenerator);
}

void T2Zcontroller::slotExportT2ZMapsToZycorClicked()
{
  getSourceAndT2zDir();
  const QStringList iterations = QDir(t2zDir_).entryList({"T2Zcal_*"}, QDir::Filter::Dirs, QDir::Time | QDir::Reversed);
  if (iterations.empty())
  {
    Logger::log() << "No T2Z results are available for export.\n" << Logger::endl();
    return;
  }

  QDir sourceDir(t2zDir_ + "/" + iterations.last());

  QString targetPath = QFileDialog::getExistingDirectory(nullptr, "Choose a location for the exported zycor maps", QDir::currentPath(),
                                                           QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

  QDir targetDir(targetPath);
  if (!targetDir.exists() || targetPath == "")
  {
    Logger::log() << "Target directory is not set" << Logger::endl();
    return;
  }

  CMBProjectReader projectReader;
  projectReader.load(sourceDir.path() + "/" + casaScenario_.project3dFilename());

  CMBMapReader mapReader;
  mapReader.load((sourceDir.path() + "/" + casaScenario_.project3dFilename()).toStdString());
  double xMin = 0.0;
  double xMax = 0.0;
  double yMin = 0.0;
  double yMax = 0.0;
  long numI = 0;
  long numJ = 0;
  mapReader.getMapDimensions(xMin, xMax, yMin, yMax, numI, numJ);
  const casaWizard::MapMetaData metaData(xMin, xMax, yMin, yMax, numI, numJ);

  ZycorWriter writer;
  for (const QString& mapName : projectReader.mapNamesT2Z())
  {
    const VectorVectorMap map = mapReader.getMapData(mapName.toStdString());
    writer.writeToFile(targetDir.path().toStdString()+"/" + mapName.toStdString() + ".zyc", map.getData(), metaData);
  }

  Logger::log() << "Finished exporting converted depth maps to Zycor" << Logger::endl();
}

void T2Zcontroller::slotUpdateTabGUI(int tabID)
{
  if (tabID != static_cast<int>(lithology::TabID::T2Z))
  {
    return;
  }

  t2zTab_->comboBoxReferenceSurface()->setCurrentIndex(casaScenario_.t2zReferenceSurface());
  t2zTab_->comboBoxProjectSelection()->setCurrentText(casaScenario_.t2zRunOnOriginalProject() ? "Original Project" : "Optimized Project");
  t2zTab_->spinBoxSubSampling()->setValue(casaScenario_.t2zSubSampling());
  t2zTab_->spinBoxNumberOfCPUs()->setValue(casaScenario_.t2zNumberCPUs());
  t2zTab_->comboBoxClusterSelection()->setCurrentText(casaScenario_.clusterName());

  updateSurfaces();
  updateProjectSelectionOptions();
}

void T2Zcontroller::updateSurfaces()
{
  QStringList surfaces = casaScenario_.projectReader().surfaceNames();
  t2zTab_->setReferenceSurfaces(surfaces);
}

void T2Zcontroller::updateProjectSelectionOptions()
{
  QStringList projectSelectionOptions;
  if (QDir(casaScenario_.optimizedProjectDirectory()).exists())
  {
    projectSelectionOptions.push_back("Optimized Project");
  }
  if (casaScenario_.project3dFilename() != "")
  {
    projectSelectionOptions.push_back("Original Project");
  }

  t2zTab_->addProjectSelectionOptions(projectSelectionOptions);
}

} // namespace sac

} // namespace casaWizard
