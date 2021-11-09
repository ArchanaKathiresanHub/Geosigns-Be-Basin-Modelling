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
#include "extractWellDataVSET.h"

#include "model/casaScenario.h"

#include <QString>

namespace casaWizard
{

CalibrationTargetCreator::CalibrationTargetCreator(CasaScenario& casaScenario,
                                                   CalibrationTargetManager& calibrationTargetManager,
                                                   ExtractWellData& extractWellData) :
  importCalibrationTargetManager_{},
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
  addWellDataToCalibrationTargetManager();

  calibrationTargetManager_.appendFrom(importCalibrationTargetManager_);
}

void CalibrationTargetCreator::readMetaDataFromFile()
{
  addWellMetaDataToCalibrationTargetManager();
}

void CalibrationTargetCreator::addWellDataToCalibrationTargetManager()
{
  extractWellData_.resetExtractor();

  while (extractWellData_.hasNextWell())
  {
    extractWellData_.extractDataNextWell();
    if (extractWellData_.wellName().isEmpty())
    {
      continue;
    }
    const int wellIndex = importCalibrationTargetManager_.addWell(extractWellData_.wellName(), extractWellData_.xCoord(), extractWellData_.yCoord());

    const QVector<QString> variableUserNames = extractWellData_.calibrationTargetVarsUserName();

    importCalibrationTargetManager_.setWellMetaData(wellIndex, extractWellData_.metaData());

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
        if (importCalibrationTargetManager_.getCauldronPropertyName(variableUserNames[iVariable]) == "TwoWayTime" && z[iTarget] == 0.0 && value[iTarget] == 0.0)
        {
          continue;
        }
        if (importCalibrationTargetManager_.getCauldronPropertyName(variableUserNames[iVariable]) == "Unknown")
        {
          continue;
        }

        const QString targetName(importCalibrationTargetManager_.getCauldronPropertyName(variableUserNames[iVariable]) + "(" +
                                 QString::number(extractWellData_.xCoord(),'f',1) + "," +
                                 QString::number(extractWellData_.yCoord(),'f',1) + "," +
                                 QString::number(z[iTarget],'f',1) + ")");
        importCalibrationTargetManager_.addCalibrationTarget(targetName, variableUserNames[iVariable],
                                                      wellIndex, z[iTarget], value[iTarget], stdDeviations[iTarget]);

      }
      nTotalTargets += nTargetsPerVariable[iVariable];
    }
  }

  importCalibrationTargetManager_.renamePropertiesAfterImport();
}

void CalibrationTargetCreator::addWellMetaDataToCalibrationTargetManager()
{
  extractWellData_.resetExtractor();
  while (extractWellData_.hasNextWell())
  {
    extractWellData_.extractMetaDataNextWell();

    const QVector<QString> variableUserNames = extractWellData_.calibrationTargetVarsUserName();
    const QVector<QString> variableCauldronNames = extractWellData_.calibrationTargetVarsCauldronName();

    for (int i = 0; i < variableUserNames.size(); i++)
    {
      importCalibrationTargetManager_.addToMapping(variableUserNames[i], variableCauldronNames[i]);
      importCalibrationTargetManager_.addUnits(variableUserNames[i], extractWellData_.units()[i]);
    }
  }
}

void CalibrationTargetCreator::addNewMapping(const QMap<QString, QString> newMapping)
{
  for (const QString& key : newMapping.keys())
  {
    if (newMapping[key] != "Depth")
    {
      importCalibrationTargetManager_.addToMapping(key, newMapping[key]);
    }
  }
}

void CalibrationTargetCreator::getNamesAndUnits(const QString& depthUserPropertyName, QStringList& propertyUserNames, QStringList& defaultCauldronNames, QStringList& units)
{
  propertyUserNames = importCalibrationTargetManager_.userNameToCauldronNameMapping().keys();

  for (const auto& propertyName : propertyUserNames)
  {
    if (propertyName == depthUserPropertyName)
    {
      defaultCauldronNames.push_back("Depth");
    }
    else if (defaultCauldronNames.contains(importCalibrationTargetManager_.getCauldronPropertyName(propertyName)))
    {
      defaultCauldronNames.push_back("Unknown");
    }
    else
    {
      defaultCauldronNames.push_back(importCalibrationTargetManager_.getCauldronPropertyName(propertyName));
    }

    units.push_back(importCalibrationTargetManager_.getUnit(propertyName));
  }
}

} // namespace casaWizard
