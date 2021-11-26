//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// Genex6
#include "GenexBaseSourceRock.h"
#include "ChemicalModel.h"
#include "AdsorptionFunctionFactory.h"
#include "AdsorptionSimulatorFactory.h"
#include "SourceRockTypeNameMappings.h"
#include "MixingParameters.h"

// DataAccess library
#include "Surface.h"
#include "Snapshot.h"

// utilities library
#include "ConstantsNumerical.h"
#include "LogHandler.h"

#include "SnapshotInterval.h"

#include "Simulator.h"
#include "SourceRockNode.h"
#include "Input.h"
#include "ConstantsGenex.h"

// std library
#include <vector>

using namespace std;

using namespace DataAccess;
using Interface::Snapshot;
using Interface::Formation;
using Interface::Surface;

namespace Genex6
{

const double GenexBaseSourceRock::conversionCoeffs [8] =
   { -2.60832073307101E-05, 0.236463623513642,
     -0.0319467563289369, 0.00185738251210839,
     2.36948559032296E-05, -6.62225531134738E-06,
     2.38411451425613E-07, -2.692340754443E-09 };

GenexBaseSourceRock::GenexBaseSourceRock (DataAccess::Interface::ProjectHandle& projectHandle, database::Record* record) :
  DataAccess::Interface::SourceRock {projectHandle, record},
  s_CfgFileNameBySRType{SourceRockTypeNameMappings::getInstance().CfgFileNameBySRType()}
{
   m_theSimulator = nullptr;
   m_formation = nullptr;
   m_theChemicalModel  = nullptr;

   m_theChemicalModel1 = nullptr;
   m_theChemicalModel2 = nullptr;

   m_adsorptionSimulator = nullptr;
   m_adsorptionSimulator2 = nullptr;

   m_applySRMixing = false;
   m_isSulphur = false;
}

GenexBaseSourceRock::~GenexBaseSourceRock(void)
{
   clearBaseAll();
}

void GenexBaseSourceRock::getHIBounds( double &HILower, double &HIUpper ) {
   HILower = 28.47;
   HIUpper = 890.1;
}

double GenexBaseSourceRock::convertHCtoHI( double aHC ) {

   double HILower;
   double HIUpper;
   double HIValue;
   double HCValue;

   getHIBounds( HILower, HIUpper );

   // Simple bisection method to compute HI from H/C
   while ( fabs( HILower - HIUpper ) > 0.0001 ) {
      HIValue = 0.5 * (HILower + HIUpper);
      HCValue = convertHItoHC( HIValue );

      if ( HCValue > aHC ) {
         HIUpper = HIValue;
      }
      else {
         HILower = HIValue;
      }
   }

   return HIValue;
}

double GenexBaseSourceRock::convertHItoHC( double aHI ) {

   if ( aHI != Interface::DefaultUndefinedMapValue ) {
      int i;
      double hc = conversionCoeffs[7];
      const double sqrtHI = sqrt( aHI );

      for ( i = 6; i >= 0; --i ) {
         hc = hc * sqrtHI + conversionCoeffs[i];
      }

      return floor( hc * 1000 + 0.5 ) / 1000;
   }
   else {
      return  Interface::DefaultUndefinedMapValue;
   }
}

void GenexBaseSourceRock::clearBaseAll()
{
  clearBase();
}

void GenexBaseSourceRock::clearBase()
{
   clearSnapshotIntervals();
   clearSourceRockNodeAdsorptionHistory ();
   clearSimulatorBase();
}

void GenexBaseSourceRock::clearSimulatorBase()
{
  if (m_theSimulator)
  {
    delete m_theSimulator;
    m_theSimulator = nullptr;
    m_theChemicalModel = nullptr;
  }
  if (m_theChemicalModel1)
  {
    m_theChemicalModel1 = nullptr;
  }
  if (m_theChemicalModel2)
  {
    m_theChemicalModel2 = nullptr;
  }

  if ( m_adsorptionSimulator != nullptr ) {
     delete m_adsorptionSimulator;
     m_adsorptionSimulator = nullptr;
  }
  if ( m_adsorptionSimulator2 != nullptr ) {
     delete m_adsorptionSimulator2;
     m_adsorptionSimulator2 = nullptr;
  }
}

void GenexBaseSourceRock::clearSnapshotIntervals()
{
   std::vector<SnapshotInterval*>::iterator itEnd = m_theIntervals.end();
   for(std::vector<SnapshotInterval*>::iterator it = m_theIntervals.begin(); it != itEnd; ++ it) {
     delete (*it);
   }
   m_theIntervals.clear();
}

double GenexBaseSourceRock::getDepositionTime() const
{
   const Surface * topSurface = m_formation->getTopSurface ();
   const Snapshot * depoSnapshot = topSurface->getSnapshot ();
   return depoSnapshot->getTime ();
}

bool GenexBaseSourceRock::compute()
{
   bool status = true;

   LogHandler( LogHandler::INFO_SEVERITY ) << "Ready to compute SourceRock at : "<< m_formation->getName();

   if(status) status = initialize();

   if(status) status = preprocess();

   if(status) status = addHistoryToNodes ();

   if(status) status = process();

   return status;
}


void GenexBaseSourceRock::saveSourceRockNodeAdsorptionHistory () {

   SourceRockAdsorptionHistoryList::iterator histIter;

   for ( histIter = m_sourceRockNodeAdsorptionHistory.begin (); histIter != m_sourceRockNodeAdsorptionHistory.end (); ++histIter ) {
      (*histIter)->save ();
   }

}

void GenexBaseSourceRock::clearSourceRockNodeAdsorptionHistory () {

   SourceRockAdsorptionHistoryList::iterator histIter;

   for ( histIter = m_sourceRockNodeAdsorptionHistory.begin (); histIter != m_sourceRockNodeAdsorptionHistory.end (); ++histIter ) {
      delete *histIter;
   }

   m_sourceRockNodeAdsorptionHistory.clear ();
}


double GenexBaseSourceRock::getMaximumTimeStepSize ( const double depositionTime ) const {
   return m_theSimulator->GetMaximumTimeStepSize ( depositionTime );
}

const AdsorptionSimulator* GenexBaseSourceRock::getAdsorptionSimulator() const
{
  if( m_adsorptionSimulator  != nullptr ) return m_adsorptionSimulator;
  if( m_adsorptionSimulator2 != nullptr ) return m_adsorptionSimulator2;

  return nullptr;
}

const ChemicalModel* GenexBaseSourceRock::getChemicalModel2() const
{
    return m_theChemicalModel2;
}

void GenexBaseSourceRock::setSimulatorToChemicalModel1()
{
    m_theSimulator->setChemicalModel(m_theChemicalModel1);
}

const ChemicalModel* GenexBaseSourceRock::getChemicalModel1() const
{
    return m_theChemicalModel1;
}

void GenexBaseSourceRock::computeSnapshotIntervals (const DataAccess::Interface::SnapshotList & snapshots)
{
   m_depositionTime = getDepositionTime ();

   DataAccess::Interface::SnapshotList::const_reverse_iterator snapshotIter;

   const DataAccess::Interface::Snapshot * start;
   const DataAccess::Interface::Snapshot * end;

   if (snapshots.size () >= 1) {
      end = 0;
      for(snapshotIter = snapshots.rbegin (); snapshotIter != snapshots.rend () - 1; ++ snapshotIter) {

         start = (*snapshotIter);
         end = 0;

         if ((m_depositionTime > start->getTime ()) ||
             (fabs (m_depositionTime - start->getTime ()) < Genex6::Constants::Zero)) {
            start = *snapshotIter;
            end = *(snapshotIter + 1);
            SnapshotInterval *theInterval = new SnapshotInterval (start, end);

            m_theIntervals.push_back (theInterval);
         }
      }
   } else {
      //throw
   }
}

void GenexBaseSourceRock::initializeSourceRock2(MixingParameters& mixParams, const DataAccess::Interface::SourceRock& sourceRock2, bool printInitialisationDetails)
{
  mixParams.maximumTimeStepSize1 = m_theSimulator->getMaximumTimeStepSize();
  mixParams.numberOfTimeSteps1 = m_theSimulator->getNumberOfTimesteps();

   m_theChemicalModel2 = loadChemicalModel( sourceRock2, printInitialisationDetails );

   assert ( m_theChemicalModel2 != nullptr );

   // How to choose the timeStepSize and numberOfTimeSteps?
   // If hcValueMixing == HC value of one of the mixed SourceRock, then fraction of the second SourceRock type is 0,
   // therefore we can choose timeStepSize and numberOfTimeSteps of the first Source Rock.
   // If hcValueMixing != HC, then choose the minimun timeStep and maximum number of time steps.

   mixParams.maximumTimeStepSize2 = m_theSimulator->getMaximumTimeStepSize();
   mixParams.numberOfTimeSteps2 = m_theSimulator->getNumberOfTimesteps();

   mixParams.hcValue1 = getHcVRe05();
   mixParams.hcValue2 = sourceRock2.getHcVRe05();
}

void GenexBaseSourceRock::applySRMixing(const MixingParameters& mixParams, const double hcValueMixing, bool printInitialisationDetails)
{
	if (mixParams.hcValue1 == hcValueMixing) {
		m_theSimulator->setMaximumTimeStepSize(mixParams.maximumTimeStepSize1);
		m_theSimulator->setNumberOfTimesteps(mixParams.numberOfTimeSteps1);
	}
	else if (mixParams.hcValue2 != hcValueMixing) {
		if (mixParams.maximumTimeStepSize1 < mixParams.maximumTimeStepSize2) m_theSimulator->setMaximumTimeStepSize(mixParams.maximumTimeStepSize1);
		if (mixParams.numberOfTimeSteps1 > mixParams.numberOfTimeSteps2) m_theSimulator->setNumberOfTimesteps(mixParams.numberOfTimeSteps1);
		else m_theSimulator->setNumberOfTimesteps(mixParams.numberOfTimeSteps1);
	}
	if (printInitialisationDetails) {
		LogHandler(LogHandler::INFO_SEVERITY) << "Applying Source Rock mixing H/C = " << hcValueMixing;
	}
}

void GenexBaseSourceRock::validateChemicalModel2(bool printInitialisationDetails, bool& status)
{
  status = m_theChemicalModel2->Validate();

  if(  m_theChemicalModel1->GetNumberOfSpecies() < m_theChemicalModel2->GetNumberOfSpecies() ) {
     m_theChemicalModel = m_theChemicalModel2;
  }

  if(printInitialisationDetails ) {
     if(!status) {
        LogHandler( LogHandler::ERROR_SEVERITY ) << "Invalid Chemical Model. Please check your source rock " << m_formation->getSourceRockType2Name() << " input parameters. Aborting...";
        LogHandler( LogHandler::INFO_SEVERITY ) << "----------------------------------------------------------------------------------";
     }
  }
}

void GenexBaseSourceRock::validateChemicalModel1(bool printInitialisationDetails, bool& status)
{
  status =  m_theChemicalModel1->Validate();

  if(printInitialisationDetails ) {

     if(status) {
        LogHandler( LogHandler::INFO_SEVERITY ) << "End Of Initialization.";
        LogHandler( LogHandler::INFO_SEVERITY ) << "-------------------------------------" ;
     } else {
        LogHandler( LogHandler::ERROR_SEVERITY ) << "Invalid Chemical Model. Please check your source rock input parameters. Aborting...";
        LogHandler( LogHandler::INFO_SEVERITY )  << "----------------------------------------------------------------------------------";
     }
  }
}

void GenexBaseSourceRock::processNodeAtInterpolatedTime(Input *theInput, Genex6::SourceRockNode& itNode)
{
    itNode.AddInput(theInput);

    m_theSimulator->setChemicalModel( m_theChemicalModel1 );

    bool isInitialTimeStep = itNode.RequestComputation(0, *m_theSimulator );
    if( m_applySRMixing ) {
      m_theSimulator->setChemicalModel( m_theChemicalModel2 );
      itNode.RequestComputation( 1, *m_theSimulator );
      m_theSimulator->setChemicalModel( m_theChemicalModel1 );
    }

    if ( not isInitialTimeStep && doApplyAdsorption ()) {
      m_adsorptionSimulator->compute( *theInput, itNode.GetSimulatorState(0));
    }
    if( m_applySRMixing && not isInitialTimeStep && m_adsorptionSimulator2 != nullptr ) {
      m_adsorptionSimulator2->compute( *theInput, itNode.GetSimulatorState(1));
    }

    if( m_applySRMixing ) {
      itNode.RequestMixing( m_theChemicalModel );
    }

    if ( not isInitialTimeStep ) {
      itNode.collectHistory ();
    }

    itNode.clearInputHistory();
}


void GenexBaseSourceRock::processNode(Input *theInput, Genex6::SourceRockNode& itNode, bool adsorptionActive, bool adsorptionOutputPropertiesActive)
{
  itNode.AddInput(theInput);

  m_theSimulator->setChemicalModel( m_theChemicalModel1 );


  bool isInitialTimeStep = itNode.RequestComputation(0, *m_theSimulator ); // 0 - first SourceRock

  if( m_applySRMixing ) {
    m_theSimulator->setChemicalModel( m_theChemicalModel2 );
    itNode.RequestComputation( 1, *m_theSimulator ); // 1 -  second Source Rock
    m_theSimulator->setChemicalModel( m_theChemicalModel1 );
  }

  if ( not isInitialTimeStep && adsorptionActive) {
    m_adsorptionSimulator->compute( *theInput, itNode.GetSimulatorState(0));
  }
  if( m_applySRMixing && !isInitialTimeStep && m_adsorptionSimulator2 != nullptr ) {
    m_adsorptionSimulator2->compute( *theInput, itNode.GetSimulatorState(1));
  }

  if ( m_applySRMixing ) {
    itNode.RequestMixing( m_theChemicalModel ); // we always use the chemicalModel with bigger number of Species - m_theChemicalModel - for mixing
  }

  if ( !isInitialTimeStep && adsorptionActive ) {
    itNode.getPrincipleSimulatorState ().postProcessShaleGasTimeStep ( m_theChemicalModel, theInput->GetPreviousTime() - theInput->GetTime() );
  }

  if ( not isInitialTimeStep ) {
    itNode.collectHistory ();
  }

  if ( adsorptionOutputPropertiesActive ) {
    itNode.updateAdsorptionOutput ( *getAdsorptionSimulator() );
  }
}

void GenexBaseSourceRock::initializeSimulator(bool printInitialisationDetails)
{
  if(printInitialisationDetails ) {
     LogHandler( LogHandler::INFO_SEVERITY ) << "Start Of Initialization...";
  }

  if( m_theSimulator == 0 ) {
     m_theSimulator = new Genex6::Simulator() ;
     m_theChemicalModel1 = loadChemicalModel( *this, printInitialisationDetails );
  }

  assert ( m_theSimulator != nullptr );
  m_theSimulator->setChemicalModel( m_theChemicalModel1 );
  m_theChemicalModel = m_theChemicalModel1;
}


void GenexBaseSourceRock::initializeAdsorptionModel(bool printInitialisationDetails, bool& status, bool isTOCDependent, const std::string& adsorptionCapacityFunctionName,
                                                const std::string& adsorptionSimulatorName, bool computeOTGC, DataAccess::Interface::ProjectHandle& projectHandle)
{
  if ( printInitialisationDetails ) {
     LogHandler( LogHandler::INFO_SEVERITY ) << "Applying adsorption, TOCDependent is " << (isTOCDependent ? "true" : "false" )
        << ", function is " << adsorptionCapacityFunctionName
        << ", OTGC is " << ( computeOTGC ? "on" : "off" );
  }

  AdsorptionFunction*  adsorptionFunction = AdsorptionFunctionFactory::getInstance ().getAdsorptionFunction ( projectHandle,
                                                                                                              isTOCDependent,
                                                                                                              adsorptionCapacityFunctionName);

  m_adsorptionSimulator = AdsorptionSimulatorFactory::getInstance ().getAdsorptionSimulator ( projectHandle,
                                                                                              m_theChemicalModel1->getSpeciesManager(),
                                                                                              adsorptionSimulatorName,
                                                                                              computeOTGC,
                                                                                              false );
  status = status && adsorptionFunction->isValid ();

  if ( not adsorptionFunction->isValid () && printInitialisationDetails ) {
     LogHandler( LogHandler::ERROR_SEVERITY ) << "Invalid adsorption function. Please check adsorption function parameters. Aborting ...";
     LogHandler( LogHandler::ERROR_SEVERITY ) << adsorptionFunction->getErrorMessage ();
  }

  assert ( m_adsorptionSimulator != nullptr );
  assert ( adsorptionFunction != nullptr );

  m_adsorptionSimulator->setAdsorptionFunction ( adsorptionFunction );
}

void GenexBaseSourceRock::initializeAdsorptionModelSR2(const DataAccess::Interface::SourceRock& sourceRock2, bool& status, const bool printInitialisationDetails,
                                                  DataAccess::Interface::ProjectHandle& projectHandle)
{
  if (printInitialisationDetails)
  {
    LogHandler( LogHandler::ERROR_SEVERITY ) << "Applying adsorption for SourceRock type 2, TOCDependent is "
       << (sourceRock2.adsorptionIsTOCDependent () ? "true" : "false" ) << ", function is "
       << sourceRock2.getAdsorptionCapacityFunctionName () << ", OTGC is " << (sourceRock2.doComputeOTGC () ? "on" : "off" );
  }

  AdsorptionFunction*  adsorptionFunction = AdsorptionFunctionFactory::getInstance ().getAdsorptionFunction ( projectHandle,
                                                                                                              sourceRock2.adsorptionIsTOCDependent (),
                                                                                                              sourceRock2.getAdsorptionCapacityFunctionName ());

  m_adsorptionSimulator2 = AdsorptionSimulatorFactory::getInstance ().getAdsorptionSimulator ( projectHandle,
                                                                                               m_theChemicalModel2->getSpeciesManager(),
                                                                                               sourceRock2.getAdsorptionSimulatorName (),
                                                                                               sourceRock2.doComputeOTGC (),
                                                                                               false );

  assert ( m_adsorptionSimulator2 != 0 );
  assert ( adsorptionFunction != 0 );

  status = status && adsorptionFunction->isValid ();

  if ( !adsorptionFunction->isValid () && printInitialisationDetails ) {
     LogHandler( LogHandler::ERROR_SEVERITY ) << "Invalid adsorption function. Please check adsorption function parameters. Aborting ...";
     LogHandler( LogHandler::ERROR_SEVERITY ) << adsorptionFunction->getErrorMessage ();
  }

  m_adsorptionSimulator2->setAdsorptionFunction ( adsorptionFunction );
}

ChemicalModel* GenexBaseSourceRock::loadChemicalModel( const DataAccess::Interface::SourceRock& theSourceRock,
                                                    const bool printInitialisationDetails ) {

  double theScIni = theSourceRock.getScVRe05();

  double in_SC = (theScIni != 0.0 ? ( validateGuiValue(theScIni, 0.01, 0.09) == true ? theScIni : 0.03 ) : 0.0 );

  const std::string & SourceRockType = theSourceRock.getBaseSourceRockType();
  const std::string & theType = ( SourceRockType.empty() ? determineConfigurationFileName( theSourceRock.getType() ) :
                                                      determineConfigurationFileName( SourceRockType ) );

  std::string fullPath;
  char *MY_GENEX5DIR = getenv("MY_GENEX5DIR");

  if(MY_GENEX5DIR != nullptr) {
    fullPath = MY_GENEX5DIR;
  } else if(getGenexEnvironment(in_SC) != nullptr) {
    fullPath = getGenexEnvironment(in_SC);
  } else {
    LogHandler( LogHandler::ERROR_SEVERITY ) << "Environment Variable " << (getRunType(in_SC) & Genex6::Constants::SIMGENEX5 ? "GENEX5DIR" : "GENEX6DIR") << " not set. Aborting...";
    exit(1); //TODO_SK: gracefully exit. Note: this statement used to be: return false; which crashes the application as a pointer should be returned.
  }

  double theHcIni = theSourceRock.getHcVRe05();
  double theEact  = 1000.0 * theSourceRock.getPreAsphaltStartAct();

  double theAsphalteneDiffusionEnergy = 1000.0 * theSourceRock.getAsphalteneDiffusionEnergy();
  double theResinDiffusionEnergy  = 1000.0 * theSourceRock.getResinDiffusionEnergy();
  double theC15AroDiffusionEnergy = 1000.0 * theSourceRock.getC15AroDiffusionEnergy();
  double theC15SatDiffusionEnergy = 1000.0 * theSourceRock.getC15SatDiffusionEnergy();

  const double in_HC = validateGuiValue(theHcIni, 0.5, 1.8) == true ? theHcIni : 1.56;
  double in_Emean = validateGuiValue(theEact, 190000.0, 230000.0) == true ? theEact : 216000.0;

  double in_VRE = 0.5; //overwrite, no other value should be passed to the kernel
  bool flag = validateGuiValue(theAsphalteneDiffusionEnergy, 50000.0, 100000.0);
  double in_asphalteneDiffusionEnergy = (flag == true ? theAsphalteneDiffusionEnergy : 88000.0);
  flag = validateGuiValue(theResinDiffusionEnergy, 50000.0, 100000.0);
  double in_resinDiffusionEnergy  = (flag == true ? theResinDiffusionEnergy : 85000.0);
  flag = validateGuiValue(theC15AroDiffusionEnergy, 50000.0, 100000.0);
  double in_C15AroDiffusionEnergy = (flag == true ? theC15AroDiffusionEnergy : 80000.0);
  flag = validateGuiValue(theC15SatDiffusionEnergy, 50000.0, 100000.0);
  double in_C15SatDiffusionEnergy = (flag == true ? theC15SatDiffusionEnergy : 75000.0);

  ChemicalModel * theChemicalModel = m_theSimulator->loadChemicalModel(fullPath, getRunType(in_SC), theType,
                                                                       in_HC, in_SC,in_Emean, in_VRE,
                                                                       in_asphalteneDiffusionEnergy, in_resinDiffusionEnergy,
                                                                       in_C15AroDiffusionEnergy, in_C15SatDiffusionEnergy);

  if(printInitialisationDetails ) {

    ostringstream sourceRock;
    sourceRock <<  "Source Rock Type  : " << SourceRockType;
    if( m_applySRMixing ) {
      sourceRock << " (H/C = " << in_HC << ")";
    }

    LogHandler( LogHandler::INFO_SEVERITY ) << sourceRock.str();
    LogHandler( LogHandler::INFO_SEVERITY ) << "Configuration File: " << theType <<  (!(getRunType(in_SC) & Genex6::Constants::SIMGENEX5) ? " (with sulphur)" : "");
  }

  return theChemicalModel;
}

bool GenexBaseSourceRock::validateGuiValue(const double GuiValue, const double LowerBound,const double UpperBound)
{
  if(GuiValue > (LowerBound - Constants::Zero) &&
     GuiValue < (UpperBound + Constants::Zero) &&
     GuiValue > 0.0) {
    return true;
  }
  return false;
}

const std::string& GenexBaseSourceRock::determineConfigurationFileName(const std::string& SourceRockType)
{
  static std::string ret("TypeI");

  std::unordered_map<std::string, std::string>::const_iterator it = s_CfgFileNameBySRType.find(SourceRockType);

  if(it != s_CfgFileNameBySRType.end()) {
    return it->second;
  } else {
    LogHandler( LogHandler::WARNING_SEVERITY ) << "Source rock " << SourceRockType << " not found. Setting configuration file to TypeI.";
  }
  return ret;
}

int GenexBaseSourceRock::getRunType(const double in_SC) const
{
  return (in_SC != 0.0 ? Genex6::Constants::SIMGENEX : (Genex6::Constants::SIMGENEX | Genex6::Constants::SIMGENEX5));
}

char * GenexBaseSourceRock::getGenexEnvironment(const double in_SC) const
{
  if (getRunType(in_SC) & Genex6::Constants::SIMGENEX5)
  {
    return getenv("GENEX5DIR");
  }
  else
  {
    return getenv("GENEX6DIR");
  }

  return nullptr;
}

}//namespace Genex6
