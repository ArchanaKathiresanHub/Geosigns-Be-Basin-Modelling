//
// Copyright (C) 2013-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "Genex0dInputManager.h"

#include "LogHandler.h"

#include <algorithm>
#include <iostream>
#include <cstring>

namespace genex0d
{

Genex0dInputManager::Genex0dInputManager(int argc, char** argv) :
  m_argc{argc},
  m_argv{argv},
  m_inputData{},
  m_argumntFields{}
{
  setArgumentFieldNames();
}

Genex0dInputManager::ExitStatus Genex0dInputManager::initialCheck(std::string & ioErrorMessage) const
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

void Genex0dInputManager::printHelp() const
{
  LogHandler(LogHandler::INFO_SEVERITY)
      << "Genex0d extracts source rock properties at (X,Y) location, given formation name, source rock type, "
         "and properties. \n"
      << "Usage:\n"
      << " -help                       display this help message\n"
      << " -project   <project3d file> project3d file\n"
      << " -formation <formation>      name of a formation\n"
      << " -SRType    <SR type>        type of a source rock\n"
      << " -X         <X coordinate>   X coordinate\n"
      << " -Y         <Y coordinate>   Y coordinate\n"
      << " -TOC       <TOC>            total organic carbon ([0-100]%)\n"
      << " -HC        <H/C>            Hydro Carbon ratio\n"
      << " -SC        <S/C>            Sulphur Carbon ratio\n"
      << " Note: -project, -formation, and -SRType must be specified! The other parameters are optional."
      << "\n"
      << "Example:\n"
      << "  genex0d -project Project.project3d -formation \"Zechstein\" SRType \"Type I - Lacustrine\" "
         "  -X 1000.0 -Y 1000.0 -TOC 10 -HC 1.45 -SC 0.03";
}

Genex0dInputManager::ExitStatus Genex0dInputManager::checkInputIsValid(std::string & ioErrorMessage) const
{
  if (m_inputData.projectFilename.empty())
  {
    ioErrorMessage =  "No project file provided!";
    return WITH_ERROR_EXIT;
  }

  if (m_inputData.formationName.empty())
  {
    ioErrorMessage =  "No formation name provided!";
    return WITH_ERROR_EXIT;
  }

  if (m_inputData.sourceRockType.empty())
  {
    ioErrorMessage = "No source rock type (SRType) provided!";
    return WITH_ERROR_EXIT;
  }

  return NO_EXIT;
}

const Genex0dInputData & Genex0dInputManager::inputData() const
{
  return m_inputData;
}

void Genex0dInputManager::setArgumentFieldNames()
{
  m_argumntFields["-project"] = 0;
  m_argumntFields["-out"] = 0;
  m_argumntFields["-formation"] = 0;
  m_argumntFields["-SRType"] = 0;
  m_argumntFields["-X"] = 0;
  m_argumntFields["-Y"] = 0;
  m_argumntFields["-TOC"] = 0;
  m_argumntFields["-HC"] = 0;
  m_argumntFields["-SC"] = 0;
}

Genex0dInputManager::ExitStatus Genex0dInputManager::storeInput(std::string & ioErrorMessage)
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
    else if (argvn == "-out")
    {
      m_inputData.outProjectFilename = argvnp1;
    }
    else if (argvn == "-formation")
    {
      m_inputData.formationName = argvnp1;
    }
    else if (argvn == "-SRType")
    {
      m_inputData.sourceRockType = argvnp1;
    }
    else if (argvn == "-X")
    {
      m_inputData.xCoord = std::stod(argvnp1);
    }
    else if (argvn == "-Y")
    {
      m_inputData.yCoord = std::stod(argvnp1);
    }
    else if (argvn == "-TOC")
    {
      m_inputData.ToCIni = std::stod(argvnp1);
    }
    else if (argvn == "-HC")
    {
      m_inputData.HCVRe05 = std::stod(argvnp1);
    }
    else if (argvn == "-SC")
    {
      m_inputData.SCVRe05 = std::stod(argvnp1);
    }
  }

  return checkInputIsValid(ioErrorMessage);
}

} // namespace genex0d
