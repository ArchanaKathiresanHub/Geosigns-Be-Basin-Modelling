// Function to populate the calibration targets of casa scenario from an Excel file
#pragma once

class QString;

namespace casaWizard
{

class CasaScenario;

namespace calibrationTargetCreator
{

void createFromExcel(CasaScenario& casaScenario, const QString& excelFilename);

} // namespace calibrationTargetCreator

} // namespace casaWizard
