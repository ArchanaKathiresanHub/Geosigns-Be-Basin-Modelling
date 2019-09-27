//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// std library
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <vector>
#include <iostream>
#include <algorithm>

// DataAccess library
#include "ProjectHandle.h"
#include "Surface.h"
#include "Snapshot.h"
#include "GridMap.h"
#include "Grid.h"
#include "Property.h"
#include "PropertyValue.h"
#include "AttributeValue.h"
#include "LithoType.h"
#include "Interface.h"
#include "SGDensitySample.h"
#include "ObjectFactory.h"

// utilities library
#include "ConstantsMathematics.h"
#include "ConstantsNumerical.h"
#include "LogHandler.h"

#include "GenexSourceRock.h"
#include "LocalGridInterpolator.h"
#include "LinearGridInterpolator.h"
#include "SnapshotInterval.h"

#include "Simulator.h"
#include "SimulatorState.h"
#include "SourceRockNode.h"
#include "Input.h"
#include "ConstantsGenex.h"
#include "SpeciesResult.h"
#include "Utilities.h"

#include "SpeciesManager.h"

#include "GenexResultManager.h"
#include "ComponentManager.h"

#include "NumericFunctions.h"

#include "AdsorptionFunctionFactory.h"
#include "AdsorptionSimulatorFactory.h"

#include "AbstractPropertyManager.h"

using namespace std;

using namespace DataAccess;
using Interface::GridMap;
using Interface::Grid;
using Interface::Snapshot;
using Interface::Formation;
using Interface::Surface;
using Interface::Property;
using Interface::PropertyValue;
using Interface::PropertyValueList;
using Interface::AttributeValue;
using Interface::LithoType;

using Utilities::Maths::CgrConversionFactor;
using Utilities::Maths::GorConversionFactor;
using Utilities::Maths::KilogrammeToUSTon;
using Utilities::Maths::CubicMetresToCubicFeet;
using Utilities::Maths::CubicMetresToBarrel;
using Utilities::Maths::CubicMetresToCubicFeet;
using Utilities::Maths::MegaPaToPa;
using Utilities::Numerical::CauldronNoDataValue;

namespace Genex6
{
class GenexSimulator;

std::map<std::string, std::string> GenexSourceRock::s_CfgFileNameBySRType;

const double GenexSourceRock::conversionCoeffs [8] =
   { -2.60832073307101E-05, 0.236463623513642,
     -0.0319467563289369, 0.00185738251210839,
     2.36948559032296E-05, -6.62225531134738E-06,
     2.38411451425613E-07, -2.692340754443E-09 };

GenexSourceRock::GenexSourceRock (Interface::ProjectHandle * projectHandle, database::Record * record)
: Interface::SourceRock (projectHandle, record)
{
   m_theSimulator = 0;
   m_formation = 0;

   if(s_CfgFileNameBySRType.empty()) {
      initializeCfgFileNameBySRType();
   }

   m_adsorptionSimulator = 0;
   m_adsorptionSimulator2 = 0;

   m_theChemicalModel  = 0;
   m_theChemicalModel1 = 0;
   m_theChemicalModel2 = 0;

   m_applySRMixing = false;
   m_sourceRockEndMember1 = 0;
   m_sourceRockEndMember2 = 0;
   m_tocOutputMap = 0;
   m_isSulphur = false;
}

GenexSourceRock::~GenexSourceRock(void)
{
   clear();
   if(!s_CfgFileNameBySRType.empty()) {
      s_CfgFileNameBySRType.clear();
   }
}

void GenexSourceRock::getHIBounds( double &HILower, double &HIUpper ) {
   HILower = 28.47;
   HIUpper = 773.6;
}

double GenexSourceRock::convertHCtoHI( double aHC ) {

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

double GenexSourceRock::convertHItoHC( double aHI ) {

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

void GenexSourceRock::initializeCfgFileNameBySRType()
{
   //BPA1 mappings
   s_CfgFileNameBySRType["LacustrineAlgal"]         = "TypeI";
   s_CfgFileNameBySRType["MesozoicMarineShale"]     = "TypeII";
   s_CfgFileNameBySRType["MesozoicCalcareousShale"] = "TypeIIS";
   s_CfgFileNameBySRType["PaleozoicMarineShale"]    = "TypeIIHS";
   s_CfgFileNameBySRType["VitriniticCoals"]         = "TypeIII";

   s_CfgFileNameBySRType["Type_I_CenoMesozoic_Lacustrine_kin"] = "TypeI";
   s_CfgFileNameBySRType["Type_I_II_Mesozoic_MarineShale_lit"] = "TypeI_II";
   s_CfgFileNameBySRType["Type_I_II_Cenozoic_Marl_kin"]        = "TypeIIN";
   s_CfgFileNameBySRType["Type_II_Mesozoic_MarineShale_kin"]   = "TypeII";
   s_CfgFileNameBySRType["Type_II_Paleozoic_MarineShale_kin"]  = "TypeIIHS";
   s_CfgFileNameBySRType["Type_II_Mesozoic_Marl_kin"]          = "TypeIIS";
   s_CfgFileNameBySRType["Type_III_II_Mesozoic_HumicCoal_lit"] = "TypeII_III";
   s_CfgFileNameBySRType["Type_III_MesoPaleozoic_VitriniticCoal_kin"] = "TypeIII";

   s_CfgFileNameBySRType["Type_I_CenoMesozoic_Lacustrine_kin_s"] = "TypeI";
   s_CfgFileNameBySRType["Type_I_II_Mesozoic_MarineShale_lit_s"] = "TypeI_II";
   s_CfgFileNameBySRType["Type_I_II_Cenozoic_Marl_kin_s"]        = "TypeIIN";
   s_CfgFileNameBySRType["Type_II_Mesozoic_MarineShale_kin_s"]   = "TypeII";
   s_CfgFileNameBySRType["Type_II_Paleozoic_MarineShale_kin_s"]  = "TypeIIHS";
   s_CfgFileNameBySRType["Type_II_Mesozoic_Marl_kin_s"]          = "TypeIIS";
   s_CfgFileNameBySRType["Type_III_II_Mesozoic_HumicCoal_lit_s"] = "TypeII_III";

   //BPA2 mappings
   s_CfgFileNameBySRType["Type I - Lacustrine"]         = "TypeI";
   s_CfgFileNameBySRType["Type I/II - Marine Shale"]    = "TypeI_II";
   s_CfgFileNameBySRType["Type II - Mesozoic Marine"]   = "TypeII";
   s_CfgFileNameBySRType["Type II - Marine Marl"]       = "TypeIIS";
   s_CfgFileNameBySRType["Type II - Paleozoic Marine"]  = "TypeIIHS";
   s_CfgFileNameBySRType["Type II/III - Humic Coal"]    = "TypeII_III";
   s_CfgFileNameBySRType["Type III - Terrestrial Coal"] = "TypeIII";
}

void GenexSourceRock::setPropertyManager ( AbstractDerivedProperties::AbstractPropertyManager * aPropertyManager ) {

   m_propertyManager = aPropertyManager;
}

void GenexSourceRock::clear()
{
   clearSnapshotIntervals();
   clearSimulator();
   clearSourceRockNodes();
   clearSourceRockNodeAdsorptionHistory ();

   m_theSnapShotOutputMaps.clear();
   m_adsorpedOutputMaps.clear ();
   m_expelledOutputMaps.clear ();
   m_sourceRockExpelledOutputMaps.clear ();
   m_freeOutputMaps.clear ();
   m_retainedOutputMaps.clear ();

   if ( m_theChemicalModel2 != 0 ) {
      delete m_theChemicalModel2;
      m_theChemicalModel2 = 0;
   }  
   m_theChemicalModel = 0;

   if ( m_adsorptionSimulator != 0 ) {
      delete m_adsorptionSimulator;
      m_adsorptionSimulator = 0;
   }
   if ( m_adsorptionSimulator2 != 0 ) {
      delete m_adsorptionSimulator2;
      m_adsorptionSimulator2 = 0;
   }

   m_hcSaturationOutputMap = 0;
   m_irreducibleWaterSaturationOutputMap = 0;

   m_gasVolumeOutputMap = 0;
   m_oilVolumeOutputMap = 0;
   m_gasExpansionRatio = 0;
   m_gasGeneratedFromOtgc = 0;
   m_totalGasGenerated = 0;

   m_fracOfAdsorptionCap = 0;
   m_hcLiquidSaturation = 0;
   m_hcVapourSaturation = 0;
   m_adsorptionCapacity = 0;

   m_retainedOilApiOutputMap = 0;
   m_retainedCondensateApiOutputMap = 0;

   m_retainedGor = 0;
   m_retainedCgr = 0;

   m_overChargeFactor = 0;
   m_porosityLossDueToPyrobitumen = 0;
   m_h2sRisk = 0;
   m_tocOutputMap = 0;

   m_sourceRockEndMember1 = 0;
   m_sourceRockEndMember2 = 0;

   m_layerName = "";
}
void GenexSourceRock::clearSimulator()
{
   if(m_theSimulator) {
      // set ChemicalModel1 to be deleted inside Simulator destructor
      m_theSimulator->setChemicalModel( m_theChemicalModel1 );

      delete m_theSimulator;
      m_theSimulator = 0;

      m_theChemicalModel1 = 0; 
      m_theChemicalModel  = 0;
   }
   if ( m_adsorptionSimulator != 0 ) {
      delete m_adsorptionSimulator;
      m_adsorptionSimulator = 0;
   }
   if ( m_adsorptionSimulator2 != 0 ) {
      delete m_adsorptionSimulator2;
      m_adsorptionSimulator2 = 0;
   }
}
void GenexSourceRock::clearSnapshotIntervals()
{
   std::vector<SnapshotInterval*>::iterator itEnd = m_theIntervals.end();
   for(std::vector<SnapshotInterval*>::iterator it = m_theIntervals.begin(); it != itEnd; ++ it) {
     delete (*it); 
   }
   m_theIntervals.clear();
}
void GenexSourceRock::clearSourceRockNodes()
{

   std::vector<Genex6::SourceRockNode*>::iterator itEnd = m_theNodes.end();

   for(std::vector<Genex6::SourceRockNode*>::iterator it = m_theNodes.begin(); it !=itEnd; ++ it) {
      delete (*it);
   }
   m_theNodes.clear();
}
double GenexSourceRock::getDepositionTime() const
{
   const Surface * topSurface = m_formation->getTopSurface ();
   const Snapshot * depoSnapshot = topSurface->getSnapshot ();
   return depoSnapshot->getTime ();
}
bool GenexSourceRock::doOutputAdsorptionProperties( void ) const
{
   if( m_applySRMixing ) {
      const Interface::SourceRock * sourceRock2 =  m_formation->getSourceRock2();
      return ( doApplyAdsorption() || sourceRock2->doApplyAdsorption());
   }
   return doApplyAdsorption();
  
}

AdsorptionSimulator * GenexSourceRock::getAdsorptionSimulator() const 
{
   if( m_adsorptionSimulator  != 0 ) return m_adsorptionSimulator;
   if( m_adsorptionSimulator2 != 0 ) return m_adsorptionSimulator2;

   return 0;
}

bool GenexSourceRock::setFormationData( const Interface::Formation * aFormation ) 
{
   setLayerName( aFormation->getName() );

   if( m_layerName == "" ) {
      LogHandler( LogHandler::ERROR_SEVERITY ) << "Cannot compute SourceRock " << getType() << ": the formation name is not set.";
      return false;
   }

   m_formation = aFormation; //m_projectHandle->findFormation (m_layerName);
      
   if(!m_formation->isSourceRock()) { // if SourceRock is currently inactive
      return true;
   }
   
   if( m_formation->getTopSurface()->getSnapshot()->getTime() == 0 ) {
      LogHandler( LogHandler::ERROR_SEVERITY ) << "Cannot compute SourceRock with deposition age 0 at : " << m_formation->getName();
      return false;
   }
      
   // m_isSulphur is used only to identify Sulphur in output properties
   // For SR mixing we should check both SoureRock types and then set m_isSulphur
   
   m_isSulphur = ( getScVRe05() > 0.0 ? true : false );
   
   m_applySRMixing = m_formation->getEnableSourceRockMixing();

   if( m_applySRMixing ) {
      const Interface::SourceRock * sourceRock2 =  m_formation->getSourceRock2();
      if( sourceRock2 == 0 ) {
         LogHandler( LogHandler::ERROR_SEVERITY ) << "Cannot find SourceRockType2 "<< m_formation->getSourceRockType2Name() << " for mixing at : " << m_layerName;
         return false;
      } else {
         if( ! m_isSulphur ) {
            m_isSulphur = ( sourceRock2->getScVRe05() > 0.0 ? true : false ); 
         }
      }
   }
   
   return true;
}

bool GenexSourceRock::compute()
{
   bool status = true;

   LogHandler( LogHandler::INFO_SEVERITY ) << "Ready to compute SourceRock at : "<< m_formation->getName();

   if(status) status = initialize();

   if(status) status = preprocess();

   if(status) status = addHistoryToNodes ();

   if(status) status = process();
   
   return status;
}

ChemicalModel * GenexSourceRock::loadChemicalModel( const Interface::SourceRock * theSourceRock,
                                               const bool printInitialisationDetails ) {

   double theScIni = theSourceRock->getScVRe05();
   
   double in_SC = (theScIni != 0.0 ? ( validateGuiValue(theScIni, 0.01, 0.09) == true ? theScIni : 0.03 ) : 0.0 );

   int runType = ( in_SC != 0.0 ? Genex6::Constants::SIMGENEX : (Genex6::Constants::SIMGENEX | Genex6::Constants::SIMGENEX5) );

   const string & SourceRockType = theSourceRock->getBaseSourceRockType();
   const string & theType = ( SourceRockType.empty() ? determineConfigurationFileName( theSourceRock->getType() ) :
                              determineConfigurationFileName( SourceRockType ) );

   std::string fullPath;
   char *GENEXDIR = 0;
   char *MY_GENEX5DIR = getenv("MY_GENEX5DIR");

   if( runType & Genex6::Constants::SIMGENEX5 ) { 
      GENEXDIR = getenv("GENEX5DIR"); 
   } else {
      GENEXDIR = getenv("GENEX6DIR"); 
   }
   
   if(MY_GENEX5DIR != 0) {
      fullPath = MY_GENEX5DIR;
   } else if(GENEXDIR != 0) {
       fullPath = GENEXDIR;
   } else {
      LogHandler( LogHandler::ERROR_SEVERITY ) << "Environment Variable " << (runType & Genex6::Constants::SIMGENEX5 ? "GENEX5DIR" : "GENEX6DIR") << " not set. Aborting...";
      exit(1); //TODO_SK: gracefully exit. Note: this statement used to be: return false; which crashes the application as a pointer should be returned.
   }

   double theHcIni = theSourceRock->getHcVRe05();  
   double theEact  = 1000.0 * theSourceRock->getPreAsphaltStartAct();

   double theAsphalteneDiffusionEnergy = 1000.0 * theSourceRock->getAsphalteneDiffusionEnergy();
   double theResinDiffusionEnergy  = 1000.0 * theSourceRock->getResinDiffusionEnergy();
   double theC15AroDiffusionEnergy = 1000.0 * theSourceRock->getC15AroDiffusionEnergy();
   double theC15SatDiffusionEnergy = 1000.0 * theSourceRock->getC15SatDiffusionEnergy();

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

   ChemicalModel * theChemicalModel = m_theSimulator->loadChemicalModel(fullPath, runType, theType,
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
      LogHandler( LogHandler::INFO_SEVERITY ) << "Configuration File: " << theType <<  (!(runType & Genex6::Constants::SIMGENEX5) ? " (with sulphur)" : "");
   } 

   return theChemicalModel;
}
bool GenexSourceRock::validateGuiValue(const double GuiValue, const double LowerBound,const double UpperBound)
{
   if(GuiValue > (LowerBound - Constants::Zero) &&
      GuiValue < (UpperBound + Constants::Zero) &&
      GuiValue > 0.0) {
      return true; 
   }
   return false;
}
const string & GenexSourceRock::determineConfigurationFileName(const string & SourceRockType)
{ 
   static string ret("TypeI");

   std::map<std::string, std::string>::iterator it = s_CfgFileNameBySRType.find(SourceRockType);

   if(it != s_CfgFileNameBySRType.end()) {
      return it->second;
   } else {
      LogHandler( LogHandler::WARNING_SEVERITY ) << "Source rock " << SourceRockType << " not found. Setting configuration file to TypeI.";
   }
   return ret;
}
bool GenexSourceRock::initialize ( const bool printInitialisationDetails )
{
   bool status = true;

   if(printInitialisationDetails ) {
      LogHandler( LogHandler::INFO_SEVERITY ) << "Start Of Initialization...";
   }

   if( m_theSimulator == 0 ) {
      m_theSimulator = new Genex6::Simulator() ;
      m_theChemicalModel1 = loadChemicalModel( this, printInitialisationDetails );
   }

   assert ( m_theSimulator != 0 );
   m_theSimulator->setChemicalModel( m_theChemicalModel1 );
   m_theChemicalModel =  m_theChemicalModel1;
   double maximumTimeStepSize1 = m_theSimulator->getMaximumTimeStepSize();
   int numberOfTimeSteps1   =  m_theSimulator->getNumberOfTimesteps();

   if ( m_theSimulator != 0 and  m_applySRMixing ) {

      const Interface::SourceRock * sourceRock2 = m_formation->getSourceRock2();
      m_theChemicalModel2 = loadChemicalModel( sourceRock2, printInitialisationDetails );

      assert ( m_theChemicalModel2 != 0 );

      // How to choose the timeStepSize and numberOfTimeSteps? 
      // If hcValueMixing == HC value of one of the mixed SourceRock, then fraction of the second SourceRock type is 0, 
      // therfore we can choose timeStepSize and numberOfTimeSteps of the first Source Rock.
      // If hcValueMixing != HC, then choose the minimun timeStep and maximum number of time steps.

      double maximumTimeStepSize2 = m_theSimulator->getMaximumTimeStepSize();
      int numberOfTimeSteps2   =  m_theSimulator->getNumberOfTimesteps();

      double hcValue1 = getHcVRe05();
      double hcValue2 = sourceRock2->getHcVRe05();

      if(  m_formation->getSourceRockMixingHCGridName().length() == 0 ) {
         if( m_formation->getSourceRockMixingHC() == Interface::DefaultUndefinedScalarValue ) {
            status = false;
            LogHandler( LogHandler::ERROR_SEVERITY ) << "The mixing HC value is undefined. Aborting...";
         } else {
            double hcValueMixing = ( m_formation->getSourceRockMixingHC() != 0 ? m_formation->getSourceRockMixingHC() : 0 ); 
      
            if( hcValue1 == hcValueMixing ) {
               m_theSimulator->setMaximumTimeStepSize( maximumTimeStepSize1 );
               m_theSimulator->setNumberOfTimesteps( numberOfTimeSteps1 );
            } else if ( hcValue2 != hcValueMixing ) {
               if( maximumTimeStepSize1 < maximumTimeStepSize2 ) m_theSimulator->setMaximumTimeStepSize( maximumTimeStepSize1 );
               if( numberOfTimeSteps1 > numberOfTimeSteps2 ) m_theSimulator->setNumberOfTimesteps( numberOfTimeSteps1 );
               else m_theSimulator->setNumberOfTimesteps( numberOfTimeSteps1 );
            }
            if ( printInitialisationDetails ) {
               LogHandler( LogHandler::INFO_SEVERITY ) << "Applying Source Rock mixing H/C = " << hcValueMixing;
            }
         }
      } else {
         if( maximumTimeStepSize1 < maximumTimeStepSize2 ) m_theSimulator->setMaximumTimeStepSize( maximumTimeStepSize1 );
         if( numberOfTimeSteps1 > numberOfTimeSteps2 ) m_theSimulator->setNumberOfTimesteps( numberOfTimeSteps1 );
         else m_theSimulator->setNumberOfTimesteps( numberOfTimeSteps1 );
         
         if ( printInitialisationDetails ) {
            LogHandler( LogHandler::INFO_SEVERITY ) << "Applying Source Rock mixing with HC mixing map";
         }
      }
      if( status ) {
         
         status = m_theChemicalModel2->Validate();
         
         if(  m_theChemicalModel1->GetNumberOfSpecies() < m_theChemicalModel2->GetNumberOfSpecies() ) {
            m_theChemicalModel = m_theChemicalModel2;
         }
         
         if(printInitialisationDetails ) {
            if(status) {
               LogHandler( LogHandler::INFO_SEVERITY ) << "End Of Initialization.";
               LogHandler( LogHandler::INFO_SEVERITY ) << "-------------------------------------";
            } else {
               LogHandler( LogHandler::ERROR_SEVERITY ) << "Invalid Chemical Model. Please check your source rock " << m_formation->getSourceRockType2Name() << " input parameters. Aborting...";
               LogHandler( LogHandler::INFO_SEVERITY ) << "----------------------------------------------------------------------------------";
            }
         }
      }
   }
   if ( status && m_theSimulator != 0 and doApplyAdsorption ()) {

      if ( printInitialisationDetails ) {
         LogHandler( LogHandler::INFO_SEVERITY ) << "Applying adsorption, TOCDependent is " << (adsorptionIsTOCDependent () ? "true" : "false" )
            << ", function is " << getAdsorptionCapacityFunctionName ()
            << ", OTGC is " << ( doComputeOTGC () ? "on" : "off" );
      }

      AdsorptionFunction*  adsorptionFunction = AdsorptionFunctionFactory::getInstance ().getAdsorptionFunction ( m_projectHandle,
                                                                                                                  adsorptionIsTOCDependent (),
                                                                                                                  getAdsorptionCapacityFunctionName ());

      m_adsorptionSimulator = AdsorptionSimulatorFactory::getInstance ().getAdsorptionSimulator ( m_projectHandle,
                                                                                                  m_theChemicalModel1->getSpeciesManager(),
                                                                                                  getAdsorptionSimulatorName (),
                                                                                                  doComputeOTGC (),
                                                                                                  false );

      status = status and adsorptionFunction->isValid ();

      if ( not adsorptionFunction->isValid () and printInitialisationDetails ) {
         LogHandler( LogHandler::ERROR_SEVERITY ) << "Invalid adsorption function. Please check adsorption function parameters. Aborting ...";
         LogHandler( LogHandler::ERROR_SEVERITY ) << adsorptionFunction->getErrorMessage ();
      }


      assert ( m_adsorptionSimulator != 0 );
      assert ( adsorptionFunction != 0 );

      m_adsorptionSimulator->setAdsorptionFunction ( adsorptionFunction );

      // Since it was allocated with managed = true, the adsorption-simulator 
      // must be deleted when the source-rock destructor is called.
      // It will not be deleted in the simulators destructor.
      //  m_theSimulator->addSubProcess ( m_adsorptionSimulator );
   }
   if( m_applySRMixing && status ) {

      const Interface::SourceRock * sourceRock2 =  m_formation->getSourceRock2();
      
      if( sourceRock2->doApplyAdsorption () ) {
         LogHandler( LogHandler::ERROR_SEVERITY ) << "Applying adsorption for SourceRock type 2, TOCDependent is "
            << (sourceRock2->adsorptionIsTOCDependent () ? "true" : "false" ) << ", function is "
            << sourceRock2->getAdsorptionCapacityFunctionName () << ", OTGC is " << (sourceRock2->doComputeOTGC () ? "on" : "off" );

         AdsorptionFunction*  adsorptionFunction = AdsorptionFunctionFactory::getInstance ().getAdsorptionFunction ( m_projectHandle,
                                                                                                                     sourceRock2->adsorptionIsTOCDependent (),
                                                                                                                     sourceRock2->getAdsorptionCapacityFunctionName ());
         
         m_adsorptionSimulator2 = AdsorptionSimulatorFactory::getInstance ().getAdsorptionSimulator ( m_projectHandle,
                                                                                                      m_theChemicalModel2->getSpeciesManager(),
                                                                                                      sourceRock2->getAdsorptionSimulatorName (),
                                                                                                      sourceRock2->doComputeOTGC (),
                                                                                                      false );
         
         assert ( m_adsorptionSimulator2 != 0 );
         assert ( adsorptionFunction != 0 );

         status = status and adsorptionFunction->isValid ();

         if ( not adsorptionFunction->isValid () and printInitialisationDetails ) {
            LogHandler( LogHandler::ERROR_SEVERITY ) << "Invalid adsorption function. Please check adsorption function parameters. Aborting ...";
            LogHandler( LogHandler::ERROR_SEVERITY ) << adsorptionFunction->getErrorMessage ();
         }

         m_adsorptionSimulator2->setAdsorptionFunction ( adsorptionFunction );
      }
   }

   if( status ) {
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
 
   return status;
}
bool GenexSourceRock::preprocess()
{
   bool status = true;

   LogHandler( LogHandler::INFO_SEVERITY ) << "Start of preprocessing...";

   computeSnapshotIntervals ();

   const SnapshotInterval *last = m_theIntervals.back ();
   const Snapshot *presentDay = last->getEnd ();

   const GridMap *tempAtPresentDay = getTopSurfacePropertyGridMap ("Temperature", presentDay);
   const GridMap *vre              = getSurfaceFormationPropertyGridMap ("Vr", presentDay);

   GridMap * tempMap = 0;
   if( tempAtPresentDay == 0 ) {
      const DataModel::AbstractProperty *property = m_propertyManager->getProperty( "Temperature" );
      AbstractDerivedProperties::SurfacePropertyPtr surfaceProperty = m_propertyManager->getSurfaceProperty ( property,
                                                                                                              presentDay,
                                                                                                              m_formation->getTopSurface () );
      if ( surfaceProperty != 0 ) {
         tempMap = getProjectHandle()->getFactory()->produceGridMap ( 0, 0,
                                                                      getProjectHandle()->getActivityOutputGrid (),
                                                                      surfaceProperty->getUndefinedValue(), 1 );
         if( tempMap != 0 ) {
            tempMap->retrieveData();

            for ( unsigned int i = tempMap->firstI (); i <= tempMap->lastI (); ++i ) {
               for ( unsigned int j = tempMap->firstJ (); j <= tempMap->lastJ (); ++j ) {
                  tempMap->setValue (i, j, surfaceProperty->get( i, j ));
               }
            }
            tempMap->restoreData (true);
         }
      }
   }
   const GridMap * temperatureAtPresentDay = ( tempAtPresentDay ? tempAtPresentDay : tempMap );

   GridMap * vreMap = 0;
   if( vre == 0 ) {
      const DataModel::AbstractProperty *property = m_propertyManager->getProperty( "Vr" );
      AbstractDerivedProperties::FormationSurfacePropertyPtr surfaceProperty = m_propertyManager->getFormationSurfaceProperty ( property, presentDay, 
                                                                                                                        m_formation, m_formation->getTopSurface () );
      if( surfaceProperty != 0 ) {
         vreMap = getProjectHandle()->getFactory()->produceGridMap ( 0, 0, 
                                                                          getProjectHandle()->getActivityOutputGrid (), 
                                                                          surfaceProperty->getUndefinedValue(), 1 );
         if ( vreMap ) {
            vreMap->retrieveData();
            
            for ( unsigned int i = vreMap->firstI (); i <= vreMap->lastI (); ++i ) {
               for ( unsigned int j = vreMap->firstJ (); j <= vreMap->lastJ (); ++j ) {
                  vreMap->setValue (i, j, surfaceProperty->get( i, j ));
               }
            }
            
            vreMap->restoreData (true);
         }      
      }
   }
   const GridMap * VREPresentDay = ( vre ? vre : vreMap );
   
   status = GenexSourceRock::preprocess ( temperatureAtPresentDay, VREPresentDay );
   
   if( ! status ) {
      if ( temperatureAtPresentDay == 0 ) {
         LogHandler( LogHandler::ERROR_SEVERITY ) << "Unsuccessful upload of temperature property for layer : " << getLayerName () <<
            " Terminating preprocessing...";
      } else if ( VREPresentDay == 0 ) {
         LogHandler( LogHandler::ERROR_SEVERITY ) << "Unsuccessful upload of VRe property for layer : " << getLayerName () <<
            " Terminating preprocessing...";
      }
   }
   if ( tempMap != 0 ) delete tempMap;
   if ( vreMap != 0 )  delete vreMap;
   return status;
}

bool GenexSourceRock::preprocess ( const DataAccess::Interface::GridMap* validityMap,
                              const DataAccess::Interface::GridMap* vre,
                              const bool printInitialisationDetails ) {

   bool status = true;

      

   //load thickness in ActivityOutputGrid
   const GridMap *InputThickness = getInputThicknessGridMap ();

   //TOC in ActivityOutputGrid
   Interface::SourceRockMapAttributeId dataIndex = Interface::TocIni;
   const Interface::GridMap* TOCmap = dynamic_cast< const Interface::GridMap*>(getMap (dataIndex));

   bool isVreOn = isVREoptimEnabled ();

   //
   double f1, f2;
   const GridMap *HCmap = 0;
   double Hc1 = 0.0, Hc2 = 0.0, invValue = 1.0, minHc = 0.0, maxHc = 0.0; 

   bool testPercentage = false;

   if( m_applySRMixing ) {
      HCmap = m_formation->getMixingHCMap ();
      Hc1 = getHcVRe05();
      Hc2 = m_theChemicalModel2->getHC();

      // here do all checking for h_c1 and h_c2 (zero, equal, positive and all everything...) ? Or not?
      // assume, that all to be done in BPA
         
      if( fabs( Hc1 - Hc2 ) <= Constants::Zero ) {
         if( !testPercentage ) {
            status = false;
            if (printInitialisationDetails ) {
               LogHandler( LogHandler::ERROR_SEVERITY ) << "SourceRock Type1 H/C is equal SourceRock Type2 H/C. Terminating preprocessing...";
            } 
            return status;	
         } else {
            if ( printInitialisationDetails ) {
               LogHandler( LogHandler::INFO_SEVERITY ) << "SourceRock Type1 H/C is equal SourceRock Type2 H/C. Run the percentage...";
            }

         }
      } else {
         invValue = 1.0 / ( Hc1 - Hc2 );
         minHc = min( Hc1, Hc2 );
         maxHc = max( Hc1, Hc2 );
      }
   }


   if ( validityMap != 0 && InputThickness != 0 && TOCmap != 0 )
   {
      validityMap->retrieveData ();
      InputThickness->retrieveData ();
      TOCmap->retrieveData ();

      if( HCmap != 0 ) HCmap->retrieveData();

      if (isVreOn) {
         if(vre) {
            vre->retrieveData ();
         } else {
            status = false;
            LogHandler( LogHandler::ERROR_SEVERITY ) << "Unsuccessful upload of Vr property for layer : " << m_layerName <<
               " Terminating preprocessing...";
            return status;	
         }
      }

      double lithoDensity1, lithoDensity2, lithoDensity3;

      lithoDensity1 = lithoDensity2 = lithoDensity3 = 0.0;

      const Interface::LithoType* litho1 = getLithoType1 ();
      const Interface::LithoType* litho2 = getLithoType2 ();
      const Interface::LithoType* litho3 = getLithoType3 ();

      const Interface::GridMap* litho1PercentageMap = getLithoType1PercentageMap ();
      const Interface::GridMap* litho2PercentageMap = getLithoType2PercentageMap ();
      const Interface::GridMap* litho3PercentageMap = getLithoType3PercentageMap ();

      unsigned int depthlitho1PercentageMap, depthlitho2PercentageMap, depthlitho3PercentageMap;

      depthlitho1PercentageMap = depthlitho2PercentageMap = depthlitho3PercentageMap = 0;

      if (litho1 && litho1PercentageMap)
      {
         lithoDensity1 = getLithoDensity (litho1);
         litho1PercentageMap->retrieveData ();
         depthlitho1PercentageMap = litho1PercentageMap->getDepth ();
      }

      if (litho2 && litho2PercentageMap)
      {
         lithoDensity2 = getLithoDensity (litho2);
         litho2PercentageMap->retrieveData ();
         depthlitho2PercentageMap = litho2PercentageMap->getDepth ();
      }

      if (litho3 && litho3PercentageMap)
      {
         lithoDensity3 = getLithoDensity (litho3);
         litho3PercentageMap->retrieveData ();
         depthlitho3PercentageMap = litho3PercentageMap->getDepth ();
      }

      unsigned int lowResI, lowResJ;

      unsigned int depthValidity = validityMap->getDepth ();
      unsigned int depthVre = ( vre != 0 ? vre->getDepth () : 0 );
      unsigned int depthThickness = InputThickness->getDepth ();
      unsigned int depthTOC = TOCmap->getDepth ();
      unsigned int depthHc = ( HCmap ? HCmap->getDepth () : 0 );

      Interface::ModellingMode theMode = m_projectHandle->getModellingMode ();

      unsigned int endMapI = 0;
      unsigned int endMapJ = 0;

      if (Interface::MODE3D == theMode)
      {
         endMapI = validityMap->lastI ();
         endMapJ = validityMap->lastJ ();
      }
      else if (Interface::MODE1D == theMode)
      {
         endMapI = validityMap->firstI ();
         endMapJ = validityMap->firstJ ();
      }

      for (lowResI = validityMap->firstI (); lowResI <= endMapI; ++lowResI)
      {
         for (lowResJ = validityMap->firstJ (); lowResJ <= endMapJ; ++lowResJ)
         {
            double validValue = validityMap->getValue (lowResI, lowResJ, depthValidity - 1);
            double VreAtPresentDay = 0.0;

            if ( vre != 0 && isVreOn)
            {
               VreAtPresentDay = vre->getValue (lowResI, lowResJ, depthVre - 1);
            }
            double in_thickness = InputThickness->getValue (lowResI, lowResJ, depthThickness - 1);
            double in_TOC = TOCmap->getValue (lowResI, lowResJ, depthTOC - 1);
            double inorganicDensity = 0.0;

            if (litho1 && litho1PercentageMap) {
               inorganicDensity +=
                     lithoDensity1 * 0.01 * litho1PercentageMap->getValue (lowResI, lowResJ,
                                                                           depthlitho1PercentageMap - 1);
            }

            if (litho2 && litho2PercentageMap)
            {
               inorganicDensity +=
                     lithoDensity2 * 0.01 * litho2PercentageMap->getValue (lowResI, lowResJ,
                                                                           depthlitho2PercentageMap - 1);
            }
            if (litho3 && litho3PercentageMap) {
               inorganicDensity +=
                     lithoDensity3 * 0.01 * litho3PercentageMap->getValue (lowResI, lowResJ,
                                                                           depthlitho3PercentageMap - 1);
            }

            if ( isNodeValid ( validValue, VreAtPresentDay, in_thickness, in_TOC, inorganicDensity,
                               Interface::DefaultUndefinedMapValue))
            {

              if (!isNodeActive (VreAtPresentDay, in_thickness, in_TOC, inorganicDensity, validValue))
              {
                // not sure if this still constitutes an optimization.....
                in_thickness = 0;
                in_TOC = 0;
                inorganicDensity = 0;
              }
               
               if( HCmap != 0 ) {

                  double hcValue = HCmap->getValue (lowResI, lowResJ, depthHc - 1);
                  if( hcValue != Interface::DefaultUndefinedMapValue ) {
                     if( testPercentage ) {
                        f1 = hcValue;
                     } else {
                        if(( minHc > hcValue ) || ( maxHc < hcValue ) ) { // may be it's not necessary if already has been done in BPA..
                           status = false;
                           LogHandler( LogHandler::ERROR_SEVERITY ) << "HC map value  " << hcValue << " is out of range: H/C1 = " << Hc1 << " and H/C2 = " << Hc2 << ".";
                           break;
                        }
                        if( hcValue == Hc1 ) { f1 = 1.0; }
                        else if (  hcValue == Hc2 ) { f1 = 0.0; }
                        else {
                           f1 = ( hcValue - Hc2 ) * invValue;
                        }
                     }
                     f2 = 1.0 - f1;
                  } else {
                     f1 = f2 =  Interface::DefaultUndefinedMapValue;
                  }
                     
               } else {
                  f1 = f2 = 0.0;
               }
               
               
               Genex6::SourceRockNode * theNode = new Genex6::SourceRockNode (in_thickness, in_TOC, inorganicDensity, f1, f2, lowResI, lowResJ);
               addNode (theNode);
            }

         }

      }
#ifdef OBSOLETE
      if (Interface::MODE1D == theMode) {
         if (m_theNodes.empty ()) {
            // This is a perfectly legitimate situation!!!
            status = false;
            LogHandler( LogHandler::WARNING_SEVERITY ) << "No valid Source Rock Nodes. Terminating preprocessing...";
         }
      }
#endif

      //restore local map data
      validityMap->restoreData ();
      InputThickness->restoreData ();
      TOCmap->restoreData ();

      if( HCmap ) HCmap->restoreData();

      if (vre && isVreOn) {
         vre->restoreData ();
      }
      if (litho1PercentageMap) {
         litho1PercentageMap->restoreData ();
      }
      if (litho2PercentageMap) {
         litho2PercentageMap->restoreData ();
      }
      if (litho3PercentageMap) {
         litho3PercentageMap->restoreData ();
      }

      if (printInitialisationDetails ) {

         if( status ) {
            LogHandler( LogHandler::INFO_SEVERITY ) << "End of preprocessing.";
            LogHandler( LogHandler::INFO_SEVERITY ) << "-------------------------------------";
         } else {
            LogHandler( LogHandler::ERROR_SEVERITY ) << "Terminating preprocessing...";
         }

      }

   }
   else
   {
      status = false;

      if ( validityMap == 0) {
         LogHandler( LogHandler::ERROR_SEVERITY ) << "Unsuccessful upload of temperature property for layer : " << m_layerName <<
            " Terminating preprocessing...";
      } else if ( InputThickness == 0) {
         LogHandler( LogHandler::ERROR_SEVERITY ) << "Unsuccessful upload of thickness for layer : " << m_layerName << " Terminating preprocessing...";
      } else if ( TOCmap == 0) {
         LogHandler( LogHandler::ERROR_SEVERITY ) << "Unsuccessful upload of TOC for layer : " << m_layerName << " Terminating preprocessing...";
      } else {
         LogHandler( LogHandler::INFO_SEVERITY ) << "Terminating preprocessing...";
      }

   }

   return status;
}


bool GenexSourceRock::addHistoryToNodes () {

   std::vector<Genex6::SourceRockNode*>::iterator itEnd = m_theNodes.end();

   const double originX = m_projectHandle->getActivityOutputGrid ()->minIGlobal ();
   const double originY = m_projectHandle->getActivityOutputGrid ()->minJGlobal ();

   const double deltaX = m_projectHandle->getActivityOutputGrid ()->deltaIGlobal ();
   const double deltaY = m_projectHandle->getActivityOutputGrid ()->deltaJGlobal ();

   const double northEastCornerX = originX + m_projectHandle->getActivityOutputGrid ()->numIGlobal () * deltaX;
   const double northEastCornerY = originY + m_projectHandle->getActivityOutputGrid ()->numJGlobal () * deltaY;

   DataAccess::Interface::PointAdsorptionHistoryList* historyList = m_projectHandle->getPointAdsorptionHistoryList ( m_layerName );
   DataAccess::Interface::PointAdsorptionHistoryList::const_iterator historyIter;

   for ( historyIter = historyList->begin (); historyIter != historyList->end (); ++historyIter ) {

      const double x = (*historyIter)->getX ();
      const double y = (*historyIter)->getY ();

      if ( m_projectHandle->getModellingMode () == Interface::MODE1D ||
	    (NumericFunctions::inRange ( x, originX, northEastCornerX ) && NumericFunctions::inRange ( y, originY, northEastCornerY )) ){

         for(std::vector<Genex6::SourceRockNode*>::iterator it = m_theNodes.begin(); it !=itEnd; ++ it) { // and not historyAssociatedWithNode
            Genex6::SourceRockNode* node = *it;

            Genex6::NodeAdsorptionHistory* adsorptionHistory;

            int i = int ( floor (( x - originX ) / deltaX + 0.5 ));
            int j = int ( floor (( y - originY ) / deltaY + 0.5 ));

            if ( m_projectHandle->getModellingMode () == Interface::MODE1D ||
		  (i == (int)(node->GetI ()) && j == (int)(node->GetJ ()))) {

               if ( doOutputAdsorptionProperties ()) {
                  SourceRockAdsorptionHistory* history = new SourceRockAdsorptionHistory ( m_projectHandle, *historyIter );

                  adsorptionHistory = AdsorptionSimulatorFactory::getInstance ().allocateNodeAdsorptionHistory ( m_theChemicalModel->getSpeciesManager(), //m_theSimulator->getSpeciesManager (), 
                                                                                                                 m_projectHandle,
                                                                                                                 getAdsorptionSimulatorName ());

                  if ( adsorptionHistory != 0 ) {
                     // Add the node-adsorption-history object to the sr-history-object.
                     history->setNodeAdsorptionHistory ( adsorptionHistory );
                     node->addNodeAdsorptionHistory ( adsorptionHistory );
                     m_sourceRockNodeAdsorptionHistory.push_back ( history );
                  } else {
                     delete history;
                  }

               } else {
                  // Add Genex history 
                  SourceRockAdsorptionHistory* history = new SourceRockAdsorptionHistory ( m_projectHandle, *historyIter );
                  adsorptionHistory = AdsorptionSimulatorFactory::getInstance ().allocateNodeAdsorptionHistory ( m_theChemicalModel->getSpeciesManager(), 
                                                                                                                 m_projectHandle,
                                                                                                                 GenexSimulatorId );

                  if ( adsorptionHistory != 0 ) {
                     // Add the node-adsorption-history object to the sr-history-object.
                     history->setNodeGenexHistory ( adsorptionHistory );
                     node->addNodeAdsorptionHistory ( adsorptionHistory );
                     m_sourceRockNodeAdsorptionHistory.push_back ( history );
                  } else {
                     delete history;
                  }

               }

            }

         }

      } else {
         LogHandler( LogHandler::WARNING_SEVERITY ) << "History point (" << x << ", " << y << ") lies outside of the domain: ("
               << originX << ", " << originY << ") x (" << northEastCornerX << ", " << northEastCornerY << ")";
      }

   }

   delete historyList;

   return true;
}


void GenexSourceRock::saveSourceRockNodeAdsorptionHistory () {

   SourceRockAdsorptionHistoryList::iterator histIter;

   for ( histIter = m_sourceRockNodeAdsorptionHistory.begin (); histIter != m_sourceRockNodeAdsorptionHistory.end (); ++histIter ) {
      (*histIter)->save ();
   }

}

void GenexSourceRock::clearSourceRockNodeAdsorptionHistory () {

   SourceRockAdsorptionHistoryList::iterator histIter;

   for ( histIter = m_sourceRockNodeAdsorptionHistory.begin (); histIter != m_sourceRockNodeAdsorptionHistory.end (); ++histIter ) {
      delete *histIter;
   }

   m_sourceRockNodeAdsorptionHistory.clear ();
}

bool GenexSourceRock::isNodeActive ( const double VreAtPresentDay,
                                const double in_thickness,
                                const double in_TOC,
                                const double inorganicDensity,
                                const double temperatureAtPresentDay ) const
{
   bool ret = true;

   // Only active if thickness is greater than 1cm.
   if (in_thickness <= 0.01) ret = false;
   if (in_TOC < 0.0 or EqualDouble(in_TOC, 0.0)) ret = false;
   if (inorganicDensity < 0.0 or EqualDouble(inorganicDensity, 0.0)) ret = false;
   if (EqualDouble(temperatureAtPresentDay, 0.0)) ret = false;

   //and if the VRE optimization is enabled, 
   if (ret and isVREoptimEnabled ())
   {
      //the VRE value at present day must be defined...
      static const double &VREthreshold = getVREthreshold ();

      //and above the VRE threshold that was defined through the GUI...
      if (VreAtPresentDay <= (VREthreshold - 0.001))
      {
	 ret = false;
      }
   }

   return ret;
}

bool GenexSourceRock::isNodeValid(const double temperatureAtPresentDay, const double VreAtPresentDay, 
                             const double thickness, const double TOC, const double inorganicDensity, 
                             const double mapUndefinedValue) const
{
   using namespace Genex6;//the fabsEqualDouble and EqualDouble are defined in Utitilies.h of genex5_kernel
   bool ret = false;
   //A node needs to be defined in low res...
   if ((!fabsEqualDouble(temperatureAtPresentDay, mapUndefinedValue)) &&
      (!fabsEqualDouble(thickness, mapUndefinedValue)) &&
      (!fabsEqualDouble(TOC, mapUndefinedValue)) &&
      (!fabsEqualDouble(inorganicDensity, mapUndefinedValue))) {

      ret = true;

      //and if the VRE optimization is enabled, 
      if(isVREoptimEnabled()) {
         //the VRE value at present day must be defined...
         if(fabsEqualDouble(VreAtPresentDay, mapUndefinedValue)) {
            ret = false;
         }             
      }
   }  
   
   return ret;    
}

bool GenexSourceRock::process()
{
   bool status = true;
   const SnapshotInterval *first   = m_theIntervals.front();
   const Snapshot *simulationStart = first->getStart();
   double t                        = simulationStart->getTime();
   double previousTime;
   double dt                       = m_theSimulator->GetMaximumTimeStepSize(m_depositionTime);

   LogHandler( LogHandler::INFO_SEVERITY ) << "Chosen maximum timestep size:" << dt;
   LogHandler( LogHandler::INFO_SEVERITY ) << "-------------------------------------";

   LogHandler( LogHandler::INFO_SEVERITY ) << "Start Of processing...";
   LogHandler( LogHandler::INFO_SEVERITY ) << "-------------------------------------";

   m_runtime = 0.0;
   m_time = 0.0;

   //compute first snapshot 
   // d + dt is outside of the time-domain for the source-rock.
   // Should there be any computation at this point?
   // Should there be any output at this point: i) is anything generated? ii) most/all? maps will be filled with null/default /values.
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


void GenexSourceRock::initializeSnapShotOutputMaps ( const vector<string> & requiredPropertyNames,
                                                const vector<string> & theRequestedPropertyNames )
{
   using namespace CBMGenerics;
   GenexResultManager & theResultManager = GenexResultManager::getInstance();

   vector<string>::const_iterator it;

   for ( it = requiredPropertyNames.begin(); it != requiredPropertyNames.end(); ++it ) {
      m_theSnapShotOutputMaps[(*it)] =  0; 
   }

   for ( it = theRequestedPropertyNames.begin(); it != theRequestedPropertyNames.end(); ++it ) {
      //  we dont output Sulphur related properties if S/C = 0
      if(!( m_isSulphur == false && theResultManager.IsSulphurResult(*it) )) {
         //  we dont output SRF output if mixing is not applying
         if(!( m_applySRMixing == false && ((*it) == "SourceRockEndMember1" || (*it) == "SourceRockEndMember2"))) {
            m_theSnapShotOutputMaps[(*it)] =  0; 
         }
      } 
   }

   m_hcSaturationOutputMap = 0;
   m_irreducibleWaterSaturationOutputMap = 0;

   m_gasVolumeOutputMap = 0;
   m_oilVolumeOutputMap = 0;
   m_gasExpansionRatio = 0;
   m_gasGeneratedFromOtgc = 0;
   m_totalGasGenerated = 0;

   m_fracOfAdsorptionCap = 0;
   m_hcLiquidSaturation = 0;
   m_hcVapourSaturation = 0;
   m_adsorptionCapacity = 0;

   m_retainedOilApiOutputMap = 0;
   m_retainedCondensateApiOutputMap = 0;

   m_retainedGor = 0;
   m_retainedCgr = 0;

   m_overChargeFactor = 0;
   m_porosityLossDueToPyrobitumen = 0;
   m_h2sRisk = 0;
   m_tocOutputMap = 0;

   m_sourceRockEndMember1 = 0;
   m_sourceRockEndMember2 = 0;

  if ( m_theSimulator != 0 ) { // this piece of code is never executed. m_Simulator == 0 at this point

      if ( doOutputAdsorptionProperties ()) {
         const SpeciesManager& speciesManager = m_theSimulator->getSpeciesManager ();

         int speciesIndex;

         for ( speciesIndex = speciesManager.firstSpecies (); speciesIndex <= speciesManager.lastSpecies (); ++speciesIndex ) {
            ComponentManager::SpeciesNamesId species = speciesManager.mapIdToComponentManagerSpecies ( speciesIndex );

            if ( species != ComponentManager::UNKNOWN ) {

               if ( getAdsorptionSimulator()->speciesIsSimulated ( species )) {
                  m_adsorpedOutputMaps [ species ] = 0;
               }

               m_sourceRockExpelledOutputMaps [ species ] = 0;
               m_retainedOutputMaps [ species ] = 0;
            }

         }

         m_hcSaturationOutputMap = 0;
         m_irreducibleWaterSaturationOutputMap = 0;

         m_gasVolumeOutputMap = 0;
         m_oilVolumeOutputMap = 0;
         m_gasExpansionRatio = 0;
         m_gasGeneratedFromOtgc = 0;
         m_totalGasGenerated = 0;
         m_fracOfAdsorptionCap = 0;
         m_hcLiquidSaturation = 0;
         m_hcVapourSaturation = 0;
         m_adsorptionCapacity = 0;
         m_retainedOilApiOutputMap = 0;
         m_retainedCondensateApiOutputMap = 0;
         m_retainedGor = 0;
         m_retainedCgr = 0;
      }

   }


}
void GenexSourceRock::createSnapShotOutputMaps(const Snapshot *theSnapshot)
{
   using namespace CBMGenerics;

   std::map<std::string, GridMap*>::iterator it;

   for(it = m_theSnapShotOutputMaps.begin(); it != m_theSnapShotOutputMaps.end(); ++ it) { 
      
      GridMap *theMap = createSnapshotResultPropertyValueMap(it->first, theSnapshot);

      if(theMap) {   
         it->second = theMap;
         (it->second)->retrieveData();
      } else {

         LogHandler( LogHandler::ERROR_SEVERITY ) << "Unsuccessful creation of map :" << it->first;

      }

   }
   if( m_applySRMixing ) {
      std::map<std::string, GridMap*>::iterator it = m_theSnapShotOutputMaps.find("SourceRockEndMember1");
      if( it != m_theSnapShotOutputMaps.end() ) {
         m_sourceRockEndMember1 = it->second; 
         if ( m_sourceRockEndMember1 != 0 ) {
            m_sourceRockEndMember1->retrieveData ();
         }
      }
      
      it = m_theSnapShotOutputMaps.find("SourceRockEndMember2");
      if( it != m_theSnapShotOutputMaps.end() ) {
         m_sourceRockEndMember2 = it->second; 
         if ( m_sourceRockEndMember2 != 0 ) {
            m_sourceRockEndMember2->retrieveData ();
         }
      }
   } 

   it = m_theSnapShotOutputMaps.find("TOC");
   if( it != m_theSnapShotOutputMaps.end() ) {
      m_tocOutputMap = it->second; 
      if ( m_tocOutputMap != 0 ) {
         m_tocOutputMap->retrieveData ();
      }
   }
      
   if ( doOutputAdsorptionProperties ()) {
      GridMap* theMap;
      int speciesIndex;

      m_hcSaturationOutputMap = createSnapshotResultPropertyValueMap ( "HcSaturation",
                                                                       theSnapshot );

      if ( m_hcSaturationOutputMap != 0 ) {
         m_hcSaturationOutputMap->retrieveData ();
      }

      m_irreducibleWaterSaturationOutputMap = createSnapshotResultPropertyValueMap ( "ImmobileWaterSat",
                                                                                     theSnapshot );

      if ( m_irreducibleWaterSaturationOutputMap != 0 ) {
         m_irreducibleWaterSaturationOutputMap->retrieveData ();
      }

      m_adsorptionCapacity = createSnapshotResultPropertyValueMap ( "AdsorptionCapacity",
                                                                    theSnapshot );

      if ( m_adsorptionCapacity != 0 ) {
         m_adsorptionCapacity->retrieveData ();
      }


      m_fracOfAdsorptionCap = createSnapshotResultPropertyValueMap ( "FractionOfAdsorptionCap",
                                                                     theSnapshot );

      if ( m_fracOfAdsorptionCap != 0 ) {
         m_fracOfAdsorptionCap->retrieveData ();
      } 

      m_hcLiquidSaturation = createSnapshotResultPropertyValueMap ( "HcLiquidSat",
                                                                    theSnapshot );

      if ( m_hcLiquidSaturation != 0 ) {
         m_hcLiquidSaturation->retrieveData ();
      }

      m_hcVapourSaturation = createSnapshotResultPropertyValueMap ( "HcVapourSat",
                                                                    theSnapshot );

      if ( m_hcVapourSaturation != 0 ) {
         m_hcVapourSaturation->retrieveData ();
      }

      m_gasVolumeOutputMap = createSnapshotResultPropertyValueMap ( "RetainedGasVolumeST",
                                                                    theSnapshot );

      if ( m_gasVolumeOutputMap != 0 ) {
         m_gasVolumeOutputMap->retrieveData ();
      }

      m_oilVolumeOutputMap = createSnapshotResultPropertyValueMap ( "RetainedOilVolumeST",
                                                                    theSnapshot );

      if ( m_oilVolumeOutputMap != 0 ) {
         m_oilVolumeOutputMap->retrieveData ();
      }

      m_gasExpansionRatio = createSnapshotResultPropertyValueMap ( "GasExpansionRatio_Bg",
                                                                   theSnapshot );
      
      if ( m_gasExpansionRatio != 0 ) {
         m_gasExpansionRatio->retrieveData ();
      }

      m_gasGeneratedFromOtgc = createSnapshotResultPropertyValueMap ( "Oil2GasGeneratedCumulative",
                                                                      theSnapshot );
      
      if ( m_gasGeneratedFromOtgc != 0 ) {
         m_gasGeneratedFromOtgc->retrieveData ();
      }

      m_totalGasGenerated = createSnapshotResultPropertyValueMap ( "TotalGasGeneratedCumulative",
                                                                   theSnapshot );
      
      if ( m_totalGasGenerated != 0 ) {
         m_totalGasGenerated->retrieveData ();
      }

      m_retainedOilApiOutputMap = createSnapshotResultPropertyValueMap ( "RetainedOilApiSR",
                                                                         theSnapshot );

      if ( m_retainedOilApiOutputMap != 0 ) {
         m_retainedOilApiOutputMap->retrieveData ();
      }

      m_retainedCondensateApiOutputMap = createSnapshotResultPropertyValueMap ( "RetainedCondensateApiSR",
                                                                                theSnapshot );

      if ( m_retainedCondensateApiOutputMap != 0 ) {
         m_retainedCondensateApiOutputMap->retrieveData ();
      }

      m_retainedGor = createSnapshotResultPropertyValueMap ( "RetainedGorSR",
                                                             theSnapshot );

      if ( m_retainedGor != 0 ) {
         m_retainedGor->retrieveData ();
      }

      m_retainedCgr = createSnapshotResultPropertyValueMap ( "RetainedCgrSR",
                                                             theSnapshot );

      if ( m_retainedCgr != 0 ) {
         m_retainedCgr->retrieveData ();
      }

      m_overChargeFactor = createSnapshotResultPropertyValueMap ( "OverChargeFactor",
                                                                  theSnapshot );

      if ( m_overChargeFactor != 0 ) {
         m_overChargeFactor->retrieveData ();
      }

      m_porosityLossDueToPyrobitumen = createSnapshotResultPropertyValueMap ( "PorosityLossFromPyroBitumen",
                                                                              theSnapshot );

      if ( m_porosityLossDueToPyrobitumen != 0 ) {
         m_porosityLossDueToPyrobitumen->retrieveData ();
      }

      if ( m_isSulphur ) {
         m_h2sRisk = createSnapshotResultPropertyValueMap ( "H2SRisk",
                                                            theSnapshot );

         if ( m_h2sRisk != 0 ) {
            m_h2sRisk->retrieveData ();
         }

      } else {
         m_h2sRisk = 0;
      }

      const SpeciesManager& speciesManager = m_theChemicalModel->getSpeciesManager (); // m_ChemicalModel - reference to "biggest" chemicalModel

      for ( speciesIndex = speciesManager.firstSpecies (); speciesIndex <= speciesManager.lastSpecies (); ++speciesIndex ) {
         ComponentManager::SpeciesNamesId species = speciesManager.mapIdToComponentManagerSpecies ( speciesIndex );

         if ( species != ComponentManager::UNKNOWN ) {

            theMap = createSnapshotResultPropertyValueMap ( ComponentManager::getInstance().getSpeciesName ( species ) + "Retained",
                                                            theSnapshot );

            if ( theMap != 0 ) {
               m_retainedOutputMaps [ species ] = theMap;
               theMap->retrieveData ();
            }

            theMap = createSnapshotResultPropertyValueMap ( ComponentManager::getInstance().getSpeciesSourceRockExpelledByName ( species ),
                                                            theSnapshot );
            

            if ( theMap != 0 ) {
               m_sourceRockExpelledOutputMaps [ species ] = theMap;
               theMap->retrieveData ();
            }

            if ( getAdsorptionSimulator()->speciesIsSimulated ( species )) {
               theMap = createSnapshotResultPropertyValueMap ( getAdsorptionSimulator()->getAdsorpedSpeciesName ( species ), theSnapshot );

               if ( theMap != 0 ) {
                  m_adsorpedOutputMaps [ species ] = theMap;
                  theMap->retrieveData ();
               }

            }

         }

      }
   }

}

void GenexSourceRock::saveSnapShotOutputMaps()
{
   std::map<std::string, GridMap*>::iterator it;
   std::map < CBMGenerics::ComponentManager::SpeciesNamesId, Interface::GridMap* >::iterator adsorpedIt;

   for(it = m_theSnapShotOutputMaps.begin(); it != m_theSnapShotOutputMaps.end(); ++ it) {

      if(it->second != 0 ) {
         (it->second)->restoreData();
         (it->second) = 0;
      }

   }  


   if ( m_hcSaturationOutputMap != 0 ) {
      m_hcSaturationOutputMap->restoreData ();
      m_hcSaturationOutputMap = 0;
   }

   if ( m_irreducibleWaterSaturationOutputMap != 0 ) {
      m_irreducibleWaterSaturationOutputMap->restoreData ();
      m_irreducibleWaterSaturationOutputMap = 0;
   }

   if ( m_adsorptionCapacity != 0 ) {
      m_adsorptionCapacity->restoreData ();
      m_adsorptionCapacity = 0;
   }

   if ( m_fracOfAdsorptionCap != 0 ) {
      m_fracOfAdsorptionCap->restoreData ();
      m_fracOfAdsorptionCap = 0;
   }

   if ( m_hcLiquidSaturation != 0 ) {
      m_hcLiquidSaturation->restoreData ();
      m_hcLiquidSaturation = 0;
   }

   if ( m_hcVapourSaturation != 0 ) {
      m_hcVapourSaturation->restoreData ();
      m_hcVapourSaturation = 0;
   }

   if ( m_gasVolumeOutputMap != 0 ) {
      m_gasVolumeOutputMap->restoreData ();
      m_gasVolumeOutputMap = 0;
   }

   if ( m_oilVolumeOutputMap != 0 ) {
      m_oilVolumeOutputMap->restoreData ();
      m_oilVolumeOutputMap = 0;
   }

   if ( m_gasExpansionRatio != 0 ) {
      m_gasExpansionRatio->restoreData ();
      m_gasExpansionRatio = 0;
   }

   if ( m_gasGeneratedFromOtgc != 0 ) {
      m_gasGeneratedFromOtgc->restoreData ();
      m_gasGeneratedFromOtgc = 0;
   }

   if ( m_totalGasGenerated != 0 ) {
      m_totalGasGenerated->restoreData ();
      m_totalGasGenerated = 0;
   }

   if ( m_retainedOilApiOutputMap != 0 ) {
      m_retainedOilApiOutputMap->restoreData ();
      m_retainedOilApiOutputMap = 0;
   }

   if ( m_retainedCondensateApiOutputMap != 0 ) {
      m_retainedCondensateApiOutputMap->restoreData ();
      m_retainedCondensateApiOutputMap = 0;
   }

   if ( m_retainedGor != 0 ) {
      m_retainedGor->restoreData ();
      m_retainedGor = 0;
   }

   if ( m_retainedCgr != 0 ) {
      m_retainedCgr->restoreData ();
      m_retainedCgr = 0;
   }

   if ( m_overChargeFactor != 0 ) {
      m_overChargeFactor->restoreData ();
      m_overChargeFactor = 0;
   }

   if ( m_porosityLossDueToPyrobitumen != 0 ) {
      m_porosityLossDueToPyrobitumen->restoreData ();
      m_porosityLossDueToPyrobitumen = 0;
   }

   if ( m_h2sRisk != 0 ) {
      m_h2sRisk->restoreData ();
      m_h2sRisk = 0;
   }

   if ( m_sourceRockEndMember1 != 0 ) {
      m_sourceRockEndMember1->restoreData ();
      m_sourceRockEndMember1 = 0;
   }

   if ( m_sourceRockEndMember2 != 0 ) {
      m_sourceRockEndMember2->restoreData ();
      m_sourceRockEndMember2 = 0;
   }

   if ( m_tocOutputMap != 0 ) {
      m_tocOutputMap->restoreData ();
      m_tocOutputMap = 0;
   }

   for ( adsorpedIt = m_sourceRockExpelledOutputMaps.begin (); adsorpedIt != m_sourceRockExpelledOutputMaps.end (); ++adsorpedIt ) {

      if ( adsorpedIt->second != 0 ) {
         (adsorpedIt->second)->restoreData ();
         (adsorpedIt->second) = 0;
      }

   }

   for ( adsorpedIt = m_retainedOutputMaps.begin (); adsorpedIt != m_retainedOutputMaps.end (); ++adsorpedIt ) {

      if ( adsorpedIt->second != 0 ) {
         (adsorpedIt->second)->restoreData ();
         (adsorpedIt->second) = 0;
      }

   }

   for ( adsorpedIt = m_adsorpedOutputMaps.begin (); adsorpedIt != m_adsorpedOutputMaps.end (); ++adsorpedIt ) {

      if ( adsorpedIt->second != 0 ) {
         (adsorpedIt->second)->restoreData ();
         (adsorpedIt->second) = 0;
      }

   }

}

void GenexSourceRock::updateSnapShotOutputMaps(Genex6::SourceRockNode *theNode)
{
   using namespace CBMGenerics;
   ComponentManager & theManager = ComponentManager::getInstance();
   GenexResultManager & theResultManager = GenexResultManager::getInstance();

   std::map<std::string, GridMap*>::iterator it;
   std::map<std::string, GridMap*>::iterator snapshotMapContainerEnd = m_theSnapShotOutputMaps.end();
 
   const unsigned int i = theNode->GetI ();
   const unsigned int j = theNode->GetJ ();

   //first the mandatory
   int speciesIndex, specId, resultIndex;

   SimulatorState& theSimulatorState = theNode->getPrincipleSimulatorState();
   m_theSimulator->setChemicalModel( m_theChemicalModel ); 

   for (speciesIndex = 0; speciesIndex < ComponentManager::NUMBER_OF_SPECIES; ++ speciesIndex) {
      it = m_theSnapShotOutputMaps.find(theManager.getSpeciesOutputPropertyName(speciesIndex, doOutputAdsorptionProperties ()));
      
      if(it != snapshotMapContainerEnd) {               
         specId = m_theSimulator->GetSpeciesIdByName(theManager.getSpeciesName(speciesIndex));

         if( specId < 0 ) {
            // to support both GX5 and GX6 config files
         } else {
            Genex6::SpeciesResult &theResult = theSimulatorState.GetSpeciesResult(specId);
            (it->second)->setValue(i, j, theResult.GetExpelledMass()); 
         }

      }

   }

   m_theSimulator->setChemicalModel( m_theChemicalModel1 );

   // then the optional results 
   for(resultIndex = 0; resultIndex < GenexResultManager::NumberOfResults; ++ resultIndex) {

      if(theResultManager.IsResultRequired(resultIndex)) {
         it = m_theSnapShotOutputMaps.find(theResultManager.GetResultName(resultIndex));

         if(it != snapshotMapContainerEnd ) { 

            if ( doOutputAdsorptionProperties ()) {
               // If there is a shale-gas simulation then get the results that were computed after a shale-gas simulation.
               (it->second)->setValue(i, j, theSimulatorState.getShaleGasResult ( resultIndex ));
            } else {
               // Otherwise the optional results are those computed by GenEx.
               (it->second)->setValue(i, j, theSimulatorState.GetResult( resultIndex ));
            }

         }

      }

   } 

   if( m_applySRMixing ) { 
      if ( m_sourceRockEndMember1 != 0 ) {
         m_sourceRockEndMember1->setValue ( i, j, 100.0 * theNode->GetF1() );
      }
      if ( m_sourceRockEndMember2 != 0 ) {
         m_sourceRockEndMember2->setValue ( i, j, 100.0 * theNode->GetF2() );
      }
   }
   if( m_tocOutputMap != 0 ) {
      m_tocOutputMap->setValue ( i, j, theSimulatorState.getCurrentToc () );
   }

   if ( doOutputAdsorptionProperties ()) {

      using namespace Genex6;

      const double meanBulkDensity = getProjectHandle ()->getSGDensitySample ()->getDensity ();

      // Converts m^3/m^3 to ft^3/ton.
      const double SCFpTonGasVolumeConversionFactor = CubicMetresToCubicFeet / ( KilogrammeToUSTon * meanBulkDensity );

      // Converts m^3/m^2 -> bcf/km^2
      const double BCFpKm2GasVolumeConversionFactor = CubicMetresToCubicFeet / 1.0e3; // = 1.0e6 * m^3->f^3 / 1.0e9.

      // Convert m^3/m^2 -> mega barrel/km^2.
      const double OilVolumeConversionFactor = 1.0e6 * CubicMetresToBarrel / 1.0e6;

      double gasVolume;
      double oilVolume;
      double gasExpansionRatio;
      double oilApi;
      double condensateApi;
      double gor;
      double cgr;
      double overChargeFactor = 0.0;


      m_irreducibleWaterSaturationOutputMap->setValue ( i, j, theSimulatorState.getIrreducibleWaterSaturation ());
      m_hcSaturationOutputMap->setValue ( i, j, theSimulatorState.getHcSaturation ());

      theNode->computeHcVolumes ( gasVolume, oilVolume, gasExpansionRatio, gor, cgr, oilApi, condensateApi );

      m_gasVolumeOutputMap->setValue ( i, j, gasVolume * BCFpKm2GasVolumeConversionFactor );
      m_oilVolumeOutputMap->setValue ( i, j, oilVolume * OilVolumeConversionFactor );
      m_gasExpansionRatio->setValue ( i, j, gasExpansionRatio );

      if ( m_retainedGor != 0 ) {

         if ( gor != 99999.0 ) {
            m_retainedGor->setValue ( i, j, gor * GorConversionFactor );
         } else {
            m_retainedGor->setValue ( i, j, 99999.0 );
         }

      }

      if ( m_retainedCgr != 0 ) {

         if ( cgr != 99999.0 ) {
            m_retainedCgr->setValue ( i, j, cgr * CgrConversionFactor );
         } else {
            m_retainedCgr->setValue ( i, j, 99999.0 );
         }

      }

      const SpeciesManager& speciesManager = * theSimulatorState.getSpeciesManager(); 
      
      if ( m_overChargeFactor != 0 ) {
         theNode->computeOverChargeFactor ( overChargeFactor );
         m_overChargeFactor->setValue ( i, j, overChargeFactor );
      }

      if ( m_porosityLossDueToPyrobitumen != 0 ) {
         double bitumenVolume = theSimulatorState.getImmobileSpecies ().getRetainedVolume ( theNode->getThickness ());
         m_porosityLossDueToPyrobitumen->setValue ( i, j, 100.0 * bitumenVolume );
      }

      if ( m_isSulphur and m_h2sRisk != 0 ) {
         m_h2sRisk->setValue ( i, j, theSimulatorState.getH2SFromGenex () + theSimulatorState.getH2SFromOtgc ());
      }

      m_gasGeneratedFromOtgc->setValue ( i, j, theSimulatorState.getTotalGasFromOtgc ());
      m_totalGasGenerated->setValue ( i, j,
                                      theSimulatorState.getTotalGasFromOtgc () +
                                      theSimulatorState.GetGroupResult ( CBMGenerics::GenexResultManager::HcGasGeneratedCum ));

      if ( m_retainedOilApiOutputMap != 0 ) {
         m_retainedOilApiOutputMap->setValue ( i, j, oilApi );
      }

      if ( m_retainedCondensateApiOutputMap != 0 ) {
         m_retainedCondensateApiOutputMap->setValue ( i, j, condensateApi );
      }

      if ( theSimulatorState.getEffectivePorosity () != 0.0 ) {
         m_hcLiquidSaturation->setValue ( i, j, 
                                          theSimulatorState.getRetainedLiquidVolume () / theSimulatorState.getEffectivePorosity ());
         m_hcVapourSaturation->setValue ( i, j,
                                          theSimulatorState.getRetainedVapourVolume () / theSimulatorState.getEffectivePorosity ());
      } else {
         m_hcLiquidSaturation->setValue ( i, j, 0.0 );
         m_hcVapourSaturation->setValue ( i, j, 0.0 );
      }

      for ( speciesIndex = speciesManager.firstSpecies (); speciesIndex <= speciesManager.lastSpecies (); ++speciesIndex ) {
         ComponentManager::SpeciesNamesId species = speciesManager.mapIdToComponentManagerSpecies ( speciesIndex );

         if ( species != ComponentManager::UNKNOWN ) {
            const Genex6::SpeciesState* speciesState = theSimulatorState.GetSpeciesStateById ( speciesIndex );
            const Genex6::SpeciesResult& result = theSimulatorState.GetSpeciesResult ( speciesIndex );

            if ( getAdsorptionSimulator()->speciesIsSimulated ( species )) {

               m_adsorpedOutputMaps [ species ]->setValue ( i, j, SCFpTonGasVolumeConversionFactor * result.getAdsorpedMol () / Genex6::Constants::VolumeMoleMethaneAtSurfaceConditions);

               // POTENTIAL problem here, if more than one species is considered for adsorption then this map will be overwritten.
               if ( speciesState->getAdsorptionCapacity () != 0.0 ) {
                  m_fracOfAdsorptionCap->setValue ( i, j, ( result.getAdsorpedMol () / Genex6::Constants::VolumeMoleMethaneAtSurfaceConditions ) / speciesState->getAdsorptionCapacity ());
               } else {
                  m_fracOfAdsorptionCap->setValue ( i, j, 0.0 );
               }

               m_adsorptionCapacity->setValue ( i, j, speciesState->getAdsorptionCapacity () * SCFpTonGasVolumeConversionFactor  );
            }

            m_sourceRockExpelledOutputMaps [ species ]->setValue ( i, j, speciesState->getMassExpelledFromSourceRock ());
            m_retainedOutputMaps [ species ]->setValue ( i, j, speciesState->getRetained ());
         }

      }

   } // doAdsorption

}

bool GenexSourceRock::computeSnapShot ( const double previousTime,
                                   const Snapshot *theSnapshot )
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

   if( calcVes == 0 || calcTemp == 0 || calcVre == 0 ) {
      status = false;
      if( calcTemp == 0 ) {
         LogHandler( LogHandler::ERROR_SEVERITY ) << "Missing Temperature map for the shapshot  :" << time << ". Aborting... ";
         LogHandler( LogHandler::INFO_SEVERITY ) << " ------------------------------------:";
      }
      if( calcVre == 0 ) {
         LogHandler( LogHandler::ERROR_SEVERITY ) << "Missing Vr map for the shapshot  :" << time << ". Aborting... ";
         LogHandler( LogHandler::INFO_SEVERITY ) << " ------------------------------------:";
      }
      if( calcVes == 0 ) {
         LogHandler( LogHandler::ERROR_SEVERITY ) << "Missing VES map for the shapshot  :" << time << ". Aborting... ";
         LogHandler( LogHandler::INFO_SEVERITY ) << " ------------------------------------:";
      }
   }
   if( status && doApplyAdsorption () && ( calcLP == 0 || calcHP == 0 || calcPressure == 0 ||  calcPorosity == 0 || calcPermeability == 0 )) {
      status = false;

      if( calcLP == 0 ) {
         LogHandler( LogHandler::ERROR_SEVERITY ) << "Missing lithostatic pressure for the shapshot  :" << time << ". Aborting... ";
         LogHandler( LogHandler::INFO_SEVERITY ) << " ------------------------------------:";
      }
      if( calcHP == 0 ) {
         LogHandler( LogHandler::ERROR_SEVERITY ) << "Missing hydrostatic pressure for the shapshot  :" << time << ". Aborting... ";
         LogHandler( LogHandler::INFO_SEVERITY ) << " ------------------------------------:";
      }
      if( calcPressure == 0 ) {
         LogHandler( LogHandler::ERROR_SEVERITY ) << "Missing pore pressure for the shapshot  :" << time << ". Aborting... ";
         LogHandler( LogHandler::INFO_SEVERITY ) << " ------------------------------------:";
      }
      if( calcPorosity == 0 ) {
         LogHandler( LogHandler::ERROR_SEVERITY ) << "Missing porosity for the shapshot  :" << time << ". Aborting... ";
         LogHandler( LogHandler::INFO_SEVERITY ) << " ------------------------------------:";
      }
      if( calcPermeability == 0 ) {
         LogHandler( LogHandler::ERROR_SEVERITY ) << "Missing permeability for the shapshot  :" << time << ". Aborting... ";
         LogHandler( LogHandler::INFO_SEVERITY ) << " ------------------------------------:";
      }

   }
   if( status ) {
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
   }

   return status;
}

void GenexSourceRock::zeroTimeStepAccumulations () {

   std::vector<Genex6::SourceRockNode*>::iterator itNode;

   for(itNode = m_theNodes.begin(); itNode != m_theNodes.end(); ++itNode)
   {
      (*itNode)->zeroTimeStepAccumulations ();
   }

}


void GenexSourceRock::collectSourceRockNodeHistory () {

   std::vector<Genex6::SourceRockNode*>::iterator itEnd = m_theNodes.end();

   for(std::vector<Genex6::SourceRockNode*>::iterator it = m_theNodes.begin(); it !=itEnd; ++ it) {
      (*it)->collectHistory ();
   }

}


double GenexSourceRock::getMaximumTimeStepSize ( const double depositionTime ) const {
   return m_theSimulator->GetMaximumTimeStepSize ( depositionTime );
}

void GenexSourceRock::computeTimeInstance ( const double &startTime,
                                       const double &endTime,
                                       const LocalGridInterpolator* ves,
                                       const LocalGridInterpolator* temperature,
                                       const LocalGridInterpolator* thicknessScaling,
                                       const LocalGridInterpolator* lithostaticPressure,
                                       const LocalGridInterpolator* hydrostaticPressure,
                                       const LocalGridInterpolator* porePressure,
                                       const LocalGridInterpolator* porosity,
                                       const LocalGridInterpolator* permeability,
                                       const LocalGridInterpolator* vre ) {   

   bool useMaximumVes = isVESMaxEnabled();
   double maximumVes = getVESMax();
   maximumVes *= MegaPaToPa;

   std::vector<Genex6::SourceRockNode*>::iterator itNode;
   
   for(itNode = m_theNodes.begin(); itNode != m_theNodes.end(); ++ itNode) {
      double in_VES = ves->evaluateProperty( (*itNode)->GetI(), (*itNode)->GetJ(), endTime );
      if(useMaximumVes && in_VES > maximumVes) {
	      in_VES = maximumVes;
      }

      double in_startTemp = temperature->evaluateProperty( (*itNode)->GetI(), (*itNode)->GetJ(), startTime );
      double in_endTemp = temperature->evaluateProperty( (*itNode)->GetI(), (*itNode)->GetJ(), endTime );

      double nodeLithostaticPressure = lithostaticPressure ? 1.0e6 * lithostaticPressure->evaluateProperty( (*itNode)->GetI(), (*itNode)->GetJ(), endTime ) : CauldronNoDataValue;
      double nodeHydrostaticPressure = hydrostaticPressure ? 1.0e6 * hydrostaticPressure->evaluateProperty( (*itNode)->GetI(), (*itNode)->GetJ(), endTime ) : CauldronNoDataValue;
      double startNodePorePressure = porePressure ? 1.0e6 * porePressure->evaluateProperty( (*itNode)->GetI(), (*itNode)->GetJ(), startTime ) : CauldronNoDataValue;
      double endNodePorePressure = porePressure ?  1.0e6 * porePressure->evaluateProperty( (*itNode)->GetI(), (*itNode)->GetJ(), endTime ) : CauldronNoDataValue;
      double nodePorosity = porosity ? 0.01 * porosity->evaluateProperty( (*itNode)->GetI(), (*itNode)->GetJ(), endTime ) : CauldronNoDataValue;
      double nodePermeability = permeability ? permeability->evaluateProperty( (*itNode)->GetI(), (*itNode)->GetJ(), endTime ) : CauldronNoDataValue;
      double nodeVre = vre->evaluateProperty ( (*itNode)->GetI(), (*itNode)->GetJ(), endTime );

      double in_thicknessScaling = thicknessScaling ? thicknessScaling->evaluateProperty( (*itNode)->GetI(), (*itNode)->GetJ(), endTime ) : 1.0;

      Genex6::Input *theInput = new Genex6::Input ( startTime, endTime,
                                                    in_startTemp,
                                                    in_endTemp,
                                                    in_VES,
                                                    nodeLithostaticPressure,
                                                    nodeHydrostaticPressure,
                                                    startNodePorePressure,
                                                    endNodePorePressure,
                                                    nodePorosity,
                                                    nodePermeability,
                                                    nodeVre,
                                                    (*itNode)->GetI (),
                                                    (*itNode)->GetJ (),
                                                    in_thicknessScaling );

      (*itNode)->AddInput(theInput);
      
      m_theSimulator->setChemicalModel( m_theChemicalModel1 );

      bool isInitialTimeStep = (*itNode)->RequestComputation(0, *m_theSimulator );
      if( m_applySRMixing ) {
         m_theSimulator->setChemicalModel( m_theChemicalModel2 );
         (*itNode)->RequestComputation( 1, *m_theSimulator );
         m_theSimulator->setChemicalModel( m_theChemicalModel1 );
      }

      if ( not isInitialTimeStep && doApplyAdsorption ()) {
         m_adsorptionSimulator->compute( *theInput, &(*itNode)->GetSimulatorState(0)); 
      }
      if( m_applySRMixing && not isInitialTimeStep && m_adsorptionSimulator2 != 0 ) {
         m_adsorptionSimulator2->compute( *theInput, &(*itNode)->GetSimulatorState(1)); 
      }
      
      if( m_applySRMixing ) {
         (*itNode)->RequestMixing( m_theChemicalModel );
      }

      if ( not isInitialTimeStep ) {
         (*itNode)->collectHistory ();
      }

      (*itNode)->ClearInputHistory();
   } 
}

GridMap *GenexSourceRock::createSnapshotResultPropertyValueMap ( const std::string& propertyName,
                                                            const Snapshot*    theSnapshot)
{
   PropertyValue *thePropertyValue = m_projectHandle->createMapPropertyValue (propertyName, theSnapshot, 0, 
                                                                              m_formation, 0);
   GridMap *theMap = 0;

   if(thePropertyValue) {
      theMap = thePropertyValue->getGridMap();
   }

   return theMap;   
}

void GenexSourceRock::addNode(Genex6::SourceRockNode* in_Node)
{
   m_theNodes.push_back(in_Node);
}
double GenexSourceRock::getLithoDensity(const LithoType *theLitho) const
{
   Interface::LithoTypeAttributeId theId = Interface::Density;
   const AttributeValue theDensity = theLitho->getAttributeValue(theId);
   double density = theDensity.getDouble();
   return density;
}
const GridMap * GenexSourceRock::getLithoType1PercentageMap() const
{
   return m_formation->getLithoType1PercentageMap();
}
const GridMap * GenexSourceRock::getLithoType2PercentageMap() const
{
   return m_formation->getLithoType2PercentageMap();
}

const GridMap * GenexSourceRock::getLithoType3PercentageMap() const
{
   return m_formation->getLithoType3PercentageMap();
}
const LithoType * GenexSourceRock::getLithoType1() const 
{
   return m_formation->getLithoType1 (); 
}
const LithoType * GenexSourceRock::getLithoType2() const 
{
   return m_formation->getLithoType2 (); 
}
const LithoType * GenexSourceRock::getLithoType3() const 
{
   return m_formation->getLithoType3 (); 
}
const GridMap * GenexSourceRock::getInputThicknessGridMap () const
{
   return m_formation->getInputThicknessMap();
}
const GridMap * GenexSourceRock::getTopSurfacePropertyGridMap (const string & propertyName, 
                                                          const Interface::Snapshot * snapshot) const
{
   return getPropertyGridMap (propertyName, snapshot, 0, 0, m_formation->getTopSurface ());
}

const GridMap * GenexSourceRock::getSurfaceFormationPropertyGridMap (const string & propertyName,const Interface::Snapshot * snapshot) const
{
   const Formation *theFormation = m_projectHandle->findFormation (m_layerName);   

   const GridMap * result;

   result =  getPropertyGridMap (propertyName, snapshot, 0, theFormation, theFormation->getTopSurface ());

   return result;
}

const GridMap * GenexSourceRock::getPropertyGridMap (const string & propertyName,
                                                const Interface::Snapshot * snapshot,
                                                const Interface::Reservoir * reservoir, 
                                                const Formation * formation,
                                                const Surface * surface) const
{
   int selectionFlags = 0;

   if (reservoir) selectionFlags |= Interface::RESERVOIR;
   if (formation && !surface) selectionFlags |= Interface::FORMATION;
   if (surface && !formation) selectionFlags |= Interface::SURFACE;
   if (formation && surface) selectionFlags |= Interface::FORMATIONSURFACE;

   const Property* property = m_projectHandle->findProperty (propertyName);

   PropertyValueList * propertyValues = m_projectHandle->getPropertyValues ( selectionFlags,
                                                                             property,
                                                                             snapshot, 
                                                                             reservoir, 
                                                                             formation, 
                                                                             surface,
                                                                             Interface::MAP);     

   if (propertyValues->size () != 1) {
      return 0;
   }

   const GridMap *gridMap = (*propertyValues)[0]->getGridMap ();

   delete propertyValues;
   return gridMap;
}
void GenexSourceRock::computeSnapshotIntervals ()
{
   m_depositionTime = getDepositionTime ();

   Interface::SnapshotList * snapshots = m_projectHandle->getSnapshots (Interface::MINOR | Interface::MAJOR);

   Interface::SnapshotList::reverse_iterator snapshotIter;

   const Interface::Snapshot * start;
   const Interface::Snapshot * end;

   if (snapshots->size () >= 1) {
      end = 0;
      for(snapshotIter = snapshots->rbegin (); snapshotIter != snapshots->rend () - 1; ++ snapshotIter) {

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
   delete snapshots;

}

// The following functions are not the the trunk version.
// Can they be removed?
const Simulator& GenexSourceRock::getSimulator () const {
   return *m_theSimulator;
}

void GenexSourceRock::clearOutputHistory () {


   std::vector<Genex6::SourceRockNode*>::iterator itNode;

   for(itNode = m_theNodes.begin(); itNode != m_theNodes.end(); ++itNode) {
      (*itNode)->ClearOutputHistory();
   }

}

void GenexSourceRock::initialiseNodes () {

   std::vector<Genex6::SourceRockNode*>::iterator itEnd = m_theNodes.end();

   for(std::vector<Genex6::SourceRockNode*>::iterator it = m_theNodes.begin(); it !=itEnd; ++it)
   {
      (*it)->initialise ();
   }

}

}//namespace Genex6
