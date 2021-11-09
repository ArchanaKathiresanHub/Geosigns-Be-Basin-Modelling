//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "importWellPopupVSETController.h"

#include "model/input/calibrationTargetCreator.h"
#include "model/input/extractWellDataVSET.h"
#include "model/input/importOptions.h"
#include "view/importWellPopupVSET.h"

#include <QMessageBox>

namespace casaWizard
{

ImportWellPopupVSETController::ImportWellPopupVSETController(QObject* parent, CasaScenario& casaScenario) :
  ImportWellPopupController(parent, casaScenario),
  options_{},
  importWellPopup_{new ImportWellPopupVSET()}
{
  connect(importWellPopup_, SIGNAL(acceptedClicked()), this, SLOT(slotAcceptedClicked()));
}

ImportWellPopupVSETController::~ImportWellPopupVSETController()
{
  delete importWellPopup_;
  importWellPopup_ = nullptr;
}

int ImportWellPopupVSETController::executeImportWellPopup()
{  
  importWellPopup_->updateTableUsingOptions(options_);

  return importWellPopup_->exec();
}

ImportWellPopup* ImportWellPopupVSETController::importWellPopup() const
{
  return importWellPopup_;
}

void ImportWellPopupVSETController::importWellsToCalibrationTargetManager(const QString& fileName, CalibrationTargetManager& calibrationTargetManager)
{  
  ExtractWellDataVSET extractor(fileName);
  CalibrationTargetCreator targetCreator(casaScenario_, calibrationTargetManager, extractor);
  targetCreator.readMetaDataFromFile();
  options_ = extractor.getImportOptions();

  if (executeImportWellPopup() != QDialog::Accepted)
  {
    return;
  }

  extractor.setImportOptions(options_);
  importOnSeparateThread(targetCreator);  
}

void ImportWellPopupVSETController::slotAcceptedClicked()
{  
  const bool depthNotTWTfromfile = options_.depthNotTWT;
  options_ = importWellPopup_->getImportOptions();

  QMessageBox notMatchingProperty(QMessageBox::Icon::Question,
                                  "Property selection verification",
                                  "The property read from file is " + QString(depthNotTWTfromfile?"Depth":"TwoWayTime") +
                                  ", are you sure the data is " + QString(options_.depthNotTWT?"Depth":"TwoWayTime") + "?",
                                  QMessageBox::Yes | QMessageBox::No );
  if (notMatchingProperty.exec() == QMessageBox::Yes)
  {
    importWellPopup_->accept();
  }
  else
  {
    options_.depthNotTWT = depthNotTWTfromfile;
  }
}

} // namespace casaWizard

