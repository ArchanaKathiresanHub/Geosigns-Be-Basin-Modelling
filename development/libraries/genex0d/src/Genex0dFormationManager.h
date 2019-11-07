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
class Formation;
class LithoType;
class GridMap;
class ProjectHandle;
} // namespace Interface
} // namespace DataAccess

namespace genex0d
{

class Genex0dFormationManager
{
public:
  explicit Genex0dFormationManager(DataAccess::Interface::ProjectHandle * projectHandle, const std::string & formationName, const double x, const double y);

  double getInorganicDensity();
  double getThickness() const;
  std::string topSurfaceName() const;
  bool isFormationSourceRock() const;

  double depositionTimeTopSurface() const;

private:
  void setProperties(const double x, const double y);
  double getLithoDensity(const DataAccess::Interface::LithoType * theLitho);
  void calculateInorganicDensityOfLithoType(const DataAccess::Interface::LithoType * litho1,
                                            const DataAccess::Interface::GridMap * litho1PercentageMap,
                                            double & inorganicDensity);
  void setThickness();

  DataAccess::Interface::ProjectHandle * m_projectHandle;
  const DataAccess::Interface::Formation * m_formation;
  double m_thickness;
  unsigned int m_indI;
  unsigned int m_indJ;
};

} // namespace genex0d
