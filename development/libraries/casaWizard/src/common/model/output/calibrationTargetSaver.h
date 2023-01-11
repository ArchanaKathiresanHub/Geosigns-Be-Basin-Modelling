//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// Function to save the calibration targets of well prep to an Excel file
#pragma once

#include <QDir>

class QString;

namespace casaWizard
{

class CalibrationTargetManager;

class CalibrationTargetSaver
{
public:
  CalibrationTargetSaver(const CalibrationTargetManager& calibrationTargetManager, const QString& targetPath  = "");

  void saveToExcel(const QString& excelFilename) const;
  bool saveRawLocationsToCSV(const QString& filename, const char& seperator = ',', const bool included = false) const;
  bool saveRawLocationsToText(const QString& filename, const char& seperator = ' ', const bool included = false) const;

private:  
  const CalibrationTargetManager& calibrationTargetManager_;
  const QString m_targetPath;
  bool saveRawLocationsToFile(const QString& filename, const char& seperator, const bool included = false) const;
};

} // namespace casaWizard
