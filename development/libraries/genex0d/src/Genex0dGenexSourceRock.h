//
// Copyright (C) 2013-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// Genex0dGenexSourceRock class gets source rock (SR) input data, and computes SR properties.

#pragma once

#include "Genex0dGenexSourceRock.h"
#include "Genex0dSourceRock.h"

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

class Genex0dPointAdsorptionHistory;

class Genex0dGenexSourceRock : public Genex0dSourceRock
{
public:
  explicit Genex0dGenexSourceRock(DataAccess::Interface::ProjectHandle * projectHandle,
                                  const Genex0dInputData & inData,
                                  const unsigned int indI,
                                  const unsigned int indJ);
  ~Genex0dGenexSourceRock();

  void initializeComputations(const double thickness, const double inorganicDensity, const std::vector<double> & time,
                              const std::vector<double> & temperature, const std::vector<double> & pressure);

  const Genex6::SourceRockNode & getSourceRockNode() const;
  const Genex6::Simulator & simulator() const;

  // --------------------

  bool initialize(const bool printInitialisationDetails = true) final;
  bool addHistoryToNodes() final;

protected:
  bool preprocess() final;
  bool process() final;
  bool computeSnapShot(const double previousTime,
                       const DataAccess::Interface::Snapshot *theSnapshot) final;

private:
  char * getGenexEnvironment() const;
  int getRunType() const;

  std::unique_ptr<Genex6::SourceRockNode> m_sourceRockNode;
  double m_thickness;
  const unsigned int m_indI;
  const unsigned int m_indJ;

  std::vector<double> m_inTimes;
  std::vector<double> m_inTemperatures;
  std::vector<double> m_inPressures;
  Genex0dPointAdsorptionHistory * m_pointAdsorptionHistory;
};

} // namespace genex0d
