//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include <memory>

#include "importOptions.h"

class QString;


namespace casaWizard
{

class CasaScenario;
class CalibrationTargetManager;
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

private:
  void AddWellDataToCalibrationTargetManager();
  void AddWellMetaDataToCalibrationTargetManager();

  CalibrationTargetManager& calibrationTargetManager_;
  CasaScenario& casaScenario_;
  ExtractWellData& extractWellData_;
};

} // namespace casaWizard
