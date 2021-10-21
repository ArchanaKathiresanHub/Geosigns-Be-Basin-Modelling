// Function to populate the calibration targets of casa scenario from an Excel file
#pragma once

class QString;

namespace casaWizard
{

class CasaScenario;
class CalibrationTargetManager;

class CalibrationTargetCreator
{
public:
  CalibrationTargetCreator(CasaScenario& casaScenario, casaWizard::CalibrationTargetManager& calibrationTargetManager);
  void createFromExcel(const QString& excelFilename);

private:  

  CalibrationTargetManager& calibrationTargetManager_;
  CasaScenario& casaScenario_;
};

} // namespace casaWizard
