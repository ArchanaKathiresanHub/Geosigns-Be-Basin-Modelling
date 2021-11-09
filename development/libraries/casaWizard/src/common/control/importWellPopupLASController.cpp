//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "importWellPopupLASController.h"

#include "model/input/calibrationTargetCreator.h"
#include "model/input/defaultUnitConversions.h"
#include "model/input/extractWellDataLAS.h"
#include "model/input/importOptions.h"
#include "view/importWellPopupLAS.h"

namespace casaWizard
{

ImportWellPopupLASController::ImportWellPopupLASController(QObject* parent, CasaScenario& casaScenario) :
  ImportWellPopupController(parent, casaScenario),
  importWellPopup_{new ImportWellPopupLAS()}
{
  connect(importWellPopup_, SIGNAL(acceptedClicked()), this, SLOT(slotAcceptedClicked()));
}

ImportWellPopupLASController::~ImportWellPopupLASController()
{
  delete importWellPopup_;
  importWellPopup_ = nullptr;
}

void ImportWellPopupLASController::importWellsToCalibrationTargetManager(const QStringList& fileNames, CalibrationTargetManager& calibrationTargetManager)
{
  ExtractWellDataLAS extractor(fileNames, options_);
  CalibrationTargetCreator targetCreator(casaScenario_, calibrationTargetManager, extractor);
  targetCreator.readMetaDataFromFile();

  QStringList propertyUserNames;
  QStringList defaultCauldronNames;
  QStringList units;
  targetCreator.getNamesAndUnits(options_.depthUserPropertyName, propertyUserNames, defaultCauldronNames, units);

  if (executeImportWellPopup(propertyUserNames, defaultCauldronNames, units) != QDialog::Accepted)
  {
    return;
  }

  targetCreator.addNewMapping(importWellPopup_->getCurrentMapping());
  importOnSeparateThread(targetCreator);
}

int ImportWellPopupLASController::executeImportWellPopup(const QStringList& propertyUserNames, const QStringList& defaultCauldronNames, const QStringList& units)
{
  if (options_.lasVersion != 2.0)
  {
    QMessageBox removeFolder(QMessageBox::Icon::Warning,
                             "Unsupported LAS version",
                             "The detected LAS vesion of the input file is different from the supported 2.0 version. This might result in incorrect import data for this file. Continue anyway?",
                             QMessageBox::Yes | QMessageBox::No );
    if (removeFolder.exec() == QMessageBox::No)
    {
      return QDialog::Rejected;
    }
  }

  importWellPopup_->updatePropertyTableWithUnits(propertyUserNames, defaultCauldronNames,
                                {"TwoWayTime", "GammaRay", "BulkDensity", "SonicSlowness",
                                 "Pressure", "Temperature", "VRe", "Velocity", "Depth",  "DT_FROM_VP", "TWT_FROM_DT", "Unknown"}, units, getUnitConversions(units));
  if (options_.singleFile)
  {
    importWellPopup_->setElevationInfo(options_.elevationCorrection, options_.elevationCorrectionUnit, options_.referenceCorrection, options_.referenceCorrectionUnit, options_.allFilesHaveElevation);
  }
  else
  {
    importWellPopup_->setElevationInfo(options_.allFilesHaveElevation);
  }
  return importWellPopup_->exec();
}

QVector<double> ImportWellPopupLASController::getUnitConversions(const QStringList& units)
{
  QVector<double> unitConversions;
  for (const QString& unit : units)
  {
    unitConversions.push_back(defaultUnitConversions.value(unit, -1.0));
  }

  return unitConversions;
}

ImportWellPopup* ImportWellPopupLASController::importWellPopup() const
{
  return importWellPopup_;
}

void ImportWellPopupLASController::slotAcceptedClicked()
{  
  options_.correctForElevation = importWellPopup_->correctForElevation();
  options_.userPropertyNameToUnitConversion = importWellPopup_->getUnitConversions();
  const QMap<QString, QString>& mapping = importWellPopup_->getCurrentMapping();
  for (const QString& userPropertyName : mapping.keys())
  {
    if (mapping[userPropertyName] == "Depth")
    {
      options_.depthUserPropertyName = userPropertyName;
    }
  }
  importWellPopup_->accept();
}

} // namespace casaWizard

