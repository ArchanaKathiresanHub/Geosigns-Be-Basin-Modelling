//
// Copyright (C) 2012-2017 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "Genex0dGenexSourceRock.h"

#include "CommonDefinitions.h"
#include "Genex0dSourceRockDefaultProperties.h"

// Genex6
#include "ConstantsGenex.h"
#include "Simulator.h"
#include "SourceRockNode.h"

// Genex6_kernel
#include "AdsorptionSimulatorFactory.h"
#include "SourceRockAdsorptionHistory.h"

// DataAccess
#include "Interface.h"

// Utilities
#include "LogHandler.h"

namespace genex0d
{

Genex0dGenexSourceRock::Genex0dGenexSourceRock (DataAccess::Interface::ProjectHandle * projectHandle,
                                                const Genex0dInputData & inData,
                                                const unsigned int indI,
                                                const unsigned int indJ) :
  Genex0dSourceRock{projectHandle, inData},
  m_sourceRockNode{nullptr},
  m_thickness{0.0},
  m_indI{indI},
  m_indJ{indJ}
{
}

Genex0dGenexSourceRock::~Genex0dGenexSourceRock()
{
}

int Genex0dGenexSourceRock::getRunType() const
{
  return (m_srProperties.SCVRe05() != 0.0 ? Genex6::Constants::SIMGENEX : (Genex6::Constants::SIMGENEX | Genex6::Constants::SIMGENEX5));
}

char * Genex0dGenexSourceRock::getGenexEnvironment() const
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

void Genex0dGenexSourceRock::initializeComputations(const double thickness, const double inorganicDensity, const std::vector<double> & time,
                                                    const std::vector<double> & temperature, const std::vector<double> & pressure)
{
  m_theSimulator.reset(new Genex6::Simulator(getGenexEnvironment(), getRunType(),
                                             m_srProperties.typeNameID(), m_srProperties.HCVRe05(), m_srProperties.SCVRe05(),
                                             m_srProperties.activationEnergy(), m_srProperties.Vr(),
                                             m_srProperties.AsphalteneDiffusionEnergy(), m_srProperties.ResinDiffusionEnergy(),
                                             m_srProperties.C15AroDiffusionEnergy(), m_srProperties.C15SatDiffusionEnergy()));

  if (!m_theSimulator->Validate())
  {
    throw Genex0dException() << "Validation of Genex simulator failed!";
  }

  // Note: the last two arguments are not relevant (set to default values) and are not used.
  m_sourceRockNode.reset(new Genex6::SourceRockNode(thickness, m_srProperties.TocIni(), inorganicDensity, 1.0, 0.0));

  if (m_sourceRockNode == nullptr)
  {
    throw Genex0dException() << "Initialization of genex0d failed!";
  }

  //  m_sourceRockNode->CreateInputPTHistory(time, temperature, pressure);
  //  m_sourceRockNode->RequestComputation(*m_theSimulator);
}

const Genex6::SourceRockNode & Genex0dGenexSourceRock::getSourceRockNode() const
{
  return *m_sourceRockNode;
}

const Genex6::Simulator & Genex0dGenexSourceRock::simulator() const
{
  return *m_theSimulator;
}

// --------------------------------------------------------------------------------

bool Genex0dGenexSourceRock::initialize(const bool printInitialisationDetails)
{
  bool status = true;

  if(printInitialisationDetails )
  {
    LogHandler(LogHandler::INFO_SEVERITY) << "Start Of Initialization...";
  }

  if (m_theSimulator == nullptr)
  {
    throw Genex0dException() << "Initialization of genex0d failed!";
  }

  m_theChemicalModel = m_theSimulator->getChemicalModel();

  if (doApplyAdsorption())
  {
    throw Genex0dException() << "genex0d does not perform adsorption simulation yet!";
  }

  double maximumTimeStepSize = m_theSimulator->getMaximumTimeStepSize();
  int numberOfTimeSteps = m_theSimulator->getNumberOfTimesteps();
  m_theSimulator->setMaximumTimeStepSize(maximumTimeStepSize);
  m_theSimulator->setNumberOfTimesteps(numberOfTimeSteps);

  if (status)
  {
    status =  m_theChemicalModel->Validate();

    if (printInitialisationDetails)
    {
      if(status)
      {
        LogHandler( LogHandler::INFO_SEVERITY ) << "End Of Initialization.";
        LogHandler( LogHandler::INFO_SEVERITY ) << "-------------------------------------" ;
      }
      else
      {
        LogHandler( LogHandler::ERROR_SEVERITY ) << "Invalid Chemical Model. Please check your source rock input parameters. Aborting...";
        LogHandler( LogHandler::INFO_SEVERITY )  << "----------------------------------------------------------------------------------";
      }
    }
  }

  return status;
}

bool Genex0dGenexSourceRock::preprocess()
{
  LogHandler(LogHandler::INFO_SEVERITY) << "Start of preprocessing...";

  if (m_sourceRockNode == nullptr)
  {
    return false;
  }

  computeSnapshotIntervals();
  return true;
}

bool Genex0dGenexSourceRock::addHistoryToNodes()
{
  Genex6::NodeAdsorptionHistory * adsorptionHistory =
      dynamic_cast<Genex6::NodeAdsorptionHistory *>(Genex6::GenexHistory(m_theChemicalModel->getSpeciesManager(), m_projectHandle));

  if ( adsorptionHistory != 0 ) {
    // Add the node-adsorption-history object to the sr-history-object.
    history->setNodeGenexHistory ( adsorptionHistory );
    m_sourceRockNode->addNodeAdsorptionHistory ( adsorptionHistory );
    m_sourceRockNodeAdsorptionHistory.push_back ( history );
  } else {
    delete history;
  }

}


} // namespace genex0d
