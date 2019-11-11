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
#include "NodeAdsorptionHistory.h"

#include "GenexSourceRock.h"

#include <memory>
#include <string>
#include <vector>

namespace DataAccess
{
namespace Interface
{
class ProjectHandle;
} // namespace Interface
} // namespace DataAccess

namespace database
{
class Record;
class Table;
} // namespace database

namespace DerivedProperties
{
class DerivedPropertyManager;
} // namespace DerivedProperties

namespace Genex6
{
class Simulator;
class SnapshotInterval;
class SourceRockNode;
} // namespace Genex6

namespace genex0d
{

class Genex0dFormationManager;
class Genex0dProjectManager;

class Genex0dSourceRock : public Genex6::GenexSourceRock
{
public:
  explicit Genex0dSourceRock(const std::string & sourceRockType,
                             Genex0dProjectManager & projectMgr,
                             const DataAccess::Interface::Formation * formation);
  ~Genex0dSourceRock();

  void computeData(const double thickness, const double inorganicDensity, const std::vector<double> & time,
                   const std::vector<double> & temperature, const std::vector<double> & pressure);

  void setToCIni(const double TOC);
  void setSCVRe05(const double SCVRe05);
  void setHCVRe05(const double HCVRe05);
  void setActivationEnergy(const double activationEnergyScaled);

  Genex6::SourceRockNode & getSourceRockNode();
  const Genex6::SourceRockNode & getSourceRockNode() const;
  const Genex6::Simulator & simulator() const;

  Genex0dSourceRockProperty srProperties() const;

private:
  char * getGenexEnvironment() const;
  int getRunType() const;
  void initialize();
  void addHistoryToSourceRockNode();




  bool process();
  void computeSnapShot(const double previousTime,
                       const DataAccess::Interface::Snapshot *theSnapshot);




  Genex0dProjectManager & m_projectMgr;
  const DataAccess::Interface::Formation * m_formation;

  std::unique_ptr<Genex6::SourceRockNode> m_sourceRockNode;
  std::unique_ptr<Genex6::Simulator> m_simulator;
  std::string m_sourceRockType;
  Genex0dSourceRockProperty m_srProperties;
  double m_thickness;

  std::unique_ptr<Genex6::NodeAdsorptionHistory> m_genexHistory;
  DerivedProperties::DerivedPropertyManager * m_propertyManager;
};

} // namespace genex0d
