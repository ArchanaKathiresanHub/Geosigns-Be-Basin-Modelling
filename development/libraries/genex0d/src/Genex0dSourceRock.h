//
// Copyright (C) 2013-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// Genex0dSourceRock class gets source rock (SR) input data, and computes SR properties.

#pragma once

#include "Genex0dSourceRockProperty.h"

#include <memory>
#include <string>
#include <vector>

namespace Genex6
{
class Simulator;
class SourceRockNode;
} // namespace Genex6

namespace database
{
class Record;
class Table;
} // namespace database

namespace DataAccess
{
namespace Interface
{
class ProjectHandle;
} // namespace Interface
} // namespace DataAccess

namespace genex0d
{

class Genex0dSourceRock
{
public:
  explicit Genex0dSourceRock(const std::string & sourceRockType);
  ~Genex0dSourceRock();

  void computeData(const double thickness, const double inorganicDensity, const std::vector<double> & time,
                   const std::vector<double> & temperature, const std::vector<double> & pressure);

  void setToCIni(const double TOC);
  void setSCVRe05(const double SCVRe05);
  void setHCVRe05(const double HCVRe05);
  void setActivationEnergy(const double activationEnergyScaled);

  const Genex6::SourceRockNode & getSourceRockNode() const;
  const Genex6::Simulator & simulator() const;

private:
  char * getGenexEnvironment() const;
  int getRunType() const;

  std::unique_ptr<Genex6::SourceRockNode> m_sourceRockNode;
  std::unique_ptr<Genex6::Simulator> m_simulator;
  std::string m_sourceRockType;
  Genex0dSourceRockProperty m_srProperties;
  double m_thickness;
};

} // namespace genex0d
