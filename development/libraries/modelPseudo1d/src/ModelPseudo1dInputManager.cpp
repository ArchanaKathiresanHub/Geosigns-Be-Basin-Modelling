//
// Copyright (C) 2013-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "ModelPseudo1dInputManager.h"

#include "LogHandler.h"

#include <iostream>
#include <cstring>

namespace modelPseudo1d
{

ModelPseudo1dInputManager::ModelPseudo1dInputManager(int argc, char** argv) :
  m_argc{argc},
  m_argv{argv},
  m_argumntFields{},
  m_inputData{}
{
  setDefaultInputValues();
  setArgumentFieldNames();
}

void ModelPseudo1dInputManager::setDefaultInputValues()
{
  m_inputData.outProjectFilename = "ProjectPseudo1d.project3d";
}

ModelPseudo1dInputManager::ExitStatus ModelPseudo1dInputManager::initialCheck(std::string & ioErrorMessage) const
{
  if (m_argc <= 2 || std::strcmp(m_argv[1], "-help") == 0)
  {
    printHelp();
    return NO_ERROR_EXIT;
  }
  else if (m_argc < 7)
  {
    ioErrorMessage = "Not enough arguments provided! \n Use -help flag to see all options.";
    return WITH_ERROR_EXIT;
  }

  return NO_EXIT;
}

void ModelPseudo1dInputManager::printHelp() const
{
  LogHandler(LogHandler::INFO_SEVERITY)
      << "ModelPseudo1d converts a 3D model with input HDF maps at given (X,Y) location, to a pseudo 1D model, i.e., "
         "a 3D model with only one cell in (X,Y) plane. The application extracts scalar values from the input HDF maps, "
         "and stores the extracted scalars in the project3d tables. The resulting model is then used independent of input HDF maps."
      << "\n"
      << "Usage:\n"
      << " -help                            displays help message\n"
      << " -project <project3d file>        project3d file\n"
      << " -X       <X coordinate>          X coordinate\n"
      << " -Y       <Y coordinate>          Y coordinate\n"
      << " -out     <output project3d file> resulting output project3d file (default: ProjectPseudo1d.project3d)\n"
      << " Note: the arguments -project, -X, and -Y must be specified!"
      << "\n"
      << "Example:\n"
      << "  ModelPseudo1d -project Project.project3d -X 1000.0 -Y 1000.0 -out ProjectPseudo1d.project3d";
}

ModelPseudo1dInputManager::ExitStatus ModelPseudo1dInputManager::checkInputIsValid(std::string & ioErrorMessage) const
{
  if (m_inputData.projectFilename.empty())
  {
    ioErrorMessage =  "No project file provided!";
    return WITH_ERROR_EXIT;
  }

  return NO_EXIT;
}

const ModelPseudo1dInputData & ModelPseudo1dInputManager::inputData() const
{
  return m_inputData;
}

void ModelPseudo1dInputManager::setArgumentFieldNames()
{
  m_argumntFields["-project"] = 0;
  m_argumntFields["-X"] = 0;
  m_argumntFields["-Y"] = 0;
  m_argumntFields["-out"] = 0;
}

ModelPseudo1dInputManager::ExitStatus ModelPseudo1dInputManager::storeInput(std::string & ioErrorMessage)
{
  for (int argn = 1; argn < m_argc; ++argn)
  {
    const std::string argvn = std::string(m_argv[argn]);
    if (m_argumntFields.count(argvn) == 0)
    {
      ioErrorMessage = "Unknown argument provided: \"" + argvn
          + "\"\n Use -help flag to see all options.";
      return WITH_ERROR_EXIT;
    }
    if (m_argumntFields.at(argvn) > 0)
    {
      ioErrorMessage = "Repeated argument found: \"" + argvn + "\".\n";
      return WITH_ERROR_EXIT;
    }
    m_argumntFields.at(argvn)++;

    if (argn + 1 >= m_argc)
    {
      ioErrorMessage = "Input error! Empty argument provided for: " + argvn;
      return WITH_ERROR_EXIT;
    }

    const std::string argvnp1 = std::string(m_argv[++argn]);
    if (argvnp1.empty() || m_argumntFields.count(argvnp1) != 0)
    {
      ioErrorMessage = "Input error! Empty argument value for: " + argvn;
      return WITH_ERROR_EXIT;
    }

    if (argvn == "-project")
    {
      m_inputData.projectFilename = argvnp1;
    }
    else if (argvn == "-X")
    {
      m_inputData.xCoord = std::stod(argvnp1);
    }
    else if (argvn == "-Y")
    {
      m_inputData.yCoord = std::stod(argvnp1);
    }
    else if (argvn == "-out")
    {
      m_inputData.outProjectFilename = argvnp1;
    }
  }

  return checkInputIsValid(ioErrorMessage);
}

} // namespace genex0d
