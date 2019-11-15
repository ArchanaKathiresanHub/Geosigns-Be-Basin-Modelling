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
class Grid;
class ProjectHandle;
} // namespace Interface
} // namespace DataAccess

namespace genex0d
{

class Genex0dFormationManager
{
public:
  explicit Genex0dFormationManager(DataAccess::Interface::ProjectHandle * projectHandle, const std::string & formationName);

  void setProperties(const double x, const double y);
  bool isFormationSourceRock() const;
  const DataAccess::Interface::Formation * formation() const;
  unsigned int indI() const;
  unsigned int indJ() const;

private:
  DataAccess::Interface::ProjectHandle * m_projectHandle;
  const DataAccess::Interface::Formation * m_formation;
  const DataAccess::Interface::Grid * m_gridLowResolution;
  unsigned int m_indI;
  unsigned int m_indJ;
};

} // namespace genex0d
