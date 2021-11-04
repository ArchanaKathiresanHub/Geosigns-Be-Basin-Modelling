//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "importWellPopupXlsxController.h"
#include "view/importWellPopupXlsx.h"
#include "model/input/calibrationTargetCreator.h"
#include "model/input/extractWellDataXlsx.h"

namespace casaWizard
{

ImportWellPopupXlsxController::ImportWellPopupXlsxController(QObject* parent, CasaScenario& casaScenario) :
  ImportWellPopupController(parent, casaScenario),
  importWellPopup_(new ImportWellPopupXlsx())
{
  connect(importWellPopup_, SIGNAL(acceptedClicked()), this, SLOT(slotAcceptedClicked()));
}

ImportWellPopupXlsxController::~ImportWellPopupXlsxController()
{
  delete importWellPopup_;
  importWellPopup_ = nullptr;
}

int ImportWellPopupXlsxController::executeImportWellPopup()
{
  QStringList units;
  QStringList defaultCauldronNames;
  for (const auto& propertyName : importCalibrationTargetManager_.userNameToCauldronNameMapping().keys())
  {
    if (defaultCauldronNames.contains(importCalibrationTargetManager_.getCauldronPropertyName(propertyName)))
    {
      defaultCauldronNames.push_back("Unknown");
    }
    else
    {
      defaultCauldronNames.push_back(importCalibrationTargetManager_.getCauldronPropertyName(propertyName));
    }
    units.push_back(importCalibrationTargetManager_.getUnit(propertyName));
  }

  importWellPopup_->updateTable(importCalibrationTargetManager_.userNameToCauldronNameMapping().keys(), defaultCauldronNames,
                                {"TwoWayTime", "GammaRay", "BulkDensity", "SonicSlowness",
                                 "Pressure", "Temperature", "VRe", "Velocity", "DT_FROM_VP", "TWT_FROM_DT", "Unknown"});
  return importWellPopup_->exec();
}

ImportWellPopup*ImportWellPopupXlsxController::importWellPopup() const
{
  return importWellPopup_;
}

bool ImportWellPopupXlsxController::importWellsToCalibrationTargetManager(const QString& fileName)
{
  ExtractWellDataXlsx extractor(fileName);
  CalibrationTargetCreator targetCreator(casaScenario_, importCalibrationTargetManager_, extractor);
  targetCreator.readMetaDataFromFile();

  if (executeImportWellPopup() != QDialog::Accepted)
  {
    return false;
  }

  importOnSeparateThread(targetCreator);
  return true;
}

void ImportWellPopupXlsxController::slotAcceptedClicked()
{
  addNewMapping();
  importWellPopup_->accept();
}

} // namespace casaWizard
