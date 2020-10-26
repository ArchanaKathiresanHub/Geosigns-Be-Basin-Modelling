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

namespace Genex0d
{

Genex0dInputManager::Genex0dInputManager(int argc, char** argv) :
  m_argc{argc},
  m_argv{argv},
  m_inputData{},
  m_argumentFields{}
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
      << " -out              project3d output file (optional) \n"
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
      << " -AdsSimulator     Adsorption simulator Name \n"
      << " -AdsCapacityFunc  Adsorption Capacity Function Name\n"
      << " -doOTGC           Enables Oil-To-Gass-Cracking (put 1 as an argument to enable)\n"
      << "\n"
      << "Example:\n"
      << "  genex0d -project AcquiferScale1.project3d -formation \"Formation5\" -SRType \"Type_II_Paleozoic_Marine_Shale_kin_s\" -X 0.0 -Y 0.0 -TOC 20.0 -HC 1.24 -SC 0.05 -EA 210 -Asph 87 -Resin 83 -C15Aro 75 -C15Sat 71 -AdsSimulator \"OTGCC1AdsorptionSimulator\" -AdsCapacityFunc \"Default Langmuir Isotherm\" -doOTGC 1";
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

  if (!m_inputData.whichAdsorptionSimulator.empty())
  {
    if (m_inputData.whichAdsorptionFunction.empty())
    {
      ioErrorMessage = "An adsorption simulator was provided, but no adsorption capacity function";
      return WITH_ERROR_EXIT;
    }
  }

  if (m_inputData.whichAdsorptionSimulator.empty())
  {
    if (!m_inputData.whichAdsorptionFunction.empty())
    {
      ioErrorMessage = "An adsorption capacity function was provided, but no adsorption simulator";
      return WITH_ERROR_EXIT;
    }
    if (m_inputData.doOTCG)
    {
      ioErrorMessage = "OTGC was enabled, but no adsorption simulator was provided";
      return WITH_ERROR_EXIT;
    }
  }

  if (!m_inputData.sourceRockTypeSR2.empty())
  {
    if (std::fabs(m_inputData.HCVRe05SR2 - CauldronNoDataValue) < epsilon)
    {
      ioErrorMessage = "No H/C ratio provided for source rock 2!";
      return WITH_ERROR_EXIT;
    }

    if (std::fabs(m_inputData.SCVRe05SR2 - CauldronNoDataValue) < epsilon)
    {
      ioErrorMessage = "No S/C ratio provided for source rock 2!";
      return WITH_ERROR_EXIT;
    }

    if (std::fabs(m_inputData.activationEnergySR2 - CauldronNoDataValue) < epsilon)
    {
      ioErrorMessage = "No activation energy provided for source rock 2!";
      return WITH_ERROR_EXIT;
    }

    if (std::fabs(m_inputData.resinDiffusionEnergySR2 - CauldronNoDataValue) < epsilon)
    {
      ioErrorMessage = "No resin diffusion energy provided for source rock 2!";
      return WITH_ERROR_EXIT;
    }

    if (std::fabs(m_inputData.C15AroDiffusionEnergySR2 - CauldronNoDataValue) < epsilon)
    {
      ioErrorMessage = "No C15 Aro diffusion energy provided for source rock 2!";
      return WITH_ERROR_EXIT;
    }

    if (std::fabs(m_inputData.C15SatDiffusionEnergySR2 - CauldronNoDataValue) < epsilon)
    {
      ioErrorMessage = "No C15 Sat diffusion energy provided for source rock 2!";
      return WITH_ERROR_EXIT;
    }

    if (std::fabs(m_inputData.asphalteneDiffusionEnergySR2 - CauldronNoDataValue) < epsilon)
    {
      ioErrorMessage = "No asphaltene diffusion energy provided for source rock 2!";
      return WITH_ERROR_EXIT;
    }

    if (std::fabs(m_inputData.mixingHI - CauldronNoDataValue) < epsilon)
    {
      ioErrorMessage = "A second source rock was provided, but no mixing HI!";
      return WITH_ERROR_EXIT;
    }
  }

  if (m_inputData.sourceRockTypeSR2.empty())
  {
    if (std::fabs(m_inputData.HCVRe05SR2 - CauldronNoDataValue) >= epsilon ||
        std::fabs(m_inputData.SCVRe05SR2 - CauldronNoDataValue) >= epsilon ||
        std::fabs(m_inputData.activationEnergySR2 - CauldronNoDataValue) >= epsilon ||
        std::fabs(m_inputData.resinDiffusionEnergySR2 - CauldronNoDataValue) >= epsilon ||
        std::fabs(m_inputData.C15AroDiffusionEnergySR2 - CauldronNoDataValue) >= epsilon ||
        std::fabs(m_inputData.C15SatDiffusionEnergySR2 - CauldronNoDataValue) >= epsilon ||
        std::fabs(m_inputData.asphalteneDiffusionEnergySR2 - CauldronNoDataValue) >= epsilon)
    {
      ioErrorMessage = "No source rock type provided for source rock 2!";
      return WITH_ERROR_EXIT;
    }
  }

  return NO_EXIT;
}

const Genex0dInputData & Genex0dInputManager::inputData() const
{
  return m_inputData;
}

void Genex0dInputManager::setArgumentFieldNames()
{
  m_argumentFields["-project"] = 0;
  m_argumentFields["-out"] = 0;
  m_argumentFields["-formation"] = 0;
  m_argumentFields["-SRType"] = 0;
  m_argumentFields["-X"] = 0;
  m_argumentFields["-Y"] = 0;
  m_argumentFields["-TOC"] = 0;
  m_argumentFields["-HC"] = 0;
  m_argumentFields["-SC"] = 0;
  m_argumentFields["-VesLimit"] = 0;
  m_argumentFields["-EA"] = 0;
  m_argumentFields["-Asph"] = 0;
  m_argumentFields["-Resin"] = 0;
  m_argumentFields["-C15Aro"] = 0;
  m_argumentFields["-C15Sat"] = 0;
  m_argumentFields["-AdsSimulator"] = 0;
  m_argumentFields["-AdsCapacityFunc"] = 0;
  m_argumentFields["-doOTGC"] = 0;

  // Source Rock 2 parameters
  m_argumentFields["-SRType_SR2"] = 0;
  m_argumentFields["-HC_SR2"] = 0;
  m_argumentFields["-SC_SR2"] = 0;
  m_argumentFields["-EA_SR2"] = 0;
  m_argumentFields["-Asph_SR2"] = 0;
  m_argumentFields["-Resin_SR2"] = 0;
  m_argumentFields["-C15Aro_SR2"] = 0;
  m_argumentFields["-C15Sat_SR2"] = 0;
  m_argumentFields["-MixingHI"] = 0;

}

bool Genex0dInputManager::initialCheckArgument(const std::string& argument, const std::string& argumentValue, std::string& ioErrorMessage)
{
  if (m_argumentFields.count(argument) == 0)
  {
    ioErrorMessage = "Unknown argument provided: \"" + argument
        + "\"\n Use -help flag to see all options.";
    return false;
  }
  if (m_argumentFields.at(argument) > 0)
  {
    ioErrorMessage = "Repeated argument found: \"" + argument + "\".\n";
    return false;
  }

  if (argumentValue.empty())
  {
    ioErrorMessage = "Input error! Empty argument value for: " + argument;
    return false;
  }

  return true;
}


void Genex0dInputManager::storeArgument(const std::string& argument, const std::string& argumentValue)
{
  if (argument == "-project")
  {
    m_inputData.projectFilename = argumentValue;
  }
  else if (argument == "-out")
  {
    m_inputData.outProjectFilename = argumentValue;
  }
  else if (argument == "-formation")
  {
    m_inputData.formationName = argumentValue;
  }
  else if (argument == "-SRType")
  {
    m_inputData.sourceRockType = argumentValue;
  }
  else if (argument == "-X")
  {
    m_inputData.xCoord = std::stod(argumentValue);
  }
  else if (argument == "-Y")
  {
    m_inputData.yCoord = std::stod(argumentValue);
  }
  else if (argument == "-TOC")
  {
    m_inputData.ToCIni = std::stod(argumentValue);
  }
  else if (argument == "-HC")
  {
    m_inputData.HCVRe05 = std::stod(argumentValue);
  }
  else if (argument == "-SC")
  {
    m_inputData.SCVRe05 = std::stod(argumentValue);
  }
  else if (argument == "-VesLimit")
  {
    m_inputData.maxVesEnabled = true;
    m_inputData.maxVes = std::stod(argumentValue);
  }
  else if (argument == "-EA")
  {
    m_inputData.activationEnergy = std::stod(argumentValue);
  }
  else if (argument == "-Asph")
  {
    m_inputData.asphalteneDiffusionEnergy = std::stod(argumentValue);
  }
  else if (argument == "-Resin")
  {
    m_inputData.resinDiffusionEnergy = std::stod(argumentValue);
  }
  else if (argument == "-C15Aro")
  {
    m_inputData.C15AroDiffusionEnergy = std::stod(argumentValue);
  }
  else if (argument == "-C15Sat")
  {
    m_inputData.C15SatDiffusionEnergy = std::stod(argumentValue);
  }
  else if (argument == "-AdsSimulator")
  {
    m_inputData.whichAdsorptionSimulator = argumentValue;
  }
  else if (argument == "-AdsCapacityFunc")
  {
    m_inputData.whichAdsorptionFunction = argumentValue;
  }
  else if (argument == "-doOTGC")
  {
    if (argumentValue == "1")
    {
      m_inputData.doOTCG = true ;
    }
  }

  // Source Rock 2 parameters
  else if (argument == "-SRType_SR2")
  {
    m_inputData.sourceRockTypeSR2 = argumentValue;
  }
  else if (argument == "-HC_SR2")
  {
    m_inputData.HCVRe05SR2 = std::stod(argumentValue);
  }
  else if (argument == "-SC_SR2")
  {
    m_inputData.SCVRe05SR2 = std::stod(argumentValue);
  }
  else if (argument == "-EA_SR2")
  {
    m_inputData.activationEnergySR2 = std::stod(argumentValue);
  }
  else if (argument == "-Asph_SR2")
  {
    m_inputData.asphalteneDiffusionEnergySR2 = std::stod(argumentValue);
  }
  else if (argument == "-Resin_SR2")
  {
    m_inputData.resinDiffusionEnergySR2 = std::stod(argumentValue);
  }
  else if (argument == "-C15Aro_SR2")
  {
    m_inputData.C15AroDiffusionEnergySR2 = std::stod(argumentValue);
  }
  else if (argument == "-C15Sat_SR2")
  {
    m_inputData.C15SatDiffusionEnergySR2 = std::stod(argumentValue);
  }
  else if (argument == "-MixingHI")
  {
    m_inputData.mixingHI = std::stod(argumentValue);
  }


}

Genex0dInputManager::ExitStatus Genex0dInputManager::storeInput(std::string & ioErrorMessage)
{
  for (int argn = 1; argn < m_argc; ++argn)
  {
    const std::string argument = std::string(m_argv[argn]);

    // Check if the last argument has a value (needs to be checked separately to initialize argumentValue
    if (argn + 1 >= m_argc)
    {
      ioErrorMessage = "Input error! Empty argument provided for: " + argument;
      return WITH_ERROR_EXIT;
    }

    const std::string argumentValue = std::string(m_argv[++argn]);

    if (!initialCheckArgument(argument, argumentValue, ioErrorMessage))
    {
      return WITH_ERROR_EXIT;
    }

    storeArgument(argument, argumentValue);
    m_argumentFields.at(argument)++;
  }

  return checkInputIsValid(ioErrorMessage);
}

} // namespace genex0d
