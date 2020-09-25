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
#include <cstring>
#include <iostream>
#include <cmath>

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

  return NO_EXIT;
}

void Genex0dInputManager::printHelp() const
{
  LogHandler(LogHandler::INFO_SEVERITY)
      << "Genex0d extracts source rock properties at (X,Y) location, given formation name, source rock type, "
         "and properties. \n \n"
      << "Usage:\n"
      << " -help             display this help message\n"
      << " -project          project3d file\n"
      << " -out              project3d output file\n"
      << " -formation        Formation name\n"
      << " -SRType           Source Rock Type\n"
      << " -X                X coordinate [m]\n"
      << " -Y                Y coordinate [m]\n"
      << " -TOC              Total Organic Carbon (%)\n"
      << " -HC               Hydro Carbon ratio [-]\n"
      << " -SC               Sulphur Carbon ratio [-]\n"
      << " -Asph             Asphaltene Diffusion Energy [kJ]\n"
      << " -Resin            Resin Diffusion Energy [kJ]\n"
      << " -C15Aro           C15 Aromatic Diffusion Energy [kJ]\n"
      << " -C15Sat           C15 Saturate Diffusion Energy [kJ] \n"
      << " -EA               PreAsphaltene Activation energy [kJ]\n"
      << " -VesLimit         Maximum Ves Value (optional) [MPa] \n"
      << "\n"
      << "Example:\n"
      << "  genex0d -project Project.project3d -out outProj.project3d -formation \"Zechstein\" -SRType \"Type I - Lacustrine\" -X 423000.0 -Y 6730000.0 -TOC 10 -HC 1.13 -SC 0.05 -EA 210 -Asph 87 -Resin 80 -C15Aro 77 -C15Sat 71";
}

Genex0dInputManager::ExitStatus Genex0dInputManager::checkInputIsValid(std::string & ioErrorMessage) const
{
  double epsilon = 1e-5;

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

  if (std::fabs(m_inputData.ToCIni - CauldronNoDataValue) < epsilon)
  {
    ioErrorMessage = "No initial TOC provided!";
    return WITH_ERROR_EXIT;
  }

  if (std::fabs(m_inputData.xCoord - CauldronNoDataValue) < epsilon)
  {
    ioErrorMessage = "No x-coordinate provided!";
    return WITH_ERROR_EXIT;
  }

  if (std::fabs(m_inputData.yCoord - CauldronNoDataValue) < epsilon)
  {
    ioErrorMessage = "No y-coordinate provided!";
    return WITH_ERROR_EXIT;
  }

  if (std::fabs(m_inputData.HCVRe05 - CauldronNoDataValue) < epsilon)
  {
    ioErrorMessage = "No H/C ratio provided!";
    return WITH_ERROR_EXIT;
  }

  if (std::fabs(m_inputData.SCVRe05 - CauldronNoDataValue) < epsilon)
  {
    ioErrorMessage = "No S/C ratio provided!";
    return WITH_ERROR_EXIT;
  }

  if (std::fabs(m_inputData.activationEnergy - CauldronNoDataValue) < epsilon)
  {
    ioErrorMessage = "No activation energy provided!";
    return WITH_ERROR_EXIT;
  }

  if (std::fabs(m_inputData.resinDiffusionEnergy - CauldronNoDataValue) < epsilon)
  {
    ioErrorMessage = "No resin diffusion energy provided!";
    return WITH_ERROR_EXIT;
  }

  if (std::fabs(m_inputData.C15AroDiffusionEnergy - CauldronNoDataValue) < epsilon)
  {
    ioErrorMessage = "No C15 Aro diffusion energy provided!";
    return WITH_ERROR_EXIT;
  }

  if (std::fabs(m_inputData.C15SatDiffusionEnergy - CauldronNoDataValue) < epsilon)
  {
    ioErrorMessage = "No C15 Sat diffusion energy provided!";
    return WITH_ERROR_EXIT;
  }

  if (std::fabs(m_inputData.asphalteneDiffusionEnergy - CauldronNoDataValue) < epsilon)
  {
    ioErrorMessage = "No asphaltene diffusion energy provided!";
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
  m_argumntFields["-VesLimit"] = 0;
  m_argumntFields["-EA"] = 0;
  m_argumntFields["-Asph"] = 0;
  m_argumntFields["-Resin"] = 0;
  m_argumntFields["-C15Aro"] = 0;
  m_argumntFields["-C15Sat"] = 0;
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
    else if (argvn == "-VesLimit")
    {
      m_inputData.maxVesEnabled = true;
      m_inputData.maxVes = std::stod(argvnp1);
    }
    else if (argvn == "-EA")
    {
      m_inputData.activationEnergy = std::stod(argvnp1) * 1e3;
    }
    else if (argvn == "-Asph")
    {
      m_inputData.asphalteneDiffusionEnergy = std::stod(argvnp1) * 1e3;
    }
    else if (argvn == "-Resin")
    {
      m_inputData.resinDiffusionEnergy = std::stod(argvnp1) * 1e3;
    }
    else if (argvn == "-C15Aro")
    {
      m_inputData.C15AroDiffusionEnergy = std::stod(argvnp1) * 1e3;
    }
    else if (argvn == "-C15Sat")
    {
      m_inputData.C15SatDiffusionEnergy = std::stod(argvnp1) * 1e3;
    }
  }

  return checkInputIsValid(ioErrorMessage);
}

} // namespace genex0d
