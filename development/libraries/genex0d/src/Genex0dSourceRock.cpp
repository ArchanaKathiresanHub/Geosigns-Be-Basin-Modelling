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

void Genex0dSourceRock::addHistoryToSourceRockNode()
{
  const Genex6::ChemicalModel & chemModel = m_simulator->getChemicalModel();
  DataAccess::Interface::ProjectHandle * projectHandle = m_projectMgr.projectHandle();

  m_genexHistory.reset(Genex6::AdsorptionSimulatorFactory::getInstance().allocateNodeAdsorptionHistory(
                         chemModel.getSpeciesManager(),
                         projectHandle,
                         "GenexSimulator"));

  if ( m_genexHistory ==  nullptr)
  {
    throw Genex0dException() << "Fatal error, node adsorption history could not be initiated!";
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




bool Genex0dSourceRock::computeSnapShot (const double previousTime,
                                         const DataAccess::Interface::Snapshot *theSnapshot)
{
  bool status = true;
  double time = theSnapshot->getTime();
  LogHandler( LogHandler::INFO_SEVERITY ) << "Computing SnapShot t:" << time;

  const DataModel::AbstractProperty* property = 0;

  property = m_propertyManager->getProperty ( "Ves" );
  AbstractDerivedProperties::SurfacePropertyPtr calcVes = m_propertyManager->getSurfaceProperty ( property, theSnapshot, m_formation->getTopSurface () );

  property = m_propertyManager->getProperty ( "ErosionFactor" );
  AbstractDerivedProperties::FormationMapPropertyPtr calcErosion = m_propertyManager->getFormationMapProperty ( property, theSnapshot, m_formation  );

  property = m_propertyManager->getProperty ( "Temperature" );
  AbstractDerivedProperties::SurfacePropertyPtr calcTemp = m_propertyManager->getSurfaceProperty ( property, theSnapshot, m_formation->getTopSurface () );

  property = m_propertyManager->getProperty ( "LithoStaticPressure" );
  AbstractDerivedProperties::SurfacePropertyPtr calcLP = m_propertyManager->getSurfaceProperty ( property, theSnapshot, m_formation->getTopSurface () );

  property = m_propertyManager->getProperty ( "HydroStaticPressure" );
  AbstractDerivedProperties::SurfacePropertyPtr calcHP = m_propertyManager->getSurfaceProperty ( property, theSnapshot, m_formation->getTopSurface () );

  property = m_propertyManager->getProperty ( "Pressure" );
  AbstractDerivedProperties::SurfacePropertyPtr calcPressure = m_propertyManager->getSurfaceProperty ( property, theSnapshot, m_formation->getTopSurface () );

  property = m_propertyManager->getProperty ( "Porosity" );
  AbstractDerivedProperties::FormationSurfacePropertyPtr calcPorosity = m_propertyManager->getFormationSurfaceProperty ( property, theSnapshot, m_formation, m_formation->getTopSurface () );

  property = m_propertyManager->getProperty ( "Permeability" );
  AbstractDerivedProperties::FormationSurfacePropertyPtr calcPermeability = m_propertyManager->getFormationSurfaceProperty ( property, theSnapshot, m_formation, m_formation->getTopSurface () );

  property =  m_propertyManager->getProperty ( "Vr" );
  AbstractDerivedProperties::FormationSurfacePropertyPtr calcVre = m_propertyManager->getFormationSurfaceProperty ( property, theSnapshot, m_formation, m_formation->getTopSurface () );

  if( calcVes == nullptr || calcTemp == nullptr || calcVre == nullptr )
  {
    status = false;
    if( calcTemp == nullptr ) {
      LogHandler( LogHandler::ERROR_SEVERITY ) << "Missing Temperature map for the shapshot  :" << time << ". Aborting... ";
      LogHandler( LogHandler::INFO_SEVERITY ) << " ------------------------------------:";
    }
    if( calcVre == nullptr ) {
      LogHandler( LogHandler::ERROR_SEVERITY ) << "Missing Vr map for the shapshot  :" << time << ". Aborting... ";
      LogHandler( LogHandler::INFO_SEVERITY ) << " ------------------------------------:";
    }
    if( calcVes == nullptr ) {
      LogHandler( LogHandler::ERROR_SEVERITY ) << "Missing VES map for the shapshot  :" << time << ". Aborting... ";
      LogHandler( LogHandler::INFO_SEVERITY ) << " ------------------------------------:";
    }
  }
  if( status && ( calcLP == nullptr || calcHP == nullptr || calcPressure == nullptr ||  calcPorosity == nullptr || calcPermeability == nullptr ))
  {
    status = false;

    if( calcLP == nullptr ) {
      LogHandler( LogHandler::ERROR_SEVERITY ) << "Missing lithostatic pressure for the shapshot  :" << time << ". Aborting... ";
      LogHandler( LogHandler::INFO_SEVERITY ) << " ------------------------------------:";
    }
    if( calcHP == nullptr ) {
      LogHandler( LogHandler::ERROR_SEVERITY ) << "Missing hydrostatic pressure for the shapshot  :" << time << ". Aborting... ";
      LogHandler( LogHandler::INFO_SEVERITY ) << " ------------------------------------:";
    }
    if( calcPressure == nullptr ) {
      LogHandler( LogHandler::ERROR_SEVERITY ) << "Missing pore pressure for the shapshot  :" << time << ". Aborting... ";
      LogHandler( LogHandler::INFO_SEVERITY ) << " ------------------------------------:";
    }
    if( calcPorosity == nullptr ) {
      LogHandler( LogHandler::ERROR_SEVERITY ) << "Missing porosity for the shapshot  :" << time << ". Aborting... ";
      LogHandler( LogHandler::INFO_SEVERITY ) << " ------------------------------------:";
    }
    if( calcPermeability == nullptr ) {
      LogHandler( LogHandler::ERROR_SEVERITY ) << "Missing permeability for the shapshot  :" << time << ". Aborting... ";
      LogHandler( LogHandler::INFO_SEVERITY ) << " ------------------------------------:";
    }

  }

  if(!status)
  {
    return status;
  }

  calcVes->retrieveData();
  calcTemp->retrieveData();
  calcVre->retrieveData();

  if( calcPressure ) calcPressure->retrieveData();

  createSnapShotOutputMaps(theSnapshot);

  bool useMaximumVes = isVESMaxEnabled();
  double maximumVes = getVESMax();
  maximumVes *= MegaPaToPa;

  if( calcErosion ) calcErosion->retrieveData();

  //need to optimize..
  std::vector<Genex6::SourceRockNode*>::iterator itNode;
  for(itNode = m_theNodes.begin(); itNode != m_theNodes.end(); ++ itNode) {

    double in_VES = calcVes->get ((*itNode)->GetI(), (*itNode)->GetJ());

    if(useMaximumVes && in_VES > maximumVes) {
      in_VES = maximumVes;
    }
    double in_Temp = calcTemp->get(( *itNode)->GetI(), (*itNode)->GetJ());

    double in_thicknessScaling = calcErosion ? calcErosion->get(( *itNode)->GetI(), (*itNode)->GetJ()) : 1.0;

    double nodeHydrostaticPressure =  ( calcHP ?  1.0e6 * calcHP->get ((*itNode)->GetI(), (*itNode)->GetJ()) : CauldronNoDataValue );

    double nodePorePressure = ( calcPressure ?  1.0e6 * calcPressure->get ((*itNode)->GetI(), (*itNode)->GetJ()) : CauldronNoDataValue );

    double nodePorosity =  ( calcPorosity ? 0.01 * calcPorosity->get ((*itNode)->GetI(), (*itNode)->GetJ()) : CauldronNoDataValue );

    double nodePermeability = ( calcPermeability ? calcPermeability->get ((*itNode)->GetI(), (*itNode)->GetJ()) : CauldronNoDataValue );

    double nodeVre = calcVre->get((*itNode)->GetI(), (*itNode)->GetJ() );

    double nodeLithostaticPressure =  ( calcLP ?  1.0e6 * calcLP->get ((*itNode)->GetI(), (*itNode)->GetJ()) : CauldronNoDataValue );

    Genex6::Input *theInput = new Genex6::Input( previousTime, time,
                                                 in_Temp,
                                                 // The duplicate value is not used in the fastgenex6 simulator
                                                 in_Temp,
                                                 in_VES,
                                                 nodeLithostaticPressure,
                                                 nodeHydrostaticPressure,
                                                 nodePorePressure,
                                                 // The duplicate value is not used in the fastgenex6 simulator
                                                 nodePorePressure,
                                                 nodePorosity,
                                                 nodePermeability,
                                                 nodeVre,
                                                 (*itNode)->GetI (),
                                                 (*itNode)->GetJ (),
                                                 in_thicknessScaling );

    (*itNode)->AddInput(theInput);
    m_theSimulator->setChemicalModel( m_theChemicalModel1 );

    bool isInitialTimeStep = (*itNode)->RequestComputation(0, *m_theSimulator ); // 0 - first SourceRock

    if( m_applySRMixing ) {
      m_theSimulator->setChemicalModel( m_theChemicalModel2 );
      (*itNode)->RequestComputation( 1, *m_theSimulator ); // 1 -  second Source Rock
      m_theSimulator->setChemicalModel( m_theChemicalModel1 );
    }

    if ( not isInitialTimeStep && doApplyAdsorption ()) {
      m_adsorptionSimulator->compute( *theInput, &(*itNode)->GetSimulatorState(0));
    }
    if( m_applySRMixing && not isInitialTimeStep && m_adsorptionSimulator2 != 0 ) {
      m_adsorptionSimulator2->compute( *theInput, &(*itNode)->GetSimulatorState(1));
    }

    if ( m_applySRMixing ) {
      (*itNode)->RequestMixing( m_theChemicalModel ); // we always use the chemicalModel with bigger number of Species - m_theChemicalModel - for mixing
    }

    if ( not isInitialTimeStep and doApplyAdsorption () ) {
      (*itNode)->getPrincipleSimulatorState ().postProcessShaleGasTimeStep ( m_theChemicalModel, previousTime - time );
    }

    if ( not isInitialTimeStep ) {
      (*itNode)->collectHistory ();
    }

    if ( doOutputAdsorptionProperties ()) {
      (*itNode)->updateAdsorptionOutput ( *getAdsorptionSimulator() );
    }

    updateSnapShotOutputMaps((*itNode));
    (*itNode)->ClearInputHistory();

  }

  saveSnapShotOutputMaps();

  calcVes->restoreData();
  calcTemp->restoreData();
  calcVre->restoreData();

  if( calcPressure ) calcPressure->restoreData();
  if( calcErosion )  calcErosion->restoreData();

  return status;
}








bool Genex0dSourceRock::process()
{
  bool status = true;
  const SnapshotInterval *first   = m_theIntervals.front();
  const Snapshot *simulationStart = first->getStart();
  double t                        = simulationStart->getTime();
  double previousTime;
  double dt                       = m_simulator->GetMaximumTimeStepSize(m_depositionTime);

  LogHandler( LogHandler::INFO_SEVERITY ) << "Chosen maximum timestep size:" << dt;
  LogHandler( LogHandler::INFO_SEVERITY ) << "-------------------------------------";

  LogHandler( LogHandler::INFO_SEVERITY ) << "Start Of processing...";
  LogHandler( LogHandler::INFO_SEVERITY ) << "-------------------------------------";

  //compute first snapshot
  status = computeSnapShot(t + dt, simulationStart);

  if(status == false) {
    return status;
  }

  std::vector<SnapshotInterval*>::iterator itSnapInterv ;

  LinearGridInterpolator *VESInterpolator  = new LinearGridInterpolator;
  LinearGridInterpolator *TempInterpolator = new LinearGridInterpolator;
  LinearGridInterpolator *vreInterpolator  = new LinearGridInterpolator;
  LinearGridInterpolator *porePressureInterpolator = new LinearGridInterpolator;

  LinearGridInterpolator *ThicknessScalingInterpolator = 0;
  LinearGridInterpolator *lithostaticPressureInterpolator = 0;
  LinearGridInterpolator *hydrostaticPressureInterpolator = 0;
  LinearGridInterpolator *porosityInterpolator            = 0;
  LinearGridInterpolator *permeabilityInterpolator        = 0;

  const Snapshot *intervalStart, *intervalEnd = 0;

  for(itSnapInterv = m_theIntervals.begin(); itSnapInterv != m_theIntervals.end(); ++ itSnapInterv) {

    intervalStart = (*itSnapInterv)->getStart();
    intervalEnd   = (*itSnapInterv)->getEnd();

    // Compute the number of time-steps that will be in the snapshot interval.
    double numberOfTimeSteps = std::ceil (( intervalStart->getTime () - intervalEnd->getTime ()) / dt );

    // Compute the deltaT so that there is a uniform time-step size over the snapshot interval.
    double deltaT = ( intervalStart->getTime () - intervalEnd->getTime ()) / numberOfTimeSteps;

    // Because the time-step size is computed within this loop
    // the current time (t) cannot be set outside of it.
    if ( intervalStart == simulationStart ) {
      previousTime = simulationStart->getTime ();
      t = simulationStart->getTime () - deltaT;
    }

    const DataModel::AbstractProperty* property = m_propertyManager->getProperty ( "Ves" );
    AbstractDerivedProperties::SurfacePropertyPtr startVes = m_propertyManager->getSurfaceProperty ( property, intervalStart, m_formation->getTopSurface () );
    AbstractDerivedProperties::SurfacePropertyPtr endVes   = m_propertyManager->getSurfaceProperty ( property, intervalEnd, m_formation->getTopSurface () );

    property = m_propertyManager->getProperty ( "Temperature" );
    AbstractDerivedProperties::SurfacePropertyPtr startTemp = m_propertyManager->getSurfaceProperty ( property, intervalStart, m_formation->getTopSurface () );
    AbstractDerivedProperties::SurfacePropertyPtr endTemp   = m_propertyManager->getSurfaceProperty ( property, intervalEnd, m_formation->getTopSurface () );

    property = m_propertyManager->getProperty ( "Vr" );
    AbstractDerivedProperties::FormationSurfacePropertyPtr startVr = m_propertyManager->getFormationSurfaceProperty ( property, intervalStart,  m_formation, m_formation->getTopSurface () );
    AbstractDerivedProperties::FormationSurfacePropertyPtr endVr   = m_propertyManager->getFormationSurfaceProperty ( property, intervalEnd,  m_formation, m_formation->getTopSurface () );

    property = m_propertyManager->getProperty ( "Pressure" );
    AbstractDerivedProperties::SurfacePropertyPtr startPressure = m_propertyManager->getSurfaceProperty ( property, intervalStart, m_formation->getTopSurface () );
    AbstractDerivedProperties::SurfacePropertyPtr endPressure   = m_propertyManager->getSurfaceProperty ( property, intervalEnd, m_formation->getTopSurface () );

    if( startTemp == 0 or endTemp == 0 or
        startVes == 0 or endVes == 0 or
        startVr == 0 or endVr == 0 or
        startPressure == 0 or endPressure == 0 ) {

      if ( startTemp == 0 ) {
        LogHandler( LogHandler::ERROR_SEVERITY ) << "Missing start temperature map for snapshot " << intervalStart->getTime ();
      }

      if ( endTemp == 0 ) {
        LogHandler( LogHandler::ERROR_SEVERITY ) << "Missing end temperature map for snapshot " << intervalEnd->getTime ();
      }

      if ( startVr == 0 ) {
        LogHandler( LogHandler::ERROR_SEVERITY ) << "Missing start Vr map for snapshot " << intervalStart->getTime ();
      }

      if ( endVr == 0 ) {
        LogHandler( LogHandler::ERROR_SEVERITY ) << "Missing end Vr map for snapshot " << intervalEnd->getTime ();
      }

      if ( startVes == 0 ) {
        LogHandler( LogHandler::ERROR_SEVERITY ) << "Missing start Ves map for snapshot " << intervalStart->getTime ();
      }

      if ( endVes == 0 ) {
        LogHandler( LogHandler::ERROR_SEVERITY ) << "Missing end Ves map for snapshot " << intervalEnd->getTime ();
      }

      if ( startPressure == 0 ) {
        LogHandler( LogHandler::ERROR_SEVERITY ) << "Missing pressure map for snapshot " << intervalStart->getTime ();
      }

      if ( endPressure == 0 ) {
        LogHandler( LogHandler::ERROR_SEVERITY ) << "Missing pressure map for snapshot " << intervalEnd->getTime ();
      }

      status = false;
      break;
    }

    startTemp->retrieveData();
    endTemp->retrieveData();

    startVes->retrieveData();
    endVes->retrieveData();

    startVr->retrieveData();
    endVr->retrieveData();

    startPressure->retrieveData();
    endPressure->retrieveData();

    TempInterpolator ->compute(intervalStart, startTemp,  intervalEnd, endTemp);
    VESInterpolator->compute( intervalStart, startVes, intervalEnd, endVes );
    vreInterpolator->compute (intervalStart, startVr,  intervalEnd, endVr );
    porePressureInterpolator->compute(intervalStart, startPressure, intervalEnd, endPressure );

    startTemp->restoreData();
    endTemp->restoreData();

    startVes->restoreData();
    endVes->restoreData();

    startVr->restoreData();
    endVr->restoreData();

    startPressure->restoreData();
    endPressure->restoreData();

    if( doApplyAdsorption () ) {
      property = m_propertyManager->getProperty ( "LithoStaticPressure" );
      AbstractDerivedProperties::SurfacePropertyPtr startLP = m_propertyManager->getSurfaceProperty ( property, intervalStart, m_formation->getTopSurface () );
      AbstractDerivedProperties::SurfacePropertyPtr endLP   = m_propertyManager->getSurfaceProperty ( property, intervalEnd, m_formation->getTopSurface () );

      property = m_propertyManager->getProperty ( "HydroStaticPressure" );
      AbstractDerivedProperties::SurfacePropertyPtr startHP = m_propertyManager->getSurfaceProperty ( property, intervalStart, m_formation->getTopSurface () );
      AbstractDerivedProperties::SurfacePropertyPtr endHP   = m_propertyManager->getSurfaceProperty ( property, intervalEnd, m_formation->getTopSurface () );

      property = m_propertyManager->getProperty ( "Porosity" );
      AbstractDerivedProperties::FormationSurfacePropertyPtr startPorosity = m_propertyManager->getFormationSurfaceProperty ( property, intervalStart, m_formation,m_formation->getTopSurface () );
      AbstractDerivedProperties::FormationSurfacePropertyPtr endPorosity   = m_propertyManager->getFormationSurfaceProperty ( property, intervalEnd, m_formation, m_formation->getTopSurface () );

      property = m_propertyManager->getProperty ( "Permeability" );
      AbstractDerivedProperties::FormationSurfacePropertyPtr startPermeability = m_propertyManager->getFormationSurfaceProperty ( property, intervalStart, m_formation, m_formation->getTopSurface () );
      AbstractDerivedProperties::FormationSurfacePropertyPtr endPermeability   = m_propertyManager->getFormationSurfaceProperty ( property, intervalEnd, m_formation, m_formation->getTopSurface () );

      if( startLP == 0 or endLP == 0 or
          startHP == 0 or endHP == 0 or
          startPorosity == 0 or endPorosity == 0 or
          startPermeability == 0 or endPermeability == 0 ) {

        status = false;

        if ( startLP == 0 ) {
          LogHandler( LogHandler::ERROR_SEVERITY ) << " Missing litho-static pressure map for snapshot " << intervalStart->getTime ();
        }

        if ( endLP == 0 ) {
          LogHandler( LogHandler::ERROR_SEVERITY ) << " Missing litho-static pressure map for snapshot " << intervalEnd->getTime ();
        }

        if ( startHP == 0 ) {
          LogHandler( LogHandler::ERROR_SEVERITY ) << " Missing hydro-static pressure map for snapshot " << intervalStart->getTime ();
        }

        if ( endHP == 0 ) {
          LogHandler( LogHandler::ERROR_SEVERITY ) << " Missing hydro-static pressure map for snapshot " << intervalEnd->getTime ();
        }

        if ( startPorosity == 0 ) {
          LogHandler( LogHandler::ERROR_SEVERITY ) << " Missing porosity map for snapshot " << intervalStart->getTime ();
        }

        if ( endPorosity == 0 ) {
          LogHandler( LogHandler::ERROR_SEVERITY ) << " Missing porosity map for snapshot " << intervalEnd->getTime ();
        }

        if ( startPermeability == 0 ) {
          LogHandler( LogHandler::ERROR_SEVERITY ) << " Missing permeability map for snapshot " << intervalStart->getTime ();
        }

        if ( endPermeability == 0 ) {
          LogHandler( LogHandler::ERROR_SEVERITY ) << " Missing permeability map for snapshot " << intervalEnd->getTime ();
        }

        break;
      }
      lithostaticPressureInterpolator = new LinearGridInterpolator;
      hydrostaticPressureInterpolator = new LinearGridInterpolator;
      porosityInterpolator = new LinearGridInterpolator;
      permeabilityInterpolator = new LinearGridInterpolator;

      lithostaticPressureInterpolator->compute(intervalStart, startLP, intervalEnd, endLP );
      hydrostaticPressureInterpolator->compute(intervalStart, startHP, intervalEnd, endHP );
      porosityInterpolator->compute(intervalStart, startPorosity, intervalEnd, endPorosity );
      permeabilityInterpolator->compute(intervalStart, startPermeability, intervalEnd, endPermeability );
    }

    property = m_propertyManager->getProperty ( "ErosionFactor" );

    AbstractDerivedProperties::FormationMapPropertyPtr thicknessScalingAtStart = m_propertyManager->getFormationMapProperty ( property, intervalStart, m_formation );
    AbstractDerivedProperties::FormationMapPropertyPtr thicknessScalingAtEnd   = m_propertyManager->getFormationMapProperty ( property, intervalEnd, m_formation );

    if(thicknessScalingAtStart && thicknessScalingAtEnd) {

      ThicknessScalingInterpolator = new LinearGridInterpolator;
      ThicknessScalingInterpolator->compute(intervalStart, thicknessScalingAtStart,
                                            intervalEnd,   thicknessScalingAtEnd);

    }

    double snapShotIntervalEndTime = intervalEnd->getTime();

    // t can be less that the interval end time.
    // E.g. if the last snapshot interval was very small then t may be less than the interval end-time.
    // This is okay, since the time-step performed for the end of the interval integrates the equations
    // over the time-step previousTime .. interval-end-time.
    while(t > snapShotIntervalEndTime) {
      //within the interval just compute, do not save
      // computeTimeInstance(t, VESInterpolator, TempInterpolator, ThicknessScalingInterpolator);

      if ( previousTime > t ) {

        computeTimeInstance ( previousTime, t,
                              VESInterpolator,
                              TempInterpolator,
                              ThicknessScalingInterpolator,
                              lithostaticPressureInterpolator,
                              hydrostaticPressureInterpolator,
                              porePressureInterpolator,
                              porosityInterpolator,
                              permeabilityInterpolator,
                              vreInterpolator );
      }

      previousTime = t;
      t -= deltaT;

      // If t is very close to the snapshot time then set t to be the snapshot interval end-time.
      // This is to eliminate the very small time-steps that can occur (O(1.0e-13))
      // as the time-stepping approaches a snapshot time.
      if ( t - Genex6::Constants::TimeStepFraction * deltaT < snapShotIntervalEndTime ) {
        t = snapShotIntervalEndTime;
      }


    }

    // Output at desired snapshots
    if( intervalEnd->getType() == Interface::MAJOR or m_minorOutput) {
      computeSnapShot(previousTime, intervalEnd);
      previousTime = intervalEnd->getTime();
    }

  }

  delete VESInterpolator;
  delete TempInterpolator;
  delete ThicknessScalingInterpolator;
  delete lithostaticPressureInterpolator;
  delete hydrostaticPressureInterpolator;
  delete porePressureInterpolator;
  delete porosityInterpolator;
  delete permeabilityInterpolator;
  delete vreInterpolator;

  clearSimulator();

  if(status) {
    LogHandler( LogHandler::INFO_SEVERITY ) << "-------------------------------------";
    LogHandler( LogHandler::INFO_SEVERITY ) << "End of processing.";
    LogHandler( LogHandler::INFO_SEVERITY ) << "-------------------------------------";
  }

  saveSourceRockNodeAdsorptionHistory ();

  return status;
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
