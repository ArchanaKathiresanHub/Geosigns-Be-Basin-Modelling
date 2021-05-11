#include "calibrationTargetCreator.h"

#include "extractWellDataXlsx.h"
#include "model/casaScenario.h"

#include <QString>

namespace casaWizard
{

namespace calibrationTargetCreator
{

void createFromExcel(CasaScenario& casaScenario, const QString& excelFilename)
{
  ExtractWellDataXlsx wellData{excelFilename};

  CalibrationTargetManager& calibrationTargetManager = casaScenario.calibrationTargetManager();

  const QVector<QString> wellNames = wellData.wellNames();
  for (const QString& wellName : wellNames)
  {
    wellData.extractWellData(wellName);
    const int wellIndex = calibrationTargetManager.addWell(wellName, wellData.xCoord(), wellData.yCoord());

    const QVector<QString> variableUserNames = wellData.calibrationTargetVarsUserName();
    const QVector<QString> variableCauldronNames = wellData.calibrationTargetVarsCauldronName();

    for (int i = 0; i < variableUserNames.size(); i++)
    {
      calibrationTargetManager.addToMapping(variableUserNames[i], variableCauldronNames[i]);
    }

    const QVector<std::size_t> nTargetsPerVariable = wellData.nDataPerTargetVar();

    unsigned int nTotalTargets = 0;
    for (int iVariable = 0; iVariable < variableUserNames.size(); ++iVariable)
    {
      const QVector<double> z = wellData.depth();
      const QVector<double> value = wellData.calibrationTargetValues();

      const int nZ = z.size();
      const int nValue = value.size();

      if ( nZ != nValue )
      {
        continue; // inconsistent array lengths, skip this variable
      }

      // In wellData, the targets for different variables in the same well are stored in a contiguous array
      for (unsigned int iTarget = nTotalTargets; iTarget < nTotalTargets + nTargetsPerVariable[iVariable]; ++iTarget)
      {
        const QString targetName(variableCauldronNames[iVariable] + "(" +
                                 QString::number(wellData.xCoord(),'f',1) + "," +
                                 QString::number(wellData.yCoord(),'f',1) + "," +
                                 QString::number(z[iTarget],'f',1) + ")");
        calibrationTargetManager.addCalibrationTarget(targetName, variableUserNames[iVariable],
                                                      wellIndex, z[iTarget], value[iTarget]);
      }
      nTotalTargets += nTargetsPerVariable[iVariable];
    }
  }

  calibrationTargetManager.updateObjectiveFunctionFromTargets();
}

} // namespace calibrationTargetCreator

} // namespace casaWizard
