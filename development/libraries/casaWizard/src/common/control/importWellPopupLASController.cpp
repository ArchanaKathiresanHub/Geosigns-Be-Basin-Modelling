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

bool ImportWellPopupLASController::importWellsToCalibrationTargetManager(const QString& fileName)
{
  ExtractWellDataLAS extractor(fileName, options_);
  CalibrationTargetCreator targetCreator(casaScenario_, importCalibrationTargetManager_, extractor);
  targetCreator.readMetaDataFromFile();

  if (executeImportWellPopup() != QDialog::Accepted)
  {
    return false;
  }

  importOnSeparateThread(targetCreator);
  return true;
}

int ImportWellPopupLASController::executeImportWellPopup()
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

  QStringList units;
  QStringList defaultCauldronNames;
  for (const auto& propertyName : importCalibrationTargetManager_.userNameToCauldronNameMapping().keys())
  {
    if (propertyName == options_.depthUserPropertyName)
    {
      defaultCauldronNames.push_back("Depth");
    }
    else if (defaultCauldronNames.contains(importCalibrationTargetManager_.getCauldronPropertyName(propertyName)))
    {
      defaultCauldronNames.push_back("Unknown");
    }
    else
    {
      defaultCauldronNames.push_back(importCalibrationTargetManager_.getCauldronPropertyName(propertyName));
    }

    units.push_back(importCalibrationTargetManager_.getUnit(propertyName));
  }

  importWellPopup_->updatePropertyTableWithUnits(importCalibrationTargetManager_.userNameToCauldronNameMapping().keys(), defaultCauldronNames,
                                {"TwoWayTime", "GammaRay", "BulkDensity", "SonicSlowness",
                                 "Pressure", "Temperature", "VRe", "Velocity", "Depth",  "DT_FROM_VP", "TWT_FROM_DT", "Unknown"}, units, getUnitConversions(units));
  importWellPopup_->setElevationInfo(options_.elevationCorrection, options_.elevationCorrectionUnit, options_.referenceCorrection, options_.referenceCorrectionUnit);
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
  addNewMapping();
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

