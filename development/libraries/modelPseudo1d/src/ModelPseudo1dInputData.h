//
// Copyright (C) 2013-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// ModelPseudo1dInputData class: contains input data structure

#pragma once


#include <string>

namespace mbapi
{
class Model;
} // namespace mbapi


namespace modelPseudo1d
{

struct ModelPseudo1dInputData
{ 
  mbapi::Model & m_mdl;
  double xCoord;
  double yCoord;
  std::string projectFilename;
  std::string outProjectFilename;

  ModelPseudo1dInputData(mbapi::Model& mdl,
                         const double xCoord = 0.0,
                         const double yCoord = 0.0,
                         const std::string & projectFilename = "",
                         const std::string & outProjectFilename = ""
                         ) :
    m_mdl{mdl},
    xCoord{xCoord},
    yCoord{yCoord},
    projectFilename{projectFilename},
    outProjectFilename{outProjectFilename}

  {
  }
};

} // namespace modelPseudo1d
