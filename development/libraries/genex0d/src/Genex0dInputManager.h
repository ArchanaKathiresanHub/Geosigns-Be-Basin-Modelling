//
// Copyright (C) 2013-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// Genex0dInputManager class: handles input data

#pragma once

#include "Genex0dInputData.h"

#include <string>
#include <unordered_map>

namespace genex0d
{

class Genex0dInputManager
{ 
public:
  explicit Genex0dInputManager(int argc, char** argv);

  enum ExitStatus
  {
    NO_ERROR_EXIT,
    WITH_ERROR_EXIT,
    NO_EXIT
  };

  ExitStatus initialCheck(std::string & ioErrorMessage) const;
  ExitStatus storeInput(std::string & ioErrorMessage);

  const Genex0dInputData & inputData() const;

private:
  void printHelp() const;
  ExitStatus checkInputIsValid(std::string & ioErrorMessage) const;
  void setArgumentFieldNames();

  int m_argc;
  char** m_argv;

  std::unordered_map<std::string, int> m_argumntFields;
  Genex0dInputData m_inputData;
};

} // namespace genex0d
