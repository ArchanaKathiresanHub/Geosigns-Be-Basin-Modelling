//
// Copyright (C) 2013-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// ModelPseudo1dInputManager class: handles input data

#pragma once

#include "ModelPseudo1dInputData.h"

#include <string>
#include <unordered_map>

namespace modelPseudo1d
{

class ModelPseudo1dInputManager
{ 
public:
  explicit ModelPseudo1dInputManager(int argc, char** argv);

  enum ExitStatus
  {
    NO_ERROR_EXIT,
    WITH_ERROR_EXIT,
    NO_EXIT
  };

  ExitStatus initialCheck(std::string & ioErrorMessage) const;
  ExitStatus storeInput(std::string & ioErrorMessage);

  const ModelPseudo1dInputData & inputData() const;

private:
  void setArgumentFieldNames();
  void setDefaultInputValues();
  void printHelp() const;
  ExitStatus checkInputIsValid(std::string & ioErrorMessage) const;

  int m_argc;
  char** m_argv;

  std::unordered_map<std::string, int> m_argumntFields;
  ModelPseudo1dInputData m_inputData;
};

} // namespace modelPseudo1d
