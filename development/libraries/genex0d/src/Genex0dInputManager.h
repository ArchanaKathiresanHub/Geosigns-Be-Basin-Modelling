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
#include <vector>

namespace Genex0d
{

class Genex0dInputManager
{ 
public:
  explicit Genex0dInputManager(const std::vector<std::string>& arguments);

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
  bool initialCheckArgument(const std::string& argument, const std::string& argumentValue, std::string& ioErrorMessage);
  void storeArgument(const std::string& argument, const std::string& argumentValue);
  void setArgumentFieldNames();

  std::vector<std::string> m_argv;

  std::unordered_map<std::string, int> m_argumentFields;
  Genex0dInputData m_inputData;
};

} // namespace genex0d
