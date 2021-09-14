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
                                {"TwoWayTime", "GammaRay", "BulkDensity", "SonicSlowness",
                                 "Pressure", "Temperature", "VRe", "Velocity", "Unknown"});
  return importWellPopup_->exec();
}


void ImportWellPopupController::slotAcceptedClicked()
{
  const QMap<QString,QString> newMapping = importWellPopup_->propertyMappingTable()->getCurrentMapping();

  if (mappingContainsUnknowns(newMapping))
  {
    QMessageBox unknownProperties(QMessageBox::Icon::Information,
                          "Undefined properties",
                          "At least one property does not have a valid Cauldron Property Name and is set to 'Unknown'. "
                          "Please provide a valid Cauldron Property Name for all properties",
                          QMessageBox::Ok);
    unknownProperties.exec();
    return;
  }
  if (!overwrittenMappingKeys(newMapping).empty())
  {
    QString message = "The properties with the following user names are already defined, either by previously imported calibration targets"
                      " or by the default values: \n\n";
    for (const QString& propertyUserName : overwrittenMappingKeys(newMapping))
    {
      message += propertyUserName + "\n";
    }
    message += "\nTheir 'Cauldron Property Name' will be overwritten by the current settings. Do you want to continue anyway? If not, press 'No' and rename the properties listed above.";

    QMessageBox overwriteMapping(QMessageBox::Icon::Question,
                             "Overwrite Cauldron Property Name?",
                             message,
                             QMessageBox::Yes | QMessageBox::No );

    if (overwriteMapping.exec() != QMessageBox::Yes)
    {
      return;
    }
  }

  int counter = 0;
  for (const QString& key : newMapping.keys())
  {
    importCalibrationTargetManager_.addToMapping(key, newMapping[key]);
    counter++;
  }

  renameUserPropertyNames();

  targetVariableUserNames_.clear();
  importWellPopup_->accept();
}

bool ImportWellPopupController::mappingContainsUnknowns(const QMap<QString, QString>& mapping) const
{
  for (const QString& key : mapping.keys())
  {
    if (mapping[key] == "Unknown")
    {
      return true;
    }
  }
  return false;
}

QVector<QString> ImportWellPopupController::overwrittenMappingKeys(const QMap<QString, QString>& newMapping) const
{
  const QMap<QString, QString> originalMapping = importCalibrationTargetManager_.userNameToCauldronNameMapping();

  QVector<QString> overwrittenKeys;
  for (const QString& key : newMapping.keys())
  {
    if (originalMapping.value(key, "Unknown") != "Unknown" && originalMapping[key] != newMapping[key])
    {
      overwrittenKeys.push_back(key);
    }
  }

  return overwrittenKeys;
}

void ImportWellPopupController::renameUserPropertyNames()
{
  for (int i = 0; i < targetVariableUserNames_.size(); i++)
  {
    importCalibrationTargetManager_.renameUserPropertyNameInWells(targetVariableUserNames_.toList()[i], getRenamedUserPropertyName(i));
  }
}

QString ImportWellPopupController::getRenamedUserPropertyName(const int rowNumber)
{
 return importWellPopup_->propertyMappingTable()->item(rowNumber, 0)->text();
}

CalibrationTargetManager& ImportWellPopupController::importCalibrationTargetManager()
{
  return importCalibrationTargetManager_;
}

} // namespace casaWizard
