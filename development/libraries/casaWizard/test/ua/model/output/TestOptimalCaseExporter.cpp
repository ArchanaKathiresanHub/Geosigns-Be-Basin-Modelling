//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "model/output/OptimalCaseExporter.h"
#include "model/functions/folderOperations.h"

#include <QDir>
#include <QProcess>
#include <QTextStream>

#include <gtest/gtest.h>

TEST(OptimalCaseExporter, testExportCase )
{
   using namespace casaWizard;
   QDir tmpTestDir(QDir::currentPath() + "/tmpTestDir");
   tmpTestDir.removeRecursively();
   tmpTestDir.mkdir(QDir::currentPath() + "/tmpTestDir");

   QFile::copy(QDir::currentPath() + "/Project.project3d", tmpTestDir.path() + "/Project.project3d");

   ua::optimalCaseExporter::exportOptimalCase(tmpTestDir.path(),QDir::currentPath());

   //Unzip to check the content of the zipped folder:
   QProcess process;
   process.setWorkingDirectory(QDir::currentPath());
   functions::processCommand(process, QString("unzip -o " + tmpTestDir.path() + "/optimal.zip -d " + tmpTestDir.path() + "/zippedFolderConents"));

   EXPECT_TRUE(QFile::exists(tmpTestDir.path() + "/zippedFolderConents/Project.project3d"));
   EXPECT_TRUE(QFile::exists(tmpTestDir.path() + "/zippedFolderConents/Project.txt"));
   EXPECT_TRUE(QFile::exists(tmpTestDir.path() + "/zippedFolderConents/Inputs.HDF"));

   //cleanup
   tmpTestDir.removeRecursively();
}

TEST(OptimalCaseExporter, testInvalidExport )
{
   using namespace casaWizard;
   QDir tmpTestDir(QDir::currentPath() + "/tmpTestDir");
   tmpTestDir.removeRecursively();
   tmpTestDir.mkdir(QDir::currentPath() + "/tmpTestDir");

   //optimal .zip should not be created if Project.PROJECT3D is missing:
   ua::optimalCaseExporter::exportOptimalCase(tmpTestDir.path(),QDir::currentPath());
   EXPECT_FALSE(QFile::exists(tmpTestDir.path() + "/optimal.zip"));

   //optimal .zip should not be created if Inputs.HDF is missing:
   QFile::copy(QDir::currentPath() + "/Project.project3d", tmpTestDir.path() + "/Project.project3d");
   QFile::copy(QDir::currentPath() + "/Project.txt", tmpTestDir.path() + "/Project.txt");
   ua::optimalCaseExporter::exportOptimalCase(tmpTestDir.path(),tmpTestDir.path());
   EXPECT_FALSE(QFile::exists(tmpTestDir.path() + "/optimal.zip"));

   //optimal .zip should not be created if Project.txt is missing:
   QFile::remove(tmpTestDir.path() + "/Project.txt");
   QFile::copy(QDir::currentPath() + "/Inputs.HDF", tmpTestDir.path() + "/Inputs.HDF");
   ua::optimalCaseExporter::exportOptimalCase(tmpTestDir.path(),tmpTestDir.path());
   EXPECT_FALSE(QFile::exists(tmpTestDir.path() + "/optimal.zip"));

   //cleanup
   tmpTestDir.removeRecursively();
}

TEST(OptimalCaseExporter, testScenarioName )
{
   using namespace casaWizard;
   QDir tmpTestDir(QDir::currentPath() + "/casaWorkspace-NoTimeStamp");
   QDir tmpOptimalDir(QDir::currentPath() + "/casaWorkspace-NoTimeStamp/optimal");
   tmpTestDir.removeRecursively();
   tmpTestDir.mkdir(QDir::currentPath() + "/casaWorkspace-NoTimeStamp");
   tmpOptimalDir.mkdir(QDir::currentPath() + "/casaWorkspace-NoTimeStamp/optimal");

   QFile::copy(QDir::currentPath() + "/Project.project3d", tmpOptimalDir.path() + "/Project.project3d");
   QFile::copy(QDir::currentPath() + "/Project.txt", tmpTestDir.path() + "/Project.txt");
   QFile::copy(QDir::currentPath() + "/Inputs.HDF", tmpTestDir.path() + "/Inputs.HDF");

   ua::optimalCaseExporter::exportOptimalCase(tmpOptimalDir.path() , tmpTestDir.path());

   QProcess process;
   process.setWorkingDirectory(tmpTestDir.path());
   functions::processCommand(process, QString("unzip -o " + tmpOptimalDir.path() + "/optimal.zip -d " + tmpTestDir.path() + "/zippedFolderContents"));

   QFile project(tmpTestDir.path() + "/zippedFolderContents/Project.txt");
   QString text;
   bool readNameSuccefully = false;
   if(project.open(QIODevice::Text | QIODevice::ReadOnly))
   {
      QTextStream in(&project);
      while(!in.atEnd()) {
         QString line = in.readLine();
         QStringList fields = line.split(":");
         if(fields.size() >= 2 && fields.first() == "Scenario")
         {
            readNameSuccefully = fields.last().simplified() == "Test_UA_NoTimeStamp";
            break;
         }
      }
   }
   EXPECT_TRUE(readNameSuccefully);
   tmpTestDir.removeRecursively();
}
