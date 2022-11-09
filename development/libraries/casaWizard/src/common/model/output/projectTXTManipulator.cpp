//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "projectTXTManipulator.h"

#include <QFile>
#include <QTextStream>

ProjectTXTManipulator::ProjectTXTManipulator(const QString& projectTextFile) :
  projectTextFile_{projectTextFile}
{
}

void ProjectTXTManipulator::appendStampToScenarioName(const QString& stamp) const
{
  const QString tempFileName = "tempFile.txt";
  QFile textFile(projectTextFile_);
  if (!textFile.open(QIODevice::ReadOnly | QIODevice::Text))
  {
     throw FileDoesNotExistException("File " + tempFileName.toStdString() + " could not be created");
  }

  QFile tmpFile(tempFileName);
  if (!tmpFile.open(QIODevice::WriteOnly | QIODevice::Text))
  {
    return;
  }  

  QTextStream in(&textFile);
  QTextStream out(&tmpFile);
  while (!in.atEnd())
  {
    QString line = in.readLine();
    if (line.left(9) == QString("Scenario:") || line.left(8) == QString("Project:"))
    {
      line += "_" + stamp;
    }

    out << line << Qt::endl;
  }

  textFile.close();
  tmpFile.close();
  textFile.remove();
  tmpFile.rename(projectTextFile_);
}
