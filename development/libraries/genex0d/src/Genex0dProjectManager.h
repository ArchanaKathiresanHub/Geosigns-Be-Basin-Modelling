//
// Copyright (C) 2013-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// Genex0dProjectManager class: handles the project

#pragma once

// cmbAPI
#include "cmbAPI.h"

#include <memory>
#include <string>
#include <vector>

namespace genex0d
{

class Genex0dProjectManager
{ 
public:
  explicit Genex0dProjectManager(const std::string & projectFileName, const std::string & outProjectFileName, const double xCoord, const double yCoord);

  void resetWindowingAndSampling(const unsigned int indI, const unsigned int indJ);

private:
  void reloadModel(const std::string & projectFileName);
  void saveModel(const std::string & name);

  const std::string m_projectFileName;
  const double m_xCoord;
  const double m_yCoord;
  std::unique_ptr<mbapi::Model> m_mdl;
  int m_posData;
  double m_posDataPrevious;
  std::string m_outProjectFileName;
};

} // namespace genex0d
