//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include <QString>

class ProjectTXTManipulator
{
public:
  ProjectTXTManipulator(const QString& projectTextFile);
  void appendStampToScenarioName(const QString& stamp) const;

  QString projectTextFile_;
};

class FileDoesNotExistException : std::runtime_error
{
public:
   FileDoesNotExistException(std::string str):std::runtime_error(str){}
};
