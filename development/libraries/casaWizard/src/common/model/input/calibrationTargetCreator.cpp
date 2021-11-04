//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "calibrationTargetCreator.h"

#include "extractWellDataLAS.h"
#include "extractWellDataXlsx.h"

#include "model/casaScenario.h"

#include <QString>

namespace casaWizard
{

CalibrationTargetCreator::CalibrationTargetCreator(CasaScenario& casaScenario,
                                                   CalibrationTargetManager& calibrationTargetManager,
                                                   ExtractWellData& extractWellData) :
  calibrationTargetManager_{calibrationTargetManager},
  casaScenario_{casaScenario},
  extractWellData_{extractWellData}
{
}

CalibrationTargetCreator::~CalibrationTargetCreator()
{
}

void CalibrationTargetCreator::createFromFile()
{
  AddWellDataToCalibrationTargetManager();
}

void CalibrationTargetCreator::readMetaDataFromFile()
{
  AddWellMetaDataToCalibrationTargetManager();
}

void CalibrationTargetCreator::AddWellDataToCalibrationTargetManager()
{
  extractWellData_.resetExtractor();
  while (extractWellData_.hasNextWell())
  {
    extractWellData_.extractDataNextWell();
    const int wellIndex = calibrationTargetManager_.addWell(extractWellData_.wellName(), extractWellData_.xCoord(), extractWellData_.yCoord());

    const QVector<QString> variableUserNames = extractWellData_.calibrationTargetVarsUserName();

    calibrationTargetManager_.setWellMetaData(wellIndex, extractWellData_.metaData());

    const QVector<unsigned int> nTargetsPerVariable = extractWellData_.nDataPerTargetVar();

    unsigned int nTotalTargets = 0;
    for (int iVariable = 0; iVariable < variableUserNames.size(); ++iVariable)
    {
      const QVector<double> z = extractWellData_.depth();
      const QVector<double> value = extractWellData_.calibrationTargetValues();
      const QVector<double> stdDeviations = extractWellData_.calibrationTargetStdDeviation();

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
        if (calibrationTargetManager_.getCauldronPropertyName(variableUserNames[iVariable]) == "TwoWayTime" && z[iTarget] == 0.0 && value[iTarget] == 0.0)
        {
          continue;
        }
        if (calibrationTargetManager_.getCauldronPropertyName(variableUserNames[iVariable]) == "Unknown")
        {
          continue;
        }

        const QString targetName(calibrationTargetManager_.getCauldronPropertyName(variableUserNames[iVariable]) + "(" +
                                 QString::number(extractWellData_.xCoord(),'f',1) + "," +
                                 QString::number(extractWellData_.yCoord(),'f',1) + "," +
                                 QString::number(z[iTarget],'f',1) + ")");
        calibrationTargetManager_.addCalibrationTarget(targetName, variableUserNames[iVariable],
                                                      wellIndex, z[iTarget], value[iTarget], stdDeviations[iTarget]);

      }
      nTotalTargets += nTargetsPerVariable[iVariable];
    }
  }

  calibrationTargetManager_.renamePropertiesAfterImport();
}

void CalibrationTargetCreator::AddWellMetaDataToCalibrationTargetManager()
{
  while (extractWellData_.hasNextWell())
  {
    extractWellData_.extractMetaDataNextWell();

    const QVector<QString> variableUserNames = extractWellData_.calibrationTargetVarsUserName();
    const QVector<QString> variableCauldronNames = extractWellData_.calibrationTargetVarsCauldronName();

    for (int i = 0; i < variableUserNames.size(); i++)
    {
      calibrationTargetManager_.addToMapping(variableUserNames[i], variableCauldronNames[i]);
      calibrationTargetManager_.addUnits(variableUserNames[i], extractWellData_.units()[i]);
    }
  }
}

} // namespace casaWizard
