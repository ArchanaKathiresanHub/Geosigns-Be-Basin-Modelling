//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "importWellPopupController.h"

#include "model/calibrationTarget.h"
#include "model/casaScenario.h"
#include "view/importWellPopup.h"
#include "view/importWellPropertyTable.h"

#include <QMessageBox>
#include <QSet>
#include <QString>

namespace casaWizard
{

ImportWellPopupController::ImportWellPopupController(ImportWellPopup* importwellPopup, QObject* parent):
  QObject(parent),
  importCalibrationTargetManager_{},
  importWellPopup_{importwellPopup},
  targetVariableUserNames_{}
{
  connect(importWellPopup_,                         SIGNAL(acceptedClicked()),
          this,                                     SLOT(slotAcceptedClicked()));
}

int ImportWellPopupController::executeImportWellPopup()
{
  for (const CalibrationTarget* const target : importCalibrationTargetManager_.calibrationTargets())
  {
    targetVariableUserNames_.insert(target->propertyUserName());
  }

  QStringList defaultCauldronNames;
  for (const QString& userName : targetVariableUserNames_)
  {
    defaultCauldronNames.push_back(importCalibrationTargetManager_.getCauldronPropertyName(userName));
  }

  importWellPopup_->updateTable(targetVariableUserNames_.toList(), defaultCauldronNames,
                                {"TWT_FROM_DT","TwoWayTime", "GammaRay", "BulkDensity", "SonicSlowness", "DT_FROM_VP",
                                 "Pressure", "Temperature", "VRe", "Velocity", "Unknown"});
  return importWellPopup_->exec();
}

void ImportWellPopupController::slotAcceptedClicked()
{
  const QMap<QString,QString> newMapping = importWellPopup_->propertyMappingTable()->getCurrentMapping();

  for (const QString& key : newMapping.keys())
  {    
    QString cauldronName = newMapping[key];
    // User name is the Cauldron name, or "TWT_FROM_DT" or "DT_FROM_VP"
    importCalibrationTargetManager_.renameUserPropertyNameInWells(key, cauldronName);

    // Real Cauldron property name, so remove the fake ones
    if (cauldronName == "TWT_FROM_DT") cauldronName="TwoWayTime";
    if (cauldronName == "DT_FROM_VP")  cauldronName="SonicSlowness";
    importCalibrationTargetManager_.addToMapping(newMapping[key], cauldronName);
  }

  importCalibrationTargetManager_.removeCalibrationTargetsWithUnknownPropertyUserName();

  targetVariableUserNames_.clear();
  importWellPopup_->accept();
}

CalibrationTargetManager& ImportWellPopupController::importCalibrationTargetManager()
{
  return importCalibrationTargetManager_;
}

} // namespace casaWizard
