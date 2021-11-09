//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include "importOptions.h"
#include "model/calibrationTargetManager.h"

#include <QMap>

#include <memory>

class QString;

namespace casaWizard
{

class CasaScenario;
class ExtractWellData;

class CalibrationTargetCreator
{
public:
  CalibrationTargetCreator(CasaScenario& casaScenario,
                           CalibrationTargetManager& calibrationTargetManager,
                           ExtractWellData& extractWellData);
  ~CalibrationTargetCreator();

  void createFromFile();
  void readMetaDataFromFile();
  void addNewMapping(const QMap<QString, QString> newMapping);
  void getNamesAndUnits(const QString& depthUserPropertyName, QStringList& propertyNames, QStringList& defaultCauldronNames, QStringList& units);

private:
  void addWellDataToCalibrationTargetManager();
  void addWellMetaDataToCalibrationTargetManager();  

  CalibrationTargetManager importCalibrationTargetManager_;
  CalibrationTargetManager& calibrationTargetManager_;
  CasaScenario& casaScenario_;
  ExtractWellData& extractWellData_;
};

} // namespace casaWizard
