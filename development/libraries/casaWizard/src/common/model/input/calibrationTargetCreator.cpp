#include "calibrationTargetCreator.h"

#include "extractWellDataXlsx.h"
#include "model/casaScenario.h"
#include "model/input/cmbMapReader.h"

#include <QString>

namespace casaWizard
{

CalibrationTargetCreator::CalibrationTargetCreator(CasaScenario& casaScenario, CalibrationTargetManager& calibrationTargetManager) :
  calibrationTargetManager_{calibrationTargetManager},
  casaScenario_{casaScenario}
{
}

void CalibrationTargetCreator::createFromExcel(const QString& excelFileName)
{
  ExtractWellDataXlsx wellData{excelFileName};

  CMBMapReader mapReader;
  mapReader.load(casaScenario_.project3dPath().toStdString());

  const QVector<QString> wellNames = wellData.wellNames();
  for (const QString& wellName : wellNames)
  {
    wellData.extractWellData(wellName);
    const int wellIndex = calibrationTargetManager_.addWell(wellName, wellData.xCoord(), wellData.yCoord());

    const QVector<QString> variableUserNames = wellData.calibrationTargetVarsUserName();
    const QVector<QString> variableCauldronNames = wellData.calibrationTargetVarsCauldronName();

    for (int i = 0; i < variableUserNames.size(); i++)
    {
      calibrationTargetManager_.addToMapping(variableUserNames[i], variableCauldronNames[i]);
    }

    calibrationTargetManager_.setWellMetaData(wellIndex, wellData.metaData());

    const QVector<unsigned int> nTargetsPerVariable = wellData.nDataPerTargetVar();

    unsigned int nTotalTargets = 0;
    for (int iVariable = 0; iVariable < variableUserNames.size(); ++iVariable)
    {
      const QVector<double> z = wellData.depth();
      const QVector<double> value = wellData.calibrationTargetValues();
      const QVector<double> stdDeviations = wellData.calibrationTargetStdDeviation();

      const int nZ = z.size();
      const int nValue = value.size();
      const int nStdDev = stdDeviations.size();

      if (nZ != nValue || nZ != nStdDev)
      {
        continue; // inconsistent array lengths, skip this variable
      }

      // In wellData, the targets for different variables in the same well are stored in a contiguous array
      for (unsigned int iTarget = nTotalTargets; iTarget < nTotalTargets + nTargetsPerVariable[iVariable]; ++iTarget)
      {
        // If two way time is defined at the sea surface, this value should be skipped, since the domain starts
        // at the mudline.
        if (variableCauldronNames[iVariable] == "TwoWayTime" && z[iTarget] == 0.0 && value[iTarget] == 0.0)
        {
          continue;
        }
        const QString targetName(variableCauldronNames[iVariable] + "(" +
                                 QString::number(wellData.xCoord(),'f',1) + "," +
                                 QString::number(wellData.yCoord(),'f',1) + "," +
                                 QString::number(z[iTarget],'f',1) + ")");
        calibrationTargetManager_.addCalibrationTarget(targetName, variableUserNames[iVariable],
                                                      wellIndex, z[iTarget], value[iTarget], stdDeviations[iTarget]);

      }
      nTotalTargets += nTargetsPerVariable[iVariable];
    }
    setWellHasDataInLayer(wellIndex, mapReader);
  }  
}

void CalibrationTargetCreator::setWellHasDataInLayer(const int wellIndex, const CMBMapReader& mapReader)
{
  QVector<bool> hasDataInLayer;
  auto& well = calibrationTargetManager_.well(wellIndex);
  for (const QString& layer : casaScenario_.projectReader().layerNames())
  {
    bool hasDataInCurrentLayer = false;
    for (const CalibrationTarget* target : well.calibrationTargets())
    {
      if (mapReader.checkIfPointIsInLayer(well.x(), well.y(), target->z(), layer.toStdString()))
      {
        hasDataInCurrentLayer = true;
        break;
      }
    }
    hasDataInLayer.push_back(hasDataInCurrentLayer);
  }

  calibrationTargetManager_.setHasDataInLayer(wellIndex, hasDataInLayer);
}

} // namespace casaWizard
