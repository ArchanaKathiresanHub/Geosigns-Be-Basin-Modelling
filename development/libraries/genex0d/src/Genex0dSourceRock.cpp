//
// Copyright (C) 2012-2017 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "Genex0dSourceRock.h"

#include "CommonDefinitions.h"
#include "Genex0dFormationManager.h"
#include "Genex0dProjectManager.h"
#include "Genex0dSourceRockDefaultProperties.h"

// AbstractDerivedProperties
#include "SurfaceProperty.h"

// DataModel
#include "AbstractProperty.h"

// Genex6
#include "ConstantsGenex.h"
#include "Simulator.h"

// Genex6_kernel
#include "AdsorptionSimulatorFactory.h"
#include "SourceRockNode.h"

#include "Interface.h"
#include "Snapshot.h"

#include "LogHandler.h"

namespace genex0d
{

Genex0dSourceRock::Genex0dSourceRock(const std::string & sourceRockType,
                                     Genex0dProjectManager & projectMgr,
                                     const DataAccess::Interface::Formation * formation) :
  Genex6::GenexSourceRock{projectMgr.projectHandle(), nullptr},
  m_projectMgr{projectMgr},
  m_formation{formation},
  m_sourceRockNode{nullptr},
  m_simulator{nullptr},
  m_sourceRockType{sourceRockType},
  m_srProperties{Genex0dSourceRockDefaultProperties::getInstance().getProperties(sourceRockType)},
  m_thickness{0.0},
  m_genexHistory{nullptr},
  m_propertyManager{nullptr}
{
  m_propertyManager = new DerivedProperties::DerivedPropertyManager(m_projectMgr.projectHandle());
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

void Genex0dSourceRock::initialize()
{
  m_simulator.reset(new Genex6::Simulator(getGenexEnvironment(), getRunType(),
                                          m_srProperties.typeNameID(), m_srProperties.HCVRe05(), m_srProperties.SCVRe05(),
                                          m_srProperties.activationEnergy(), m_srProperties.Vr(),
                                          m_srProperties.AsphalteneDiffusionEnergy(), m_srProperties.ResinDiffusionEnergy(),
                                          m_srProperties.C15AroDiffusionEnergy(), m_srProperties.C15SatDiffusionEnergy()));

  if (!m_simulator->Validate())
  {
    throw Genex0dException() << "Validation of Genex simulator failed!";
  }
}

void Genex0dSourceRock::computeData(const double thickness, const double inorganicDensity, const std::vector<double> & time,
                                    const std::vector<double> & temperature, const std::vector<double> & pressure)
{
  initialize();
  //  addHistoryToSourceRockNode();


  // Note: the last two arguments are not relevant (set to default values) and are not used.
  m_sourceRockNode.reset(new Genex6::SourceRockNode(thickness, m_srProperties.TocIni(), inorganicDensity, 1.0, 0.0));
  m_sourceRockNode->CreateInputPTHistory(time, temperature, pressure);
  m_sourceRockNode->RequestComputation(*m_simulator);
}


//void Genex0dSourceRock::addHistoryToSourceRockNode()
//{
//  const Genex6::ChemicalModel & chemModel = m_simulator->getChemicalModel();
//  DataAccess::Interface::ProjectHandle * projectHandle = m_projectMgr.projectHandle();

//  m_genexHistory.reset(Genex6::AdsorptionSimulatorFactory::getInstance().allocateNodeAdsorptionHistory(
//                         chemModel.getSpeciesManager(),
//                         projectHandle,
//                         "GenexSimulator"));

//  if ( m_genexHistory ==  nullptr)
//  {
//    throw Genex0dException() << "Fatal error, node adsorption history could not be initiated!";
//  }
//}

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
