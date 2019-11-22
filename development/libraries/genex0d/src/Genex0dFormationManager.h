//
// Copyright (C) 2012-2017 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// Genex0dFormationManager class: handles any functionality related to geological formation.
#pragma once

#include <memory>

namespace DataAccess
{
namespace Interface
{
class ProjectHandle;
class Formation;
class LithoType;
class GridMap;
class ObjectFactory;
} // namespace Interface
} // namespace DataAccess

namespace genex0d
{

class Genex0dFormationManager
{
public:
  explicit Genex0dFormationManager(const std::string & projectFilename, const std::string & formationName, const double x, const double y);
  ~Genex0dFormationManager();

  double getInorganicDensity();
  double getThickness() const;
  std::string getTopSurfaceName() const;
  bool isFormationSourceRock() const;

private:
  void cleanup();
  void setProperties(const double x, const double y);
  double getLithoDensity(const DataAccess::Interface::LithoType * theLitho);
  void calculateInorganicDensityOfLithoType(const DataAccess::Interface::LithoType * litho1,
                                            const DataAccess::Interface::GridMap * litho1PercentageMap,
                                            double & inorganicDensity);
  void setThickness();

  DataAccess::Interface::ObjectFactory * m_factory;
  std::unique_ptr<DataAccess::Interface::ProjectHandle> m_projectHandle;
  const DataAccess::Interface::Formation * m_formation;
  double m_thickness;
  unsigned int m_indI;
  unsigned int m_indJ;
};

} // namespace genex0d
