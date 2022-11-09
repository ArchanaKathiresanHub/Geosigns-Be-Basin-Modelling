//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "McmcOutputWriter.h"
#include "McmcTargetExportData.h"

#include "model/logger.h"
#include "model/targetParameterMapCreator.h"

#include "Qt_Utils.h"

#include <QFile>

namespace casaWizard
{

namespace ua
{

McmcOutputWriter::McmcOutputWriter(QString fileName, const McmcTargetExportData& exportData):
   m_fileName(fileName),
   m_exportData(exportData)
{}

bool McmcOutputWriter::writeToFile(QString fileName, const McmcTargetExportData& exportData)
{
   if (exportData.targetData.isEmpty() || exportData.allPropNames.size() == 0) return false;

   McmcOutputWriter writer(fileName,exportData);
   return writer.writeToFilePrivate();
}

bool McmcOutputWriter::writeToFilePrivate()
{
   if (!writeHeader()) return false;
   if (!writeData()) return false;

   QFile file(m_fileName);
   if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
   {
      Logger::log() << "Can not open file " << m_fileName << " for writing." << Logger::endl();
      return false;
   }

   Logger::log() << "Writing mcmc data to " << m_fileName << Logger::endl();

   file.write(m_writeData);
   file.close();
   return true;
}

bool McmcOutputWriter::writeData()
{
   const QMap<QString,McmcSingleTargetExportData>& data = m_exportData.targetData;

   foreach (const McmcSingleTargetExportData& value, data)
   {
      if (!writeTarget(value))
      {
         return false;
      }
   }

   return true;
}

bool McmcOutputWriter::writeTarget(const McmcSingleTargetExportData& d)
{
   m_writeData += (qtutils::escapeSpecialCharacters(d.locationName) + "; "
         + QString::number(d.xCor)  + "; "
         + QString::number(d.yCor)  + "; "
         + QString::number(d.zCor)  + "; "
         + d.stratigraphicSurface + "; "
         + QString::number(d.age)  + "; ").toUtf8();

   const QMap<QString,TargetDataSingleProperty>& targetOutputs = d.targetOutputs;

   const std::set<QString>& allPropNames = m_exportData.allPropNames;

   bool targetPropDataWritten(false);
   for (const QString& propName : allPropNames)
   {
      if (targetOutputs.contains(propName))
      {
         targetPropDataWritten = true;
         const TargetDataSingleProperty& targetDataSingleProp = targetOutputs[propName];
         m_writeData += (QString::number(targetDataSingleProp.p10) + "; "
               + QString::number(targetDataSingleProp.p50)  + "; "
               + QString::number(targetDataSingleProp.p90)  + "; "
               + QString::number(targetDataSingleProp.baseSim)  + "; "
               + QString::number(targetDataSingleProp.baseProxy) + "; "
               + QString::number(targetDataSingleProp.optimalSim)  + "; "
               + QString::number(targetDataSingleProp.optimalProxy)  + "; ").toUtf8();
      }
      else
      {
         m_writeData += "nan; nan; nan; nan; nan; nan; nan; ";
      }
   }

   m_writeData += "\n";

   return targetPropDataWritten;
}

bool McmcOutputWriter::writeHeader()
{
   const std::set<QString>& allPropNames = m_exportData.allPropNames;
   if (allPropNames.size() == 0) return false;

   m_writeData += "Location Name; "
                  "X [m]; "
                  "Y [m]; "
                  "Z [m]; "
                  "Stratigraphic surface; "
                  "Age [Ma]; ";

   for (const QString& pn : allPropNames)
   {
      const QString propName = qtutils::escapeSpecialCharacters(pn);
      const QString propUnit = targetParameterMapCreator::lookupSIUnit(propName);

      m_writeData += (propName + " P10 " + propUnit + "; "
            + propName + " P50 " + propUnit + "; "
            + propName + " P90 " + propUnit + "; "
            + propName + " Base sim " + propUnit + "; "
            + propName + " Base proxy " + propUnit + "; "
            + propName + " Optimal sim " + propUnit + "; "
            + propName + " Optimal proxy " + propUnit + "; ").toUtf8();
   }
   m_writeData += "\n";

   return true;
}

} // namespace ua

} // namespace casaWizard
