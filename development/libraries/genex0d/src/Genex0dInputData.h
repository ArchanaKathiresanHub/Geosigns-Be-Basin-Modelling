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

namespace genex0d
{

struct Genex0dInputData
{ 
  double xCoord;
  double yCoord;
  double ToCIni;
  double HCVRe05;
  double SCVRe05;
  std::string projectFilename;
  std::string outProjectFilename;
  std::string formationName;
  std::string sourceRockType;
  std::string nodeHistoryFileName;

  Genex0dInputData(const double xCoord = 0,
                   const double yCoord = 0.0,
                   const double ToCIni = 0.0,
                   const double HCVRe05 = 0.0,
                   const double SCVRe05 = 0.0,
                   const std::string & projectFilename = "",
                   const std::string & outProjectFilename = "out.project3d",
                   const std::string & formationName = "",
                   const std::string & sourceRockType = "",
                   const std::string & nodeHistoryFileName = "nodeHistory.dat" ) :
    xCoord{xCoord},
    yCoord{yCoord},
    ToCIni{ToCIni},
    HCVRe05{HCVRe05},
    SCVRe05{SCVRe05},
    projectFilename{projectFilename},
    outProjectFilename{outProjectFilename},
    formationName{formationName},
    sourceRockType{sourceRockType},
    nodeHistoryFileName{nodeHistoryFileName}
  {
  }
};

} // namespace genex0d
