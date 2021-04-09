//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "model/output/workspaceGenerator.h"

#include "expectFileEq.h"

#include <QDir>

#include <gtest/gtest.h>

class WorkspaceGenerator : public ::testing::Test
{

protected:

  bool checkFiles(const QString& src, const QString& dest)
  {
    bool found = false;

    const QDir dirOrigin(src);
    const QDir dirDest(dest);

    if (!dirOrigin.isReadable() ||
        !dirDest.isReadable())
    {
      return false;
    }

    QFileInfoList entriesOrigin = dirOrigin.entryInfoList();
    QFileInfoList entriesDest   = dirDest.entryInfoList();

    for (QFileInfo&  finfoOrigin : entriesOrigin)
    {
      if (finfoOrigin.fileName()=="."  ||
          finfoOrigin.fileName()==".." ||
          finfoOrigin.isSymLink()      ||
          finfoOrigin.fileName().contains(".casa") ||
          finfoOrigin.fileName().contains(".log") ||
          finfoOrigin.isDir())
      {
        continue;
      }

      if (!finfoOrigin.isReadable())
      {
        return false;
      }

      found = false;

      for (QFileInfo&  finfoDest : entriesDest)
      {
        if (!finfoDest.isReadable())
        {
          return false;
        }

        if(finfoOrigin.fileName()==finfoDest.fileName())
        {
          found = true;
          break;
        }
      }
      if(!found)
      {
        return false;
      }
    }
    return true;
  }

  bool deleteNewWorkspace(QDir& dir)
  {
    if (dir.exists())
    {
      EXPECT_TRUE (dir.removeRecursively());
    }
    if (dir.exists())
    {
      return false;
    }
    return true;
  }
};

TEST_F( WorkspaceGenerator, testWorkspaceGenerator )
{
  QDir oldWorkspaceLocation = QDir(QDir::current().absolutePath() + "/SAC");

  QString path = oldWorkspaceLocation.absolutePath() + "/NewWorkspaceFolder";
  QDir newWorkspaceLocation = QDir(path);
  if (newWorkspaceLocation.exists()) QDir().remove(path);

  EXPECT_TRUE (casaWizard::workspaceGenerator::createWorkspace(oldWorkspaceLocation.absolutePath(), newWorkspaceLocation.absolutePath()));
  EXPECT_TRUE (checkFiles(oldWorkspaceLocation.absolutePath() ,newWorkspaceLocation.absolutePath()));
  EXPECT_TRUE (deleteNewWorkspace(newWorkspaceLocation));
}

TEST_F( WorkspaceGenerator, testWorkspaceGeneratorUsingSuggestedName )
{
  QDir oldWorkspaceLocation = QDir(QDir::current().absolutePath() + "/SAC");
  QDir newWorkspaceLocation = QDir(casaWizard::workspaceGenerator::getSuggestedWorkspace(oldWorkspaceLocation.absolutePath()));
  if (newWorkspaceLocation.exists()) QDir().remove(newWorkspaceLocation.path());

  EXPECT_TRUE (casaWizard::workspaceGenerator::createWorkspace(oldWorkspaceLocation.absolutePath(), newWorkspaceLocation.absolutePath()));
  EXPECT_TRUE (checkFiles(oldWorkspaceLocation.absolutePath(), newWorkspaceLocation.absolutePath()));
  EXPECT_TRUE (deleteNewWorkspace(newWorkspaceLocation));
}
