//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "model/TargetInputInfoFromASCII.h"
#include "model/logger.h"
#include <QFileDialog>
#include <QTextStream>

namespace casaWizard
{
namespace ua
{
TargetInputInfoFromASCII::TargetInputInfoFromASCII(QString filePath,
                                                   QVector<bool> surfaceSelectionStates,
                                                   bool temperatureTargetsSelected,
                                                   bool vreTargetsSelected,
                                                   QString depthInput,
                                                   QString defaultName):
   TargetInputInfo(surfaceSelectionStates,
                   temperatureTargetsSelected,
                   vreTargetsSelected,
                   depthInput),
   m_filePath(filePath)
{
   if(defaultName != ""){
      m_defaultName = defaultName;
   } else {
      m_defaultName = "PSW";
   }
}


QVector<TargetInputInfo::XYName> TargetInputInfoFromASCII::getTargetLocations() const
{
   //parse
   //read file, parse, set in view
   QFile inputFile(m_filePath);
   QVector<TargetInputInfo::XYName> rowData;
   Logger::log() << "Importing prediction targets:" << Logger::endl();
   //parser (bit unsafe)
   if (inputFile.open(QIODevice::ReadOnly))
   {
      QTextStream in(&inputFile);
      int counter = 0;
      while (!in.atEnd())
      {
         counter ++;
         QString line = in.readLine();
         QStringList variables{line.split(",", QString::SplitBehavior::SkipEmptyParts)};

         if (variables.size() >= 2 && variables.size() <= 3){
            TargetInputInfo::XYName entry;

            entry.x = variables[0].toDouble();
            entry.y = variables[1].toDouble();
            entry.name = m_defaultName +"_" + QString::number(counter);

            if (variables.size() == 3 && variables[2] != ""){
               entry.name = variables[2];
            }
            rowData.push_back(entry);
         }
         else
         {
            Logger::log() << "Error on Line " + QString::number(counter - 1) + ": ";
            Logger::log() << "Incorrect line format, targets should be formatted as: x,y,name" << Logger::endl();
         }
      }
      inputFile.close();
   }
   return rowData;
}

QString TargetInputInfoFromASCII::getFilePath()
{
   return m_filePath;
}

QString TargetInputInfoFromASCII::getDefaultName()
{
   return m_defaultName;
}

void TargetInputInfoFromASCII::setFilePath(QString path)
{
   m_filePath = path;
}

void TargetInputInfoFromASCII::setDefaultName(QString name)
{
   m_defaultName = name;
}
} //namespace ua
} //namespace casaWizard
