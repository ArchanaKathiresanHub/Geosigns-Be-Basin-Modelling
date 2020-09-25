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
#include "SourceRockDefaultProperties.h"

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
#include "Formation.h"
#include "Interface.h"
#include "Snapshot.h"
#include "Surface.h"

// Utilities
#include "LogHandler.h"

namespace genex0d
{

namespace
{

double interpolateSnapshotProperty(const double inPropertyStart, const double inPropertyEnd,
                                   const double tPrevious, const double t, const double deltaTInterval)
{
  return inPropertyStart + (inPropertyEnd - inPropertyStart)/(-deltaTInterval) * (t - tPrevious);
}

} // namespace

Genex0dGenexSourceRock::Genex0dGenexSourceRock (DataAccess::Interface::ProjectHandle & projectHandle,
                                                const Genex0dInputData & inData,
                                                const unsigned int indI,
                                                const unsigned int indJ) :
  Genex0dSourceRock{projectHandle, inData},
  Genex6::GenexBaseSourceRock{},
  m_sourceRockNode{nullptr},
  m_thickness{0.0},
  m_indI{indI},
  m_indJ{indJ},
  m_inTimes{},
  m_inTemperatures{},
  m_inVesAll{},
  m_inVRE{},
  m_inPorePressure{},
  m_pointAdsorptionHistory{new Genex0dPointAdsorptionHistory(projectHandle, inData)}
{
}

Genex0dGenexSourceRock::~Genex0dGenexSourceRock()
{
}

void Genex0dGenexSourceRock::initializeComputations(const double thickness, const double inorganicDensity, const std::vector<double> & time,
                                                    const std::vector<double> & temperature, const std::vector<double> & Ves, const std::vector<double> &VRE, const std::vector<double> &porePressure)
{
  m_inTimes = time;
  m_inTemperatures = temperature;
  m_inVesAll = Ves;
  m_inVRE = VRE;
  m_inPorePressure = porePressure;
  for (double& porepressure : m_inPorePressure)
  {
    porepressure *= Utilities::Maths::MegaPaToPa;
  }

  m_theSimulator = new Genex6::Simulator();

  m_theChemicalModel = m_theSimulator->loadChemicalModel(getGenexEnvironment(m_srProperties.SCVRe05()), getRunType(m_srProperties.SCVRe05()),
                                                         m_srProperties.typeNameID(), m_srProperties.HCVRe05(), m_srProperties.SCVRe05(),
                                                         m_srProperties.activationEnergy(), m_srProperties.Vr(),
                                                         m_srProperties.AsphalteneDiffusionEnergy(), m_srProperties.ResinDiffusionEnergy(),
                                                         m_srProperties.C15AroDiffusionEnergy(), m_srProperties.C15SatDiffusionEnergy());

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
}

const Genex6::SourceRockNode & Genex0dGenexSourceRock::getSourceRockNode() const
{
  return *m_sourceRockNode;
}

const Genex6::Simulator & Genex0dGenexSourceRock::simulator() const
{
  return *m_theSimulator;
}

bool Genex0dGenexSourceRock::setFormationData( const DataAccess::Interface::Formation * aFormation )
{
  setLayerName( aFormation->getName() );

  if( getLayerName() == "" )
  {
    LogHandler( LogHandler::ERROR_SEVERITY ) << "Cannot compute SourceRock " << getType() << ": the formation name is not set.";
    return false;
  }

  m_formation = aFormation;

  const DataAccess::Interface::Surface * topSurface = m_formation->getTopSurface();
  const DataAccess::Interface::Snapshot * topSurfaceSnapShot = topSurface->getSnapshot();

  if( topSurfaceSnapShot->getTime() == 0 )
  {
    LogHandler( LogHandler::ERROR_SEVERITY ) << "Cannot compute SourceRock with deposition age 0 at : " << m_formation->getName();
    return false;
  }

  m_isSulphur = ( getScVRe05() > 0.0 ? true : false );

  return true;
}

bool Genex0dGenexSourceRock::initialize(const bool printInitialisationDetails)
{
  bool status = true;

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

  std::unique_ptr<DataAccess::Interface::SnapshotList> snapshots(m_projectHandle.getSnapshots(DataAccess::Interface::MINOR | DataAccess::Interface::MAJOR));

  computeSnapshotIntervals(*snapshots);

  if (m_theIntervals.empty())
  {
    throw Genex0dException() << "Failed while preprocessing genex0d simulation!";
  }

  // Extrapolate properties for ending of last snapshot interval
  std::vector<Genex6::SnapshotInterval*>::iterator itSnapInterv = m_theIntervals.end() - 1;
  const DataAccess::Interface::Snapshot * intervalEnd = (*itSnapInterv)->getEnd();
  const DataAccess::Interface::Snapshot * intervalStart = (*itSnapInterv)->getStart();

  itSnapInterv--;
  const DataAccess::Interface::Snapshot * prevIntervalStart = (*itSnapInterv)->getStart();
  const double dt = prevIntervalStart->getTime() - intervalStart->getTime();

  const int iLast = m_inVesAll.size() - 1;
  double ghostP = interpolateSnapshotProperty(m_inVesAll[iLast - 1], m_inVesAll[iLast], prevIntervalStart->getTime(), intervalEnd->getTime(), dt);
  double ghostT = interpolateSnapshotProperty(m_inTemperatures[iLast - 1], m_inTemperatures[iLast], prevIntervalStart->getTime(), intervalEnd->getTime(), dt);

  m_inVesAll.push_back(ghostP);
  m_inTemperatures.push_back(ghostT);
  m_inTimes.push_back(intervalEnd->getTime());

  return true;
}

bool Genex0dGenexSourceRock::addHistoryToNodes()
{
  Genex6::SourceRockAdsorptionHistory * history = new Genex6::SourceRockAdsorptionHistory(m_projectHandle, m_pointAdsorptionHistory);
  Genex6::NodeAdsorptionHistory * adsorptionHistory;

  // GenexHistory (no adsorption implemented yet)
  adsorptionHistory = Genex6::AdsorptionSimulatorFactory::getInstance().allocateNodeAdsorptionHistory(m_theChemicalModel->getSpeciesManager(),
                                                                                                      m_projectHandle,
                                                                                                      Genex6::GenexSimulatorId);

  if (adsorptionHistory == nullptr)
  {
    LogHandler(LogHandler::ERROR_SEVERITY) << "Failed while running Genex0d!";
    delete history;
    return false;
  }
  m_sourceRockNode->addNodeAdsorptionHistory(adsorptionHistory);
  history->setNodeGenexHistory(adsorptionHistory);
  m_sourceRockNodeAdsorptionHistory.push_back(history);
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

  std::vector<Genex6::SnapshotInterval*>::iterator itSnapInterv = m_theIntervals.begin();
  const DataAccess::Interface::Snapshot * intervalStart = (*itSnapInterv)->getStart();;
  const DataAccess::Interface::Snapshot * intervalEnd = (*itSnapInterv)->getEnd();

  int i = 0;
  while (itSnapInterv != m_theIntervals.end())
  {
    intervalStart = (*itSnapInterv)->getStart();
    intervalEnd = (*itSnapInterv)->getEnd();
    const double snapShotIntervalEndTime = intervalEnd->getTime();
    const double numberOfTimeSteps = std::ceil((intervalStart->getTime() - intervalEnd->getTime())/dt);

    if (numberOfTimeSteps <= 0)
    {
      Genex0dException() << "Genex0d failed while computing properties!";
    }

    const double deltaTInterval = intervalStart->getTime() - intervalEnd->getTime();
    const double deltaT = deltaTInterval / numberOfTimeSteps;

    if (m_inTimes[i] != intervalStart->getTime() || m_inTimes[i+1] != intervalEnd->getTime())
    {
      Genex0dException() << "Genex0d failed while processing, incorrect PT history!";
    }

    // Processing pressure and temperature at interval start of the first interval
    computePTSnapShot(intervalStart->getTime(), m_inVesAll[i], m_inTemperatures[i], m_inVRE[i], m_inPorePressure[i]);

    double tPrevious = intervalStart->getTime();
    double t = tPrevious - deltaT;

    // Interpolate interval time instances
    while (t > snapShotIntervalEndTime)
    {
      const double VesInterp = interpolateSnapshotProperty(m_inVesAll[i], m_inVesAll[i+1], tPrevious, t, deltaTInterval);
      const double temperatureInterp = interpolateSnapshotProperty(m_inTemperatures[i], m_inTemperatures[i+1], tPrevious, t, deltaTInterval);
      const double VreInterp = interpolateSnapshotProperty(m_inVRE[i], m_inVRE[i+1], tPrevious, t, deltaTInterval);
      const double porePressureInterp = interpolateSnapshotProperty(m_inPorePressure[i], m_inPorePressure[i+1], tPrevious, t, deltaTInterval);

      computePTSnapShot(t, VesInterp, temperatureInterp, VreInterp, porePressureInterp);

      t -= deltaT;

      if (t - Genex6::Constants::TimeStepFraction * deltaT < snapShotIntervalEndTime)
      {
        t = snapShotIntervalEndTime;
      }
    }

    ++itSnapInterv;
    ++i;
  }



  // Set the interval end for the current interval (doesn't need interpolation)
  computePTSnapShot(intervalEnd->getTime(), m_inVesAll[i], m_inTemperatures[i], m_inVRE[i], m_inPorePressure[i]);

  clearSimulatorBase();

  if (status)
  {
    LogHandler(LogHandler::INFO_SEVERITY) << "-------------------------------------";
    LogHandler(LogHandler::INFO_SEVERITY) << "End of processing.";
    LogHandler(LogHandler::INFO_SEVERITY) << "-------------------------------------";
  }

  saveSourceRockNodeAdsorptionHistory();

  return true;
}

bool Genex0dGenexSourceRock::computePTSnapShot(const double time, double inPressure, const double inTemperature, const double inVre, const double inPorePressure)
{
  LogHandler( LogHandler::INFO_SEVERITY ) << "Computing time instance t:" << time;

  // Check if Ves > maxVes
  if (isVESMaxEnabled() && inPressure > getVESMax())
  {
      inPressure = getVESMax();
  }

  Genex6::Input * theInput = new Genex6::Input(time, inTemperature, inPressure, inVre, inPorePressure);
  m_sourceRockNode->AddInput(theInput);
  m_theSimulator->setChemicalModel(m_theChemicalModel);

  bool isInitialTimeStep =  m_sourceRockNode->RequestComputation(0, *m_theSimulator);
  if (!isInitialTimeStep)
  {
    m_sourceRockNode->collectHistory();
  }
  m_sourceRockNode->ClearInputHistory();

  return true;
}

} // namespace genex0d
