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

int ImportWellPopupXlsxController::executeImportWellPopup(const QStringList& propertyUserNames, const QStringList& defaultCauldronNames)
{  
  importWellPopup_->updateTable(propertyUserNames, defaultCauldronNames,
                                {"TwoWayTime", "GammaRay", "BulkDensity", "SonicSlowness",
                                 "Pressure", "Temperature", "VRe", "Velocity", "DT_FROM_VP", "TWT_FROM_DT", "Unknown"});
  return importWellPopup_->exec();
}

ImportWellPopup*ImportWellPopupXlsxController::importWellPopup() const
{
  return importWellPopup_;
}

void ImportWellPopupXlsxController::importWellsToCalibrationTargetManager(const QStringList& fileNames, CalibrationTargetManager& calibrationTargetManager)
{  
  ExtractWellDataXlsx extractor(fileNames[0]);
  CalibrationTargetCreator targetCreator(casaScenario_, calibrationTargetManager, extractor);
  targetCreator.readMetaDataFromFile();

  QStringList propertyUserNames;
  QStringList defaultCauldronNames;
  QStringList units;
  targetCreator.getNamesAndUnits("", propertyUserNames, defaultCauldronNames, units);

  if (executeImportWellPopup(propertyUserNames, defaultCauldronNames) != QDialog::Accepted)
  {
    return;
  }

  targetCreator.addNewMapping(importWellPopup_->getCurrentMapping());
  importOnSeparateThread(targetCreator);  
}

void ImportWellPopupXlsxController::slotAcceptedClicked()
{  
  importWellPopup_->accept();
}

} // namespace casaWizard
