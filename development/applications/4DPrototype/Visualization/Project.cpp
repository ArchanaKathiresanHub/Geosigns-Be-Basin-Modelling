//
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "DataAccessProject.h"
#include "VisualizationIOProject.h"

namespace
{
  bool stringEndsWith(const std::string& str, const std::string& end)
  {
    return str.size() >= end.size() && str.substr(str.size() - end.size()) == end;
  }
}
std::shared_ptr<Project> Project::load(const std::string& path)
{
  if (stringEndsWith(path, ".xml"))
    return std::make_shared<VisualizationIOProject>(path);
  else if (stringEndsWith(path, ".project3d"))
    return std::make_shared<DataAccessProject>(path);

  throw std::runtime_error("Could not open project");
}