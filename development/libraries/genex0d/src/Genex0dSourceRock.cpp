//
// Copyright (C) 2012-2017 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "Genex0dSourceRock.h"

#include "CommonDefinitions.h"
#include "Genex0dSourceRockDefaultProperties.h"

#include "ConstantsGenex.h"
#include "Simulator.h"
#include "SourceRockNode.h"

#include "Interface.h"

namespace genex0d
{

Genex0dSourceRock::Genex0dSourceRock (const std::string & sourceRockType) :
  m_sourceRockNode{nullptr},
  m_simulator{nullptr},
  m_sourceRockType{sourceRockType},
  m_srProperties{Genex0dSourceRockDefaultProperties::getInstance().getProperties(sourceRockType)},
  m_thickness{0.0}
{
}

Genex0dSourceRock::~Genex0dSourceRock()
{
}

void Genex0dSourceRock::setToCIni(const double TOC)
{
  m_srProperties.setTocIni(TOC);
}

void Genex0dSourceRock::setSCVRe05(const double SCVRe05)
{
  m_srProperties.setSCVRe05(SCVRe05);
}

void Genex0dSourceRock::setHCVRe05(const double HCVRe05)
{
  m_srProperties.setHCVRe05(HCVRe05);
}

void Genex0dSourceRock::setActivationEnergy(const double activationEnergyScaled)
{
  // multiply by 1000.0 to convert from  KJ (front-end format) to J.
  m_srProperties.setActivationEnergy(activationEnergyScaled * 1000.0);
}

int Genex0dSourceRock::getRunType() const
{
  return (m_srProperties.SCVRe05() != 0.0 ? Genex6::Constants::SIMGENEX : (Genex6::Constants::SIMGENEX | Genex6::Constants::SIMGENEX5));
}

char * Genex0dSourceRock::getGenexEnvironment() const
{
  if (getRunType() & Genex6::Constants::SIMGENEX5)
  {
    return getenv("GENEX5DIR");
  }
  else
  {
    return getenv("GENEX6DIR");
  }

  return nullptr;
}

void Genex0dSourceRock::computeData(const double thickness, const double inorganicDensity, const std::vector<double> & time,
                                    const std::vector<double> & temperature, const std::vector<double> & pressure)
{
  // Note: the last two arguments are not relevant (set to default values) and are not used.
  m_sourceRockNode.reset(new Genex6::SourceRockNode(thickness, m_srProperties.TocIni(), inorganicDensity, 1.0, 0.0));
  m_sourceRockNode->CreateInputPTHistory(time, temperature, pressure);

  m_simulator.reset(new Genex6::Simulator(getGenexEnvironment(), getRunType(),
                                          m_srProperties.typeNameID(), m_srProperties.HCVRe05(), m_srProperties.SCVRe05(),
                                          m_srProperties.activationEnergy(), m_srProperties.Vr(),
                                          m_srProperties.AsphalteneDiffusionEnergy(), m_srProperties.ResinDiffusionEnergy(),
                                          m_srProperties.C15AroDiffusionEnergy(), m_srProperties.C15SatDiffusionEnergy()));

  if (!m_simulator->Validate())
  {
    throw Genex0dException() << "Validation of Genex simulator failed!";
  }

  m_sourceRockNode->RequestComputation(*m_simulator);
}

Genex6::SourceRockNode & Genex0dSourceRock::getSourceRockNode()
{
  return *m_sourceRockNode;
}

const Genex6::SourceRockNode & Genex0dSourceRock::getSourceRockNode() const
{
  return *m_sourceRockNode;
}

const Genex6::Simulator & Genex0dSourceRock::simulator() const
{
  return *m_simulator;
}

} // namespace genex0d
