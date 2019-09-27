//
// Copyright (C) 2013-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// Genex0dProjectManager class: handles the project

#pragma once

#include <memory>
#include <string>
#include <vector>

namespace mbapi
{
class Model;
} // namespace mbapi

namespace genex0d
{

class Genex0dProjectManager
{ 
public:
  explicit Genex0dProjectManager(const std::string & projectFileName, const double xCoord, const double yCoord,
                                 const std::string & topSurfaceName);
  ~Genex0dProjectManager();

  std::vector<double> agesFromMajorSnapShots();
  std::vector<double> requestPropertyHistory(const std::string & propertyName);

private:
  void reloadModel();
  void clearTable();
  void saveModel();
  void getValues(std::vector<double> & values) const;
  void requestPropertyInSnapshots();
  void setInTable();

  const std::string m_projectFileName;
  const double m_xCoord;
  const double m_yCoord;
  const std::string m_topSurfaceName;
  std::unique_ptr<mbapi::Model> m_mdl;
  int m_posData;
  double m_posDataPrevious;
  std::string m_propertyName;
};

} // namespace genex0d
