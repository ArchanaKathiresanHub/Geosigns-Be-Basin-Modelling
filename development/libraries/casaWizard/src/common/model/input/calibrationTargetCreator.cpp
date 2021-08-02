#include "calibrationTargetCreator.h"

#include "extractWellDataXlsx.h"
#include "model/casaScenario.h"
#include "../common/model/input/cmbMapReader.h"

#include <QString>

namespace casaWizard
{


CalibrationTargetCreator::CalibrationTargetCreator(CasaScenario& casaScenario) :
  casaScenario_{casaScenario}
{

}

void CalibrationTargetCreator::createFromExcel(const QString& excelFileName)
{
  ExtractWellDataXlsx wellData{excelFileName};

  CalibrationTargetManager& calibrationTargetManager = casaScenario_.calibrationTargetManager();
  CMBMapReader mapReader;
  mapReader.load(casaScenario_.project3dPath().toStdString());

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
        calibrationTargetManager.addCalibrationTarget(targetName, variableUserNames[iVariable],
                                                      wellIndex, z[iTarget], value[iTarget]);

      }
      nTotalTargets += nTargetsPerVariable[iVariable];
    }
    setWellHasDataInLayer(wellIndex, mapReader);
  }

  calibrationTargetManager.updateObjectiveFunctionFromTargets();
}

void CalibrationTargetCreator::setWellHasDataInLayer(const int wellIndex, const CMBMapReader& mapReader)
{
  CalibrationTargetManager& calibrationTargetManager = casaScenario_.calibrationTargetManager();

  QVector<bool> hasDataInLayer;
  auto& well = calibrationTargetManager.well(wellIndex);
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

  calibrationTargetManager.setHasDataInLayer(wellIndex, hasDataInLayer);
}

} // namespace casaWizard
