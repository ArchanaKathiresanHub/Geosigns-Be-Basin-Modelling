#include "model/output/runCaseSetFileManager.h"

#include "model/scenarioReader.h"
#include "model/scenarioWriter.h"

#include "expectFileEq.h"

#include <QDir>

#include <memory>

#include <gtest/gtest.h>

const double epsilon = 1e-6;

class RunCaseSetFileManager : public ::testing::Test
{
protected:
  void SetUp() final
  {
    caseSetPath_ = QDir::currentPath() + "/CaseSet";
    dirCaseSetPath_.setPath(caseSetPath_);

    if (dirCaseSetPath_.exists())
    {
      dirCaseSetPath_.removeRecursively();
    }

    EXPECT_TRUE(dirCaseSetPath_.mkpath(caseSetPath_));

    sizeLastIterationDirFiles_ = createIterationDir(1);
    sizeAllDirsFiles_ = sizeLastIterationDirFiles_;

    proj3dPath_ = QDir::currentPath() + QString("/Project.project3d");
    rcFileManager_.setIterationPath(proj3dPath_);
  }

  void createFile(const QString& filePath)
  {
    casaWizard::ScenarioWriter writer(filePath);
    writer.writeValue("This is a dummy file!", 0);
    writer.close();
  }

  double createIterationDir(const int n)
  {
    const QString filePathIterBaseCase = caseSetPath_ + "/Iteration_" + QString::number(n) + "/BaseCase";
    const QString filePathIterCase_1 = caseSetPath_ + "/Iteration_" + QString::number(n) + "/Case_1";
    QDir dir;
    EXPECT_TRUE(dir.mkpath(filePathIterBaseCase));
    EXPECT_TRUE(dir.mkpath(filePathIterCase_1));

    QString pathFileBase = filePathIterBaseCase + "/dummy_0.dat";
    createFile(pathFileBase);
    QFileInfo fileBase{pathFileBase};

    QString pathFile1 = filePathIterCase_1 + "/dummy_1.dat";
    createFile(pathFile1);
    QFileInfo file1{pathFile1};

    return (file1.size() + fileBase.size())/1024.0/1024.0;
  }

  double sizeLastIterationDirFiles_;
  double sizeAllDirsFiles_;
  QString caseSetPath_;
  QDir dirCaseSetPath_;
  QString proj3dPath_;
  casaWizard::ua::RunCaseSetFileManager rcFileManager_;
};

TEST_F( RunCaseSetFileManager, testFileWriteRead )
{
  casaWizard::ScenarioWriter writer{"runCaseSetFileManager.dat"};
  rcFileManager_.writeToFile(writer);
  writer.close();

  casaWizard::ua::RunCaseSetFileManager rcFileManagerForReading;
  casaWizard::ScenarioReader reader{"runCaseSetFileManager.dat"};
  rcFileManagerForReading.readFromFile(reader);
  const QString& iterationDirPathExpected = rcFileManager_.iterationDirPath();
  const QString& iterationDirPathActual = rcFileManagerForReading.iterationDirPath();
  EXPECT_EQ(iterationDirPathExpected, iterationDirPathActual);
  const QDateTime& timeOfFileCreationExpected = rcFileManager_.iterationDirDateTime();
  const QDateTime& timeOfFileCreationActual = rcFileManagerForReading.iterationDirDateTime();
  EXPECT_EQ(timeOfFileCreationExpected, timeOfFileCreationActual);
}

TEST_F( RunCaseSetFileManager, testIterationDirSize )
{
  // size of the files in the last created directory, i.e. Iteration_2
  const double dirFilesSizeExpected = sizeLastIterationDirFiles_;
  const double dirFilesSizeActual = rcFileManager_.iterationDirFilesSize();
  EXPECT_NEAR(dirFilesSizeExpected, dirFilesSizeActual, epsilon);

  const double dirSizeExpected = createIterationDir(2);
  rcFileManager_.setIterationPath(proj3dPath_);
  const double dirSizeActual = rcFileManager_.iterationDirFilesSize();
  EXPECT_NEAR(dirSizeExpected, dirSizeActual, epsilon);
  sizeAllDirsFiles_ += dirSizeExpected;

  // Size of all the files for all directories
  const double allDirsFilesSizeExpected = sizeAllDirsFiles_;
  const double allDirsFilesSizeSizeActual = rcFileManager_.allIterationDirsFilesSize(proj3dPath_);
  EXPECT_NEAR(allDirsFilesSizeExpected, allDirsFilesSizeSizeActual, epsilon);
}

TEST_F( RunCaseSetFileManager, testIsIterationDirEmpty )
{
  EXPECT_FALSE(rcFileManager_.isIterationDirEmpty());
}

TEST_F( RunCaseSetFileManager, testIterationDirName )
{
  const QString dirNameExpected = "Iteration_1";
  const QString& dirNameActual = rcFileManager_.iterationDirName();
  EXPECT_EQ(dirNameExpected, dirNameActual);
}

TEST_F( RunCaseSetFileManager, testIsIterationDirDeleted )
{
  EXPECT_FALSE(rcFileManager_.isIterationDirDeleted(proj3dPath_));
}

TEST_F( RunCaseSetFileManager, testRemoveIterationDir )
{
  EXPECT_TRUE(rcFileManager_.removeIterationDir());
  EXPECT_TRUE(rcFileManager_.isIterationDirDeleted(proj3dPath_));
}

TEST_F( RunCaseSetFileManager, testRemoveAllIterationDirs )
{
  const double dirSizeExpected = createIterationDir(2);
  rcFileManager_.setIterationPath(proj3dPath_);
  const double dirSizeActual = rcFileManager_.iterationDirFilesSize();
  EXPECT_NEAR(dirSizeExpected, dirSizeActual, epsilon);
  EXPECT_TRUE(rcFileManager_.removeAllIterationDirs(proj3dPath_));
  EXPECT_NEAR(rcFileManager_.allIterationDirsFilesSize(proj3dPath_), 0.0, epsilon);
}
