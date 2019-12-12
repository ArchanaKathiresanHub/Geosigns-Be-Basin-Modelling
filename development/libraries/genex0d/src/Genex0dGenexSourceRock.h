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

#include "GenexBaseSourceRock.h"

#include <memory>
#include <string>
#include <vector>

namespace Genex6
{
class Simulator;
class SourceRockNode;
}

namespace database
{
class Record;
class Table;
}

namespace DataAccess
{
namespace Interface
{
class ProjectHandle;
}
}

namespace genex0d
{

class Genex0dPointAdsorptionHistory;

class Genex0dGenexSourceRock : public Genex0dSourceRock, public Genex6::GenexBaseSourceRock
{
public:
  explicit Genex0dGenexSourceRock(DataAccess::Interface::ProjectHandle & projectHandle,
                                  const Genex0dInputData & inData,
                                  const unsigned int indI,
                                  const unsigned int indJ);
  ~Genex0dGenexSourceRock();

  void initializeComputations(const double thickness, const double inorganicDensity, const std::vector<double> & time,
                              const std::vector<double> & temperature, const std::vector<double> & Ves);

  const Genex6::SourceRockNode & getSourceRockNode() const;
  const Genex6::Simulator & simulator() const;

  bool initialize(const bool printInitialisationDetails = true) final;
  bool addHistoryToNodes() final;
  bool setFormationData (const DataAccess::Interface::Formation * aFormation) final;

protected:
  bool preprocess() final;
  bool process() final;

private:
  bool computePTSnapShot(const double time, const double inPressure, const double inTemperature);

  std::unique_ptr<Genex6::SourceRockNode> m_sourceRockNode;
  double m_thickness;
  const unsigned int m_indI;
  const unsigned int m_indJ;

  std::vector<double> m_inTimes;
  std::vector<double> m_inTemperatures;
  std::vector<double> m_inVesAll;
  Genex0dPointAdsorptionHistory * m_pointAdsorptionHistory;
};

} // namespace genex0d
