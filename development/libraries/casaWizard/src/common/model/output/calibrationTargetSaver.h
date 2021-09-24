//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// Function to save the calibration targets of well prep to an Excel file
#pragma once

class QString;

namespace casaWizard
{

class CalibrationTargetManager;

class CalibrationTargetSaver
{
public:
  CalibrationTargetSaver(const CalibrationTargetManager& calibrationTargetManager);
  void saveToExcel(const QString& excelFilename);

private:  

  const CalibrationTargetManager& calibrationTargetManager_;
};

} // namespace casaWizard
