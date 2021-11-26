//
// Copyright (C) 2012-2017 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "Genex0dSourceRock.h"

#include "CommonDefinitions.h"
#include "Genex0dPointAdsorptionHistory.h"
#include "SourceRockTypeNameMappings.h"

// Genex6
#include "ConstantsGenex.h"
#include "MixingParameters.h"
#include "Simulator.h"
#include "SourceRockNode.h"

// Genex6_kernel
#include "AdsorptionSimulatorFactory.h"
#include "GenexHistory.h"
#include "SnapshotInterval.h"
#include "SourceRockAdsorptionHistory.h"

#include "Interface.h"
#include <memory>

// DataAccess
#include "Formation.h"
#include "Interface.h"
#include "Snapshot.h"
#include "Surface.h"

// Utilities
#include "LogHandler.h"



namespace Genex0d
{

namespace
{

double interpolateSnapshotProperty(const double inPropertyStart, const double inPropertyEnd,
                                   const double tPrevious, const double t, const double deltaTInterval)
{
  return inPropertyStart + (inPropertyEnd - inPropertyStart)/(-deltaTInterval) * (t - tPrevious);
}

} // namespace

Genex0dSourceRock::Genex0dSourceRock(DataAccess::Interface::ProjectHandle& projectHandle,
                                     const Genex0dInputData& inData,
                                     const unsigned int indI,
                                     const unsigned int indJ) :
  GenexBaseSourceRock{projectHandle, nullptr},
  m_formationName{inData.formationName},
  m_sourceRockType{inData.sourceRockType},
  m_vreThreshold{0.5},
  m_vesMax{inData.maxVes * Utilities::Maths::MegaPaToPa},
  m_vesMaxEnabled{inData.maxVesEnabled},
  m_adsorptionCapacityFunctionName{inData.whichAdsorptionFunction},
  m_applyAdsorption{inData.whichAdsorptionSimulator != ""},
  m_adsorptionSimulatorName{inData.whichAdsorptionSimulator},
  m_doOTCG{inData.doOTCG},
  m_sourceRockNode{nullptr},
  m_thickness{0.0},
  m_indI{indI},
  m_indJ{indJ},
  m_inTimes{},
  m_inTemperatures{},
  m_inVesAll{},
  m_inVRE{},
  m_inPorePressure{},
  m_inPorosity{},
  m_inPermeability{},
  m_pointAdsorptionHistory{new Genex0dPointAdsorptionHistory(projectHandle, inData)},
  m_sourceRock2{nullptr},
  m_mixingHI{inData.mixingHI}
{
  if (!inData.sourceRockTypeSR2.empty())
  {
    m_applySRMixing = true;
  }
  setSourceRockPropertiesFromInput(inData);
}

// Constructor for sourcerock 2
Genex0dSourceRock::Genex0dSourceRock(DataAccess::Interface::ProjectHandle& projectHandle,
                                     const DataAccess::Interface::SourceRockProperty& sourceRockProperties,
                                     const Genex0dInputData& inData) :
  GenexBaseSourceRock{projectHandle, nullptr},
  m_formationName{""},
  m_sourceRockType{inData.sourceRockTypeSR2},
  m_vreThreshold{0.5},
  m_vesMax{inData.maxVes},
  m_vesMaxEnabled{inData.maxVesEnabled},
  m_adsorptionCapacityFunctionName{inData.whichAdsorptionFunction},
  m_applyAdsorption{inData.whichAdsorptionSimulator != ""},
  m_adsorptionSimulatorName{inData.whichAdsorptionSimulator},
  m_doOTCG{inData.doOTCG},
  m_sourceRockNode{nullptr},
  m_thickness{0.0},
  m_indI{0},
  m_indJ{0},
  m_inTimes{},
  m_inTemperatures{},
  m_inVesAll{},
  m_inVRE{},
  m_inPorePressure{},
  m_inPorosity{},
  m_inPermeability{},
  m_pointAdsorptionHistory{nullptr},
  m_sourceRock2{nullptr},
  m_mixingHI{CauldronNoDataValue}
{
  m_srProperties = sourceRockProperties;

}


Genex0dSourceRock::~Genex0dSourceRock()
{
  clearBase();
  clearSimulatorBase();
}

void Genex0dSourceRock::setSourceRockPropertiesFromInput(const Genex0dInputData& inData)
{
  m_srProperties.setTypeNameID(extractTypeID(m_sourceRockType));

  m_srProperties.setTocIni(inData.ToCIni);
  m_srProperties.setSCVRe05(inData.SCVRe05);
  m_srProperties.setHCVRe05(inData.HCVRe05);
  m_srProperties.setActivationEnergy(inData.activationEnergy);
  m_srProperties.setAsphalteneDiffusionEnergy(inData.asphalteneDiffusionEnergy);
  m_srProperties.setResinDiffusionEnergy(inData.resinDiffusionEnergy);
  m_srProperties.setC15AroDiffusionEnergy(inData.C15AroDiffusionEnergy);
  m_srProperties.setC15SatDiffusionEnergy(inData.C15SatDiffusionEnergy);

  if (m_applySRMixing)
  {
    DataAccess::Interface::SourceRockProperty srProperties;
    srProperties.setTypeNameID(extractTypeID(inData.sourceRockTypeSR2));
    srProperties.setHCVRe05(inData.HCVRe05SR2);
    srProperties.setSCVRe05(inData.SCVRe05SR2);
    srProperties.setActivationEnergy(inData.activationEnergySR2);
    srProperties.setAsphalteneDiffusionEnergy(inData.asphalteneDiffusionEnergySR2);
    srProperties.setResinDiffusionEnergy(inData.resinDiffusionEnergySR2);
    srProperties.setC15AroDiffusionEnergy(inData.C15AroDiffusionEnergySR2);
    srProperties.setC15SatDiffusionEnergy(inData.C15SatDiffusionEnergySR2);

    m_sourceRock2.reset(new Genex0dSourceRock(m_projectHandle, srProperties, inData));
  }
}

std::string Genex0dSourceRock::extractTypeID(const std::string& sourceRockType) const
{
  std::string typeId = "";
  std::unordered_map<std::string, std::string> typeToIDMap = Genex6::SourceRockTypeNameMappings::getInstance().CfgFileNameBySRType();
  std::unordered_map<std::string, std::string>::const_iterator it = typeToIDMap.find(sourceRockType);

  if(it != typeToIDMap.end())
  {
    typeId = it ->second;
  }

  return typeId;
}

void Genex0dSourceRock::initializeInputs(const double thickness, const double inorganicDensity, const std::vector<double> & time,
                                                    const std::vector<double>& temperature, const std::vector<double>& Ves, const std::vector<double>& VRE,
                                                    const std::vector<double>& porePressure, const std::vector<double>& permeability, const std::vector<double>& porosity)
{
  m_inTimes = time;
  m_inTemperatures = temperature;
  m_inVesAll = Ves;
  m_inVRE = VRE;
  m_inPorePressure = porePressure;
  m_inPermeability = permeability;
  m_inPorosity = porosity;
  m_thickness = thickness;
  m_inorganicDensity = inorganicDensity;

  for (double& porosity : m_inPorosity)
  {
    porosity /= 100; // Convert from percentage to fraction
  }

  for (double& porepressure : m_inPorePressure)
  {
    porepressure *= Utilities::Maths::MegaPaToPa;
  }
}

const Genex6::SourceRockNode & Genex0dSourceRock::getSourceRockNode() const
{
  return *m_sourceRockNode;
}

const Genex6::Simulator & Genex0dSourceRock::simulator() const
{
  return *m_theSimulator;
}

const std::string & Genex0dSourceRock::getLayerName (void) const
{
  return m_formationName;
}

const std::string & Genex0dSourceRock::getType (void) const
{
  return m_sourceRockType;
}

const std::string & Genex0dSourceRock::getTypeID (void) const
{
  return m_srProperties.typeNameID();
}

const double & Genex0dSourceRock::getHcVRe05(void) const
{
  return m_srProperties.HCVRe05();
}

const double & Genex0dSourceRock::getScVRe05(void) const
{
  return m_srProperties.SCVRe05();
}

const double & Genex0dSourceRock::getPreAsphaltStartAct(void) const
{
  return m_srProperties.activationEnergy();
}

const double & Genex0dSourceRock::getAsphalteneDiffusionEnergy(void) const
{
  return m_srProperties.AsphalteneDiffusionEnergy();
}

const double & Genex0dSourceRock::getResinDiffusionEnergy(void) const
{
  return m_srProperties.ResinDiffusionEnergy();
}

const double & Genex0dSourceRock::getC15AroDiffusionEnergy(void) const
{
  return m_srProperties.C15AroDiffusionEnergy();
}

const double & Genex0dSourceRock::getC15SatDiffusionEnergy(void) const
{
  return m_srProperties.C15SatDiffusionEnergy();
}

bool Genex0dSourceRock::isVREoptimEnabled(void) const
{
  return false; // TODO: See if it's necessary to be parsed from input
}

const double & Genex0dSourceRock::getVREthreshold(void) const
{
  return m_vreThreshold;
}

const std::string & Genex0dSourceRock::getBaseSourceRockType (void) const
{
  return m_sourceRockType;
}

bool Genex0dSourceRock::isVESMaxEnabled(void) const
{
  return m_vesMaxEnabled;
}

const double & Genex0dSourceRock::getVESMax(void) const
{
  return m_vesMax;
}

bool Genex0dSourceRock::doApplyAdsorption(void) const
{
  return m_applyAdsorption;
}

bool Genex0dSourceRock::adsorptionIsTOCDependent(void) const
{
  return false; // TODO: See if it's necessary to be parsed from input
}

bool Genex0dSourceRock::doComputeOTGC(void) const
{
  return m_doOTCG;
}

const string & Genex0dSourceRock::getAdsorptionCapacityFunctionName(void) const
{
  return m_adsorptionCapacityFunctionName;
}

const string & Genex0dSourceRock::getAdsorptionSimulatorName(void) const
{
  return m_adsorptionSimulatorName;
}

const DataAccess::Interface::GridMap * Genex0dSourceRock::getMap(DataAccess::Interface::SourceRockMapAttributeId attributeId) const
{
  switch (attributeId)
  {
    case DataAccess::Interface::SourceRockMapAttributeId::TocIni :
      {
        return loadMap(attributeId, m_srProperties.TocIni());
      }
    case DataAccess::Interface::SourceRockMapAttributeId::HcIni :
      {
        return loadMap(attributeId, m_srProperties.HCVRe05());
      }
  }

  return nullptr;
}

const DataAccess::Interface::GridMap * Genex0dSourceRock::loadMap(DataAccess::Interface::SourceRockMapAttributeId attributeId, const double mapScalarValue) const
{
  const  DataAccess::Interface::Grid * grid = m_projectHandle.getActivityOutputGrid();
  if (!grid)
  {
    grid = m_projectHandle.getInputGrid();
  }
  return m_projectHandle.getFactory()->produceGridMap (this, attributeId, grid, mapScalarValue);
}

bool Genex0dSourceRock::setFormationData( const DataAccess::Interface::Formation * aFormation )
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

  m_isSulphur = getScVRe05() > 0.0;

  return true;
}

bool Genex0dSourceRock::initialize(const bool printInitialisationDetails)
{
  bool status = true;
  double fractionSourceRock1 = 1.0;
  double fractionSourceRock2 = 0.0;

  initializeSimulator(printInitialisationDetails);

  if (m_theSimulator != nullptr && m_applySRMixing)
  {
    Genex6::MixingParameters mixParams;
    initializeSourceRock2(mixParams, *m_sourceRock2, printInitialisationDetails);
    double hcValueMixing = convertHItoHC(m_mixingHI);
    applySRMixing(mixParams, hcValueMixing, printInitialisationDetails);

    fractionSourceRock1 = ( hcValueMixing - mixParams.hcValue2 ) / ( mixParams.hcValue1 - mixParams.hcValue2 );
    fractionSourceRock2 = 1.0 - fractionSourceRock1;
  }

  if ( status && doApplyAdsorption())
  {
    initializeAdsorptionModel(printInitialisationDetails, status, false, getAdsorptionCapacityFunctionName(), getAdsorptionSimulatorName(), doComputeOTGC(), getProjectHandle());
  }
  if (m_applySRMixing && status && m_sourceRock2->doApplyAdsorption())
  {
    initializeAdsorptionModelSR2(*m_sourceRock2, status, printInitialisationDetails, m_projectHandle);
  }

  if( status && m_applySRMixing)
  {
    validateChemicalModel2(printInitialisationDetails, status);
  }

  if (status)
  {
    validateChemicalModel1(printInitialisationDetails, status);
  }

  m_sourceRockNode.reset(new Genex6::SourceRockNode(m_thickness, m_srProperties.TocIni(), m_inorganicDensity, fractionSourceRock1, fractionSourceRock2));

  if (m_sourceRockNode == nullptr)
  {
    throw Genex0dException() << "Initialization of genex0d failed!";
  }

  return status;
}

bool Genex0dSourceRock::preprocess()
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
  const DataAccess::Interface::Snapshot* intervalEnd = (*itSnapInterv)->getEnd();
  const DataAccess::Interface::Snapshot* intervalStart = (*itSnapInterv)->getStart();

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

bool Genex0dSourceRock::addHistoryToNodes()
{
  Genex6::SourceRockAdsorptionHistory* history = new Genex6::SourceRockAdsorptionHistory(m_projectHandle, *m_pointAdsorptionHistory);
  Genex6::NodeAdsorptionHistory* adsorptionHistory;

  if (doApplyAdsorption())
  {
    adsorptionHistory = Genex6::AdsorptionSimulatorFactory::getInstance().allocateNodeAdsorptionHistory(m_theChemicalModel->getSpeciesManager(),
                                                                                                        m_projectHandle,
                                                                                                        getAdsorptionSimulatorName());
    if ( adsorptionHistory != nullptr )
    {
      // Add the node-adsorption-history object to the sr-history-object.
      history->setNodeAdsorptionHistory ( adsorptionHistory );
      m_sourceRockNode->addNodeAdsorptionHistory ( adsorptionHistory );
      m_sourceRockNodeAdsorptionHistory.push_back ( history );
    }
    else
    {
        LogHandler(LogHandler::ERROR_SEVERITY) << "Failed while running Genex0d!";
        delete history;
        return false;
    }
  }
  else
  {
    adsorptionHistory = Genex6::AdsorptionSimulatorFactory::getInstance().allocateNodeAdsorptionHistory(m_theChemicalModel->getSpeciesManager(),
                                                                                                        m_projectHandle,
                                                                                                        Genex6::GenexSimulatorId);
    if ( adsorptionHistory != nullptr )
    {
      // Add the node-adsorption-history object to the sr-history-object.
      history->setNodeGenexHistory( adsorptionHistory );
      m_sourceRockNode->addNodeAdsorptionHistory ( adsorptionHistory );
      m_sourceRockNodeAdsorptionHistory.push_back ( history );

    }
    else
    {
        LogHandler(LogHandler::ERROR_SEVERITY) << "Failed while running Genex0d!";
        delete history;
        return false;
    }
  }

  return true;
}

bool Genex0dSourceRock::process()
{
  bool status = true;
  double dt = m_theSimulator->GetMaximumTimeStepSize(m_depositionTime);

  LogHandler(LogHandler::INFO_SEVERITY) << "Chosen maximum timestep size:" << dt;
  LogHandler(LogHandler::INFO_SEVERITY) << "-------------------------------------";

  LogHandler(LogHandler::INFO_SEVERITY) << "Start Of processing...";
  LogHandler(LogHandler::INFO_SEVERITY) << "-------------------------------------";

  std::vector<Genex6::SnapshotInterval*>::iterator itSnapInterv = m_theIntervals.begin();
  const DataAccess::Interface::Snapshot * intervalStart = (*itSnapInterv)->getStart();
  const DataAccess::Interface::Snapshot * intervalEnd = (*itSnapInterv)->getEnd();

  int i = 0;
  double porePressureInterpPrevious = m_inPorePressure[0];
  double temperatureInterpPrevious = m_inTemperatures[0];

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
    computePTSnapShot(intervalStart->getTime(), intervalStart->getTime(), m_inVesAll[i], temperatureInterpPrevious, m_inTemperatures[i], m_inVRE[i],
                      porePressureInterpPrevious, m_inPorePressure[i], m_inPermeability[i], m_inPorosity[i], false);

    double tPrevious = intervalStart->getTime();
    double t = tPrevious - deltaT;

    // Interpolate interval time instances
    while (t > snapShotIntervalEndTime)
    {
      const double VesInterp = interpolateSnapshotProperty(m_inVesAll[i], m_inVesAll[i+1], tPrevious, t, deltaTInterval);
      const double temperatureInterp = interpolateSnapshotProperty(m_inTemperatures[i], m_inTemperatures[i+1], tPrevious, t, deltaTInterval);
      const double VreInterp = interpolateSnapshotProperty(m_inVRE[i], m_inVRE[i+1], tPrevious, t, deltaTInterval);
      const double porePressureInterp = interpolateSnapshotProperty(m_inPorePressure[i], m_inPorePressure[i+1], tPrevious, t, deltaTInterval);
      const double permeabilityInterp = interpolateSnapshotProperty(m_inPermeability[i], m_inPermeability[i+1], tPrevious, t, deltaTInterval);
      const double porosityInterp = interpolateSnapshotProperty(m_inPorosity[i], m_inPorosity[i+1], tPrevious, t, deltaTInterval);

      computePTSnapShot(t + deltaT, t, VesInterp, temperatureInterpPrevious, temperatureInterp, VreInterp,
                        porePressureInterpPrevious, porePressureInterp, permeabilityInterp, porosityInterp, true);

      porePressureInterpPrevious = porePressureInterp;
      temperatureInterpPrevious = temperatureInterp;

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
  computePTSnapShot( intervalEnd->getTime(), intervalEnd->getTime(), m_inVesAll[i], m_inTemperatures[i], m_inTemperatures[i], m_inVRE[i],
                    m_inPorePressure[i], m_inPorePressure[i], m_inPermeability[i], m_inPorosity[i], false );

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

bool Genex0dSourceRock::computePTSnapShot(const double timePrevious, const double time, double inPressure,
                                          const double inTemperaturePrevious, const double inTemperature,
                                          const double inVre, const double inPorePressurePrevious, const double inPorePressure,
                                          const double inPermeability, const double inPorosity, const bool isInterpolatedTime)
{
  LogHandler( LogHandler::INFO_SEVERITY ) << "Computing time instance t:" << time;

  // Check if Ves > maxVes
  if (isVESMaxEnabled() && inPressure > getVESMax())
  {
      inPressure = getVESMax();
  }

  Genex6::Input* theInput = new Genex6::Input( timePrevious, time, inTemperaturePrevious, inTemperature, inPressure, DataAccess::Interface::DefaultUndefinedScalarValue,
                                               DataAccess::Interface::DefaultUndefinedScalarValue, inPorePressurePrevious, inPorePressure, inPorosity, inPermeability, inVre, 0, 0 );

  if (isInterpolatedTime)
  {
    processNodeAtInterpolatedTime(theInput, *m_sourceRockNode);
  }
  else
  {
    processNode(theInput, *m_sourceRockNode, doApplyAdsorption(), doApplyAdsorption());
  }
  m_sourceRockNode->clearInputHistory();

  return true;
}

} // namespace Genex0d
