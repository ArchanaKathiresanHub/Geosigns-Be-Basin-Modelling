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
#include <unordered_map>

namespace DataAccess
{
namespace Interface
{
class ProjectHandle;
class ObjectFactory;
} // namespace Interface
} // namespace DataAccess

namespace mbapi
{
class Model;
} // namespace mbapi

namespace Genex0d
{

class Genex0dProjectManager
{ 
public:
  explicit Genex0dProjectManager(const DataAccess::Interface::ProjectHandle & projectHandle, const std::string & projectFileName, const double xCoord,
                                 const double yCoord, const std::string & topSurfaceName, const std::string & formationName);
  ~Genex0dProjectManager();

  void requestPropertyHistory(const std::string & propertyName);
  void extract();
  std::vector<double> getValues(const std::string & propertyName);

  void computeAgesFromAllSnapShots(const double depositionTimeTopSurface);
  void setTopSurface(const std::string & topSurfaceName);
  std::vector<double> agesAll() const;

private:
  void reloadModel();
  void clearTable();
  void saveModel();
  void requestPropertyInSnapshots();
  void setInTable();

  const std::string m_projectFileName;
  const DataAccess::Interface::ProjectHandle & m_projectHandle;
  const double m_xCoord;
  const double m_yCoord;
  std::unique_ptr<mbapi::Model> m_mdl;
  int m_posData;
  double m_posDataPrevious;

  std::unordered_map<std::string, std::pair<int, int>> m_propertyPosMap;

  std::string m_propertyName;
  std::vector<double> m_agesAll;
  std::string m_topSurfaceName;
  std::string m_formationName;
};

} // namespace genex0d
