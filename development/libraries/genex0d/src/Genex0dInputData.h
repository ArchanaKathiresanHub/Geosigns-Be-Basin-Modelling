//
// Copyright (C) 2013-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// Genex0dInputData class: contains input data structure

#pragma once

#include <string>

// utilities library
#include "ConstantsNumerical.h"
using Utilities::Numerical::CauldronNoDataValue;

namespace genex0d
{

struct Genex0dInputData
{ 
  double xCoord;
  double yCoord;
  double ToCIni;
  double HCVRe05;
  double SCVRe05;
  double activationEnergy;
  double asphalteneDiffusionEnergy;
  double resinDiffusionEnergy;
  double C15AroDiffusionEnergy;
  double C15SatDiffusionEnergy;
  double maxVes;
  bool maxVesEnabled;
  bool doOTCG;
  std::string projectFilename;
  std::string outProjectFilename;
  std::string formationName;
  std::string sourceRockType;
  std::string nodeHistoryFileName;
  std::string whichAdsorptionSimulator;
  std::string whichAdsorptionFunction;

  Genex0dInputData(const double xCoord = CauldronNoDataValue,
                   const double yCoord = CauldronNoDataValue,
                   const double ToCIni = CauldronNoDataValue,
                   const double HCVRe05 = CauldronNoDataValue,
                   const double SCVRe05 = CauldronNoDataValue,
                   const double activationEnergy = CauldronNoDataValue,
                   const double asphalteneDiffusionEnergy = CauldronNoDataValue,
                   const double resinDiffusionEnergy = CauldronNoDataValue,
                   const double C15AroDiffusionEnergy = CauldronNoDataValue,
                   const double C15SatDiffusionEnergy = CauldronNoDataValue,
                   const double maxVes = CauldronNoDataValue,
                   const bool maxVesEnabled = false,
                   const bool doOTCG = false,
                   const std::string& projectFilename = "",
                   const std::string& outProjectFilename = "out.project3d",
                   const std::string& formationName = "",
                   const std::string& sourceRockType = "",
                   const std::string& nodeHistoryFileName = "nodeHistory.dat",
                   const std::string& whichAdsorptionSimulator = "",
                   const std::string& whichAdsorptionFunction = "") :
    xCoord{xCoord},
    yCoord{yCoord},
    ToCIni{ToCIni},
    HCVRe05{HCVRe05},
    SCVRe05{SCVRe05},
    activationEnergy{activationEnergy},
    asphalteneDiffusionEnergy{asphalteneDiffusionEnergy},
    resinDiffusionEnergy{resinDiffusionEnergy},
    C15AroDiffusionEnergy{C15AroDiffusionEnergy},
    C15SatDiffusionEnergy{C15SatDiffusionEnergy},
    maxVes{maxVes},
    maxVesEnabled{maxVesEnabled},
    doOTCG{doOTCG},
    projectFilename{projectFilename},
    outProjectFilename{outProjectFilename},
    formationName{formationName},
    sourceRockType{sourceRockType},
    nodeHistoryFileName{nodeHistoryFileName},
    whichAdsorptionSimulator{whichAdsorptionSimulator},
    whichAdsorptionFunction{whichAdsorptionFunction}
  {
  }
};

} // namespace genex0d
