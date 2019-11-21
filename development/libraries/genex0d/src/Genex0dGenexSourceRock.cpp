//
// Copyright (C) 2012-2017 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "Genex0dGenexSourceRock.h"

#include "CommonDefinitions.h"
#include "Genex0dPointAdsorptionHistory.h"
#include "Genex0dSourceRockDefaultProperties.h"

// Genex6
#include "ConstantsGenex.h"
#include "Simulator.h"
#include "SourceRockNode.h"

// Genex6_kernel
#include "AdsorptionSimulatorFactory.h"
#include "GenexHistory.h"
#include "SnapshotInterval.h"
#include "SourceRockAdsorptionHistory.h"

// DataAccess
#include "Interface.h"
#include "Snapshot.h"

// Utilities
#include "LogHandler.h"

namespace genex0d
{

namespace
{

double interpolateSnapshotProperty(const double inPropertyStart, const double inPropertyEnd,
                                   const double t, const double tPrevious, const double deltaT)
{
  const double gradient = (inPropertyEnd - inPropertyStart)/deltaT;
  return gradient * (t - tPrevious);
}

} // namespace

Genex0dGenexSourceRock::Genex0dGenexSourceRock (DataAccess::Interface::ProjectHandle * projectHandle,
                                                const Genex0dInputData & inData,
                                                const unsigned int indI,
                                                const unsigned int indJ) :
  Genex0dSourceRock{projectHandle, inData},
  m_sourceRockNode{nullptr},
  m_thickness{0.0},
  m_indI{indI},
  m_indJ{indJ},
  m_inTimes{},
  m_inTemperatures{},
  m_inPressures{},
  m_pointAdsorptionHistory{new Genex0dPointAdsorptionHistory(projectHandle, inData)}
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
  m_inTimes = time;
  m_inTemperatures = temperature;
  m_inPressures = pressure;

  m_theSimulator = new Genex6::Simulator();

//  m_theChemicalModel = m_theChemicalModel1 = m_theChemicalModel2 =
//      m_theSimulator->loadChemicalModel(getGenexEnvironment(), getRunType(),
//                                        m_srProperties.typeNameID(), m_srProperties.HCVRe05(), m_srProperties.SCVRe05(),
//                                        m_srProperties.activationEnergy(), m_srProperties.Vr(),
//                                        m_srProperties.AsphalteneDiffusionEnergy(), m_srProperties.ResinDiffusionEnergy(),
//                                        m_srProperties.C15AroDiffusionEnergy(), m_srProperties.C15SatDiffusionEnergy());


//  if (!m_theSimulator->Validate())
//  {
//    throw Genex0dException() << "Validation of Genex simulator failed!";
//  }

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

  m_theChemicalModel = m_theChemicalModel1 = m_theChemicalModel2 = loadChemicalModel(this, printInitialisationDetails);

  if (m_theSimulator == nullptr)
  {
    throw Genex0dException() << "Initialization of genex0d failed!";
  }

  m_theSimulator->setChemicalModel(m_theChemicalModel);

  if(printInitialisationDetails )
  {
    LogHandler(LogHandler::INFO_SEVERITY) << "Start Of Initialization...";
  }

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
  Genex6::GenexHistory * adsorptionHistory = new Genex6::GenexHistory(m_theChemicalModel->getSpeciesManager(), m_projectHandle);

  if (adsorptionHistory == nullptr)
  {
    LogHandler(LogHandler::ERROR_SEVERITY) << "Failed while running Genex0d!";
    return false;
  }
  m_sourceRockNode->addNodeAdsorptionHistory(adsorptionHistory);
  delete adsorptionHistory;
  return true;
}

bool Genex0dGenexSourceRock::process()
{
  bool status = true;
  double dt = m_theSimulator->GetMaximumTimeStepSize(m_depositionTime);

  LogHandler(LogHandler::INFO_SEVERITY) << "Chosen maximum timestep size:" << dt;
  LogHandler(LogHandler::INFO_SEVERITY) << "-------------------------------------";

  LogHandler(LogHandler::INFO_SEVERITY) << "Start Of processing...";
  LogHandler(LogHandler::INFO_SEVERITY) << "-------------------------------------";

  m_runtime = 0.0;
  m_time = 0.0;

  if (!status)
  {
    return status;
  }

  std::vector<Genex6::SnapshotInterval*>::iterator itSnapInterv;

  int i = 0;
  for (itSnapInterv = m_theIntervals.begin(); itSnapInterv != m_theIntervals.end(); ++ itSnapInterv)
  {
    const DataAccess::Interface::Snapshot * intervalStart = (*itSnapInterv)->getStart();;
    const DataAccess::Interface::Snapshot * intervalEnd = (*itSnapInterv)->getStart();;

    double numberOfTimeSteps = std::ceil((intervalStart->getTime() - intervalEnd->getTime())/dt);
    double deltaT = (intervalStart->getTime() - intervalEnd->getTime()) / numberOfTimeSteps;

    if (m_inTimes[i] != intervalStart->getTime() || m_inTimes[i+1] != intervalEnd->getTime())
    {
      Genex0dException() << "Incorrect PT history provided!";
    }

    // Note: the pressure and temperature at interval start and end are already defined!
    Genex6::Input * theInput = new Genex6::Input(intervalStart->getTime(), m_inPressures[i], m_inTemperatures[i]);
    m_sourceRockNode->AddInput(theInput);

    double t = intervalStart->getTime() + deltaT;
    double tPrevious = t;
    while (t < intervalEnd->getTime())
    {
      const double PressureInterp = interpolateSnapshotProperty(m_inPressures[i], m_inPressures[i+1], t, tPrevious, deltaT);
      const double TemperatureInterp = interpolateSnapshotProperty(m_inTemperatures[i], m_inTemperatures[i+1], t, tPrevious, deltaT);

      tPrevious = t;
      t = tPrevious + deltaT;
    }



    ++i;
  }

  m_sourceRockNode->RequestComputation(*m_theSimulator);

  return true;
}

bool Genex0dGenexSourceRock::computeSnapShot(const double previousTime,
                                             const DataAccess::Interface::Snapshot *theSnapshot)
{
  bool status = true;
  double time = theSnapshot->getTime();
  LogHandler( LogHandler::INFO_SEVERITY ) << "Computing SnapShot t:" << time;


  return true;
}


} // namespace genex0d
