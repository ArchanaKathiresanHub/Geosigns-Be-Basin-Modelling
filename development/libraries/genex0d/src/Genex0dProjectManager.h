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

namespace genex0d
{

class Genex0dProjectManager
{ 
public:
  explicit Genex0dProjectManager(const std::string & projectFileName, const double xCoord, const double yCoord);
  ~Genex0dProjectManager();

  std::vector<double> requestPropertyHistory(const std::string & propertyName);

  DataAccess::Interface::ProjectHandle * projectHandle();
  void computeAgesFromAllSnapShots(const double depositionTimeTopSurface);
  void setTopSurface(const std::string & topSurfaceName);
  std::vector<double> agesAll() const;

private:
  void cleanup();
  void reloadModel();
  void clearTable();
  void saveModel();
  void getValues(std::vector<double> & values) const;
  void requestPropertyInSnapshots();
  void setInTable();

  const std::string m_projectFileName;
  DataAccess::Interface::ObjectFactory * m_ObjectFactory;
  DataAccess::Interface::ProjectHandle * m_projectHandle;

  const double m_xCoord;
  const double m_yCoord;
  std::unique_ptr<mbapi::Model> m_mdl;
  int m_posData;
  double m_posDataPrevious;
  std::string m_propertyName;
  std::vector<double> m_agesAll;
  std::string m_topSurfaceName;
};

} // namespace genex0d
