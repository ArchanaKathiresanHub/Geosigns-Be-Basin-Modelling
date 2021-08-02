// Function to populate the calibration targets of casa scenario from an Excel file
#pragma once

class QString;

namespace casaWizard
{

class CasaScenario;
class CMBMapReader;

class CalibrationTargetCreator
{
public:
  CalibrationTargetCreator(CasaScenario& casaScenario);
  void createFromExcel(const QString& excelFilename);

private:
  void setWellHasDataInLayer(const int wellIndex, const CMBMapReader& mapReader);

  CasaScenario& casaScenario_;
};

} // namespace casaWizard
