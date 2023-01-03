//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "calibrationTargetSaver.h"
#include "model/calibrationTargetManager.h"
#include "model/logger.h"
#include "model/wellData.h"

#include "saveWellDataXlsx.h"

#include <QFile>
#include <QMap>
#include <QString>

namespace casaWizard
{

CalibrationTargetSaver::CalibrationTargetSaver(const CalibrationTargetManager& calibrationTargetManager, const QString& targetPath) :
   calibrationTargetManager_{calibrationTargetManager},
   m_targetPath(targetPath + "/")
{
}

void CalibrationTargetSaver::saveToExcel(const QString& excelFileName) const
{  
   QFile file(excelFileName);
   if (file.open(QFile::ReadWrite))
   {
      file.close();
      file.remove();
   }
   else
   {
      Logger::log() << "Unable to open file " << excelFileName << ". It might be opened by another program." << Logger::endl();
      return;
   }

   SaveWellDataXlsx saveWellDataXlsx(excelFileName);

   for (const Well* well : calibrationTargetManager_.activeWells())
   {
      QVector<double> depth;
      QVector<QString> calibrationTargetUserNames;
      QVector<unsigned int> nDataPerTarget;
      QVector<double> calibrationTargetValues;
      QVector<double> calibrationTargetStdDeviation;

      QMap<QString, QVector<const CalibrationTarget*>> calibrationTargetsByProperty;
      for (const CalibrationTarget* target : well->calibrationTargets())
      {
         calibrationTargetsByProperty[target->propertyUserName()].push_back(target);
      }

      for (const QString& propertyUserName : calibrationTargetsByProperty.keys())
      {
         const QVector<const CalibrationTarget*>& targets = calibrationTargetsByProperty[propertyUserName];
         calibrationTargetUserNames.push_back(propertyUserName);
         nDataPerTarget.push_back(targets.size());
         for ( const CalibrationTarget* target : targets )
         {
            depth.push_back(target->z());
            calibrationTargetValues.push_back(target->value());
            calibrationTargetStdDeviation.push_back(target->standardDeviation());
         }
      }

      WellData wellData(well->x(), well->y(), depth, calibrationTargetsByProperty.size(), calibrationTargetUserNames, nDataPerTarget, calibrationTargetValues, calibrationTargetStdDeviation, well->metaData());
      saveWellDataXlsx.saveWellData( well->name(), wellData);
   }

   if (saveWellDataXlsx.save())
   {
      Logger::log() << "Successfully saved data to " << excelFileName << Logger::endl();
   }
   else
   {
      Logger::log() << "Failed to save data to " << excelFileName << Logger::endl();
   }
}

bool CalibrationTargetSaver::saveRawLocationsToCSV(const QString& fileName, const char& separator, const bool onlyIncluded) const
{
   return saveRawLocationsToFile( fileName.endsWith(".csv") ? fileName : fileName + ".csv", separator, onlyIncluded );
}

bool CalibrationTargetSaver::saveRawLocationsToText(const QString& fileName, const char& separator, const bool onlyIncluded) const
{
   return saveRawLocationsToFile( fileName.endsWith(".txt") ? fileName : fileName + ".txt", separator, onlyIncluded );
}

bool CalibrationTargetSaver::saveRawLocationsToFile(const QString& fileName, const char& separator, const bool onlyIncluded) const
{
   QFile file(m_targetPath + fileName);
   QFileInfo fi(file);
   Logger::log() << m_targetPath << Logger::endl();
   if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
   {
      Logger::log() << "Can not open file " << fi.absoluteFilePath() << " for writing." << Logger::endl();
      return false;
   }

   Logger::log() << "Writing X-Y data to " << fi.absoluteFilePath() << Logger::endl();
   QByteArray data;
   for ( const Well* well : (onlyIncluded ? calibrationTargetManager_.activeAndIncludedWells() : calibrationTargetManager_.activeWells()) )
   {
      data += QString::number(well->x()) + separator + QString::number(well->y()) + separator + well->name() + "\n";
   }
   file.write(data);
   file.close();
   return true;
}

} // namespace casaWizard
