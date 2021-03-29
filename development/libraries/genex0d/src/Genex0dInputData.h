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

namespace Genex0d
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
  std::string adsorptionFunctionTPVData;
  std::string irreducibleWaterSaturationData;
  double HCVRe05SR2;
  double SCVRe05SR2;
  double activationEnergySR2;
  double asphalteneDiffusionEnergySR2;
  double resinDiffusionEnergySR2;
  double C15AroDiffusionEnergySR2;
  double C15SatDiffusionEnergySR2;
  std::string sourceRockTypeSR2;
  double mixingHI;

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
                   const std::string& whichAdsorptionFunction = "",
                   const std::string& adsorptionFunctionData = "",
                   const std::string& irreducibleWaterSaturationData = "",
                   const double HCVRe05SR2 = CauldronNoDataValue,
                   const double SCVRe05SR2 = CauldronNoDataValue,
                   const double activationEnergySR2 = CauldronNoDataValue,
                   const double asphalteneDiffusionEnergySR2 = CauldronNoDataValue,
                   const double resinDiffusionEnergySR2 = CauldronNoDataValue,
                   const double C15AroDiffusionEnergySR2 = CauldronNoDataValue,
                   const double C15SatDiffusionEnergySR2 = CauldronNoDataValue,
                   const std::string& sourceRockTypeSR2 = "",
                   const double mixingHI = CauldronNoDataValue) :
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
    whichAdsorptionFunction{whichAdsorptionFunction},
    adsorptionFunctionTPVData{adsorptionFunctionData},
    irreducibleWaterSaturationData{irreducibleWaterSaturationData},
    HCVRe05SR2{HCVRe05SR2},
    SCVRe05SR2{SCVRe05SR2},
    activationEnergySR2{activationEnergySR2},
    asphalteneDiffusionEnergySR2{asphalteneDiffusionEnergySR2},
    resinDiffusionEnergySR2{resinDiffusionEnergySR2},
    C15AroDiffusionEnergySR2{C15AroDiffusionEnergySR2},
    C15SatDiffusionEnergySR2{C15SatDiffusionEnergySR2},
    sourceRockTypeSR2{sourceRockTypeSR2},
    mixingHI{mixingHI}
  {
  }
};
} // namespace genex0d
