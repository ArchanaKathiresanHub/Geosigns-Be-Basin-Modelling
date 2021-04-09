//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include <gtest/gtest.h>

#include "model/output/projectTXTManipulator.h"
#include "expectFileEq.h"

#include <QFile>


TEST(ProjectTXTManipulator, testAppendTimeStamp)
{
  const QString tempProject = "TempProject.txt";
  if (QFile::exists(tempProject)) QFile::remove(tempProject);
  QFile::copy(QString("Project.txt"), tempProject);

  // Given
  ProjectTXTManipulator manipulator(tempProject);

  // When
  manipulator.appendStampToScenarioName("SAC_SomeDate-SomeTime");

  // Then
  expectFileEq(tempProject.toStdString(), "ProjectExpected.txt");
}

TEST(ProjectTXTManipulator, testAppendingStampThrowsIfFileDoesNotExist)
{
  // Given
  ProjectTXTManipulator manipulator("DoesNotExist.txt");

  // Then
  EXPECT_THROW(manipulator.appendStampToScenarioName("SAC_SomeDate-SomeTime"), FileDoesNotExistException);
}

TEST(ProjectTXTManipulator, testAppendingStampDoesNotThrowIfFileExists)
{
  const QString tempProject = "TempProject2.txt";
  if (QFile::exists(tempProject)) QFile::remove(tempProject);
  QFile::copy(QString("Project.txt"), tempProject);

  // Given
  ProjectTXTManipulator manipulator(tempProject);

  // Then
  EXPECT_NO_THROW(manipulator.appendStampToScenarioName("SAC_SomeDate-SomeTime"));
}

