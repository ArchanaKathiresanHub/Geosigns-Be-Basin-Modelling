//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 
#include "C1AdsorptionSimulator.h"

// std library
#include <iostream>
#include <iomanip>

#include "Interface/SGDensitySample.h"

#include "EosPack.h"
#include "ConstantsGenex.h"

#include "SpeciesState.h"
#include "SpeciesManager.h"

#include "SimulatorStateAdsorption.h"

// utilitites library
#include "NumericFunctions.h"
#include "ConstantsMathematics.h"

#define TESTADS 1



// CBMGenerics library
#include "ComponentManager.h"

typedef CBMGenerics::ComponentManager::SpeciesNamesId ComponentId;
typedef CBMGenerics::ComponentManager::PhaseId PhaseId;

using Utilities::Maths::CelciusToKelvin;
using namespace std;
using namespace CBMGenerics;

const double Genex6::C1AdsorptionSimulator::AdsorptionPorosityThreshold = 0.1;

const std::string Genex6::C1AdsorptionSimulator::s_nullString = "";
const std::string Genex6::C1AdsorptionSimulator::s_c1Name         = ComponentManager::getInstance ().getSpeciesName ( ComponentManager::C1 );
const std::string Genex6::C1AdsorptionSimulator::s_c1AdsorpedName = ComponentManager::getInstance ().getSpeciesName ( ComponentManager::C1 ) + "Adsorped";
const std::string Genex6::C1AdsorptionSimulator::s_c1ExpelledName = ComponentManager::getInstance ().getSpeciesName ( ComponentManager::C1 ) + "AdsorpedExpelled";
const std::string Genex6::C1AdsorptionSimulator::s_c1FreeName     = ComponentManager::getInstance ().getSpeciesName ( ComponentManager::C1 ) + "AdsorpedFree";

const bool Genex6::C1AdsorptionSimulator::s_speciesIsSimulated  [ ComponentManager::NUMBER_OF_SPECIES ] =
   { false, // asphaltene
     false, // resin
     false, // C15PlusAro
     false, // C15PlusSat
     false, // C6Minus14Aro
     false, // C6Minus14Sat
     false, // C5
     false, // C4
     false, // C3
     false, // C2
     true,  // C1
     false, // COx
     false, // N2
     false, // H2S
     false, // LSC
     false, // C15PlusAT
     false, // C6Minus14BT
     false, // C6Minus14DBT
     false, // C6Minus14BP
     false, // C15PlusAroS
     false, // C15PlusSatS
     false, // C6Minus14SatS
     false  // C6Minus14AroS 
   };

Genex6::C1AdsorptionSimulator::C1AdsorptionSimulator ( const SpeciesManager&                      speciesManager,
                                                       DataAccess::Interface::ProjectHandle* projectHandle,
                                                       const bool isManaged ) :
   Genex6::AdsorptionSimulator ( projectHandle, isManaged ),
   m_speciesManager ( speciesManager ) {
}

void Genex6::C1AdsorptionSimulator::compute ( const Input&              sourceRockInput,
                                                    SimulatorStateBase* baseState ) {


   SimulatorState* simulatorState = dynamic_cast<SimulatorState*>( baseState );

   if ( simulatorState == 0 ) {
      // Error?
   }

   // Pa.
   const double SurfacePressure = 1.01325e5;
   

   const double thickness = simulatorState->GetThickness ();
   const double inorganicDensity = getProjectHandle ()->getSGDensitySample ()->getDensity ();

   if ( thickness < 1.0e-3 ) {
      return;
   }
                                      
   unsigned int k;

   double molarPhaseViscosity  [PhaseId::NUMBER_OF_PHASES];
   double molarComponentMasses [ComponentId::NUMBER_OF_SPECIES ];
   double molarPhaseComponentMasses [PhaseId::NUMBER_OF_PHASES][ComponentId::NUMBER_OF_SPECIES ];
   double molarPhaseDensity [PhaseId::NUMBER_OF_PHASES];

   const unsigned int i = sourceRockInput.getI ();
   const unsigned int j = sourceRockInput.getJ ();

   const double temperature = sourceRockInput.GetTemperatureKelvin ();
   const double porePressure = NumericFunctions::Maximum <double>( SurfacePressure, sourceRockInput.getPorePressure ());

   double adsorptionCapacity;
   double vbaLang, vbaJGS, vbaPrang;
   const double& temperatureKelvin = temperature;
   const double  temperatureCelcius = temperature - CelciusToKelvin;

   //bool output = ( i == 0 and j == 0 );
   bool output = false;

   double freeGasMol;
   double adsorpedGasMol;
   double expelledGas;
   double expelledGasMol;
   double gasCapacity;
   double gasCapacityMol;

   double adsorpedGasMolNext;
   double adsorpedTransient = 0.0;
   double desorpedTransient = 0.0;

   for ( k = 0; k < ComponentId::NUMBER_OF_SPECIES; ++k ) {
      molarComponentMasses [ k ] = 0.0;
      molarPhaseComponentMasses [ 0 ][ k ] = 0.0;
      molarPhaseComponentMasses [ 1 ][ k ] = 0.0;
   }

   molarPhaseDensity [ 0 ] = 0.0;
   molarPhaseDensity [ 1 ] = 0.0;

   SpeciesState* c1State = simulatorState->GetSpeciesStateById ( m_speciesManager.getC1Id ());
#ifdef TESTADS
   SpeciesResult&  result = simulatorState->GetSpeciesResult(  m_speciesManager.getC1Id ());
#endif

   molarComponentMasses [ ComponentId::C1 ] = 16.043; // In g/mol.

   // Standard Conditions
   // Temperature = 288.70556 K
   // Pressure = 101.325 kPa

   molarComponentMasses [ ComponentId::COX ] = 0.0;
   molarComponentMasses [ ComponentId::H2S ] = 0.0;

   pvtFlash::EosPack::getInstance ().computeWithLumping ( temperatureKelvin, porePressure, molarComponentMasses, molarPhaseComponentMasses, molarPhaseDensity, molarPhaseViscosity );

   // Adsorption Capacity in cm^3 of Methane (STP) / gram of rock
   adsorptionCapacity = getAdsorptionCapacity ( i, j, temperatureCelcius, 1.0e-6 * porePressure, simulatorState->getCurrentToc (), ComponentManager::C1 );
   vbaLang  = getPessureLangmuir();
   vbaPrang = getPessurePrangmuir();
   vbaJGS   = getPessureJGS();

   if( vbaLang > 0 or vbaPrang > 0 or vbaJGS > 0 ) {
      vbaLang  = getPessureLangmuir();
      vbaPrang = getPessurePrangmuir();
      vbaJGS   = getPessureJGS();
   }
   // For testing/? purposes only.
   double vl;
   vl = getVL ( i, j, getVLReferenceTemperature (), simulatorState->getCurrentToc (), ComponentManager::C1 );
   simulatorState->setVLReferenceTemperature ( vl );
   vl = getVL ( i, j, temperatureCelcius, simulatorState->getCurrentToc (), ComponentManager::C1 );
   simulatorState->setVLSRTemperature ( vl );

   double pl = getPL(  i, j, temperatureCelcius, simulatorState->getCurrentToc (), ComponentManager::C1 );
   if ( output ) {
      cout << " adsorption: " << adsorptionCapacity << "  " << simulatorState->getCurrentToc () << endl;
   }

   if ( output ) {
      cout << " c1 state: " 
#ifdef TESTADS
           << result.GetExpelledMassTransient () << "  " 
#else
           << c1State->getExpelledMassTransient () << "  "
#endif
           << c1State->getFreeMol () << "  "
           << c1State->getAdsorpedMol () << "  "
           << c1State->getRetained () << "  "
           << c1State->getExpelledMol () << "  "
           << c1State->getDesorpedMol () << "  "
           << endl;

   }

   // Methane adsorption capacity in vol methane (STP) / vol rock
   gasCapacity = inorganicDensity * 1000.0 * adsorptionCapacity / 1.0e6;
#ifdef TESTADS
   expelledGas = result.GetExpelledMassTransient () / thickness; // In kg / m2 / thickness = kg / m3.
#else
   expelledGas = c1State->getExpelledMassTransient () / thickness; // In kg / m2 / thickness = kg / m3.
#endif
   freeGasMol = c1State->getFreeMol ();
   adsorpedGasMol = c1State->getAdsorpedMol ();

   c1State->setAdsorptionCapacity ( gasCapacity );

   // Convert to moles/m3
   expelledGasMol = expelledGas * 1000.0 / molarComponentMasses [ ComponentId::C1 ]; // In kg * 1000 / m3 / g / mol = mol / m3 
   gasCapacityMol = gasCapacity * 42.306553; // In mol / m3 

   double adsGas, freeGas, maxads, maxfree, molgas, molgasvol;
   adsGas = getAdsGas();
   freeGas = getFreeGas();
   maxfree = getNmaxFree();
   maxads = getNmaxAds();
   molgas = getMolesGas();
   molgasvol = getMolarGasVol();

   double retainedMols = 0.0;
   if ( gasCapacityMol >= adsorpedGasMol ) {

      // Compute adsorption at end of time-step.
      adsorpedGasMolNext = NumericFunctions::Minimum<double>( gasCapacityMol, adsorpedGasMol + expelledGasMol );

      // Compute how much was adsorped for this time-step.
      adsorpedTransient = adsorpedGasMolNext - adsorpedGasMol;

      desorpedTransient = 0.0;

      // Update new adsorption value.
      adsorpedGasMol = adsorpedGasMolNext;
      if ( gasCapacityMol > adsorpedGasMol ) {
         // Now re-adsorp some of the retained gas.
         double retained = c1State->getRetained ();
         retainedMols = 1000.0 * retained / thickness / molarComponentMasses [ ComponentId::C1 ];
         double readsorpedMols =  NumericFunctions::Minimum<double>( retainedMols, gasCapacityMol - adsorpedGasMol );

         retainedMols -= readsorpedMols;
         retained = retainedMols / 1000.0 * thickness * molarComponentMasses [ ComponentId::C1 ];
         adsorpedGasMol += readsorpedMols;

         if ( output ) {
            cout << "re-adsorped: " 
                 <<  c1State->getRetained () << "  "
                 << retained << "  "
                 << retainedMols << "  "
                 << readsorpedMols << "  "
                 << adsorpedGasMol << "  "
                 << endl;// << "  "
//                  <<  << "  "

         }

         c1State->setRetained ( retained );
      } 

   } else {

//       // Compute adsorption at end of time-step.
//       adsorpedGasMolNext = adsorpedGasMol - gasCapacityMol;

//       // Compute how much was desorped for this time-step.
//       desorpedTransient = adsorpedGasMol - adsorpedGasMolNext;

//       // Update new adsorption value.
//       adsorpedGasMol = adsorpedGasMolNext;

      adsorpedTransient = 0.0;

      desorpedTransient = adsorpedGasMol - gasCapacityMol;

      adsorpedGasMol = adsorpedGasMol - desorpedTransient;

   }

   if( false ) {
      cout << endl;
      cout << "Temp, ppress " << temperatureCelcius << " " << porePressure << endl;
      cout << "gasCapMol, expellGasMol, adsMol, adgas, freegas = " << gasCapacityMol << " " << expelledGasMol << " " << adsorpedGasMol << " " << adsGas << " " << freeGas << endl;
      cout << "(tr)adstrans mol, ret mol = " << adsorpedTransient << " " << retainedMols << endl;
       cout << "(tr)vl, pl = " << vl << " " << pl << endl;
     cout << "(test)maxads, free, molgas, molvol = " << maxads << " " << maxfree << " " << molgas << " " << molgasvol << endl;
      cout << "(test)Lang, Prag, JGS = " << vbaLang << " " << vbaPrang << " " << vbaJGS << endl;
   }
#if 0

   // Molar Volume at subsurface conditions
   double c1MolarVolume  = 1.0e-3 * molarPhaseComponentMasses [ 0 ][ ComponentId::C1 ] / molarPhaseDensity [ 0 ]; // m3 / mol
   const double porosity = sourceRockInput.getPorosity ();
   double porosityCapacity = porosity / c1MolarVolume;

   double excessCapacityMol = gasCapacityMol - adsorpedGasMol;
   if ( excessCapacityMol > 0.0 ) {

      if ( excessCapacityMol >= expelledGasMol ) {
         adsorpedGasMol += expelledGasMol;
      } else {
         adsorpedGasMol = gasCapacityMol;

         if ( porosity >= m_thresholdPorosity ) {
            freeGasMol = 0.0;
         } else {
            freeGasMol += expelledGasMol - excessCapacityMol;
         }

      }

   } else {

      if ( porosity >= m_thresholdPorosity ) {
         adsorpedGasMol = gasCapacityMol;
         freeGasMol = 0.0;
      } else {
         freeGasMol += adsorpedGasMol - gasCapacityMol + expelledGasMol;
      }
      
   }

   if ( porosity > 0.0 and freeGasMol > 0.0 ) {

     if ( freeGasMol > porosityCapacity ) {
       freeGasMol = porosityCapacity;
     }

   }
#endif

   c1State->setAdsorpedMol ( adsorpedGasMol );
   c1State->setAdsorpedMass ( adsorpedGasMol * thickness / 1000.0 * molarComponentMasses [ ComponentId::C1 ]);
   c1State->setTransientAdsorpedMass ( adsorpedTransient * thickness / 1000.0 * molarComponentMasses [ ComponentId::C1 ]);
   c1State->setTransientDesorpedMass ( desorpedTransient * thickness / 1000.0 * molarComponentMasses [ ComponentId::C1 ]);
   c1State->setDesorpedMol ( desorpedTransient + c1State->getDesorpedMol ());
   c1State->setFreeMol ( freeGasMol );
   c1State->setExpelledMol ( c1State->getExpelledMol () + expelledGasMol );

}

bool Genex6::C1AdsorptionSimulator::speciesIsSimulated ( const ComponentManager::SpeciesNamesId species ) const {

   if ( species != ComponentManager::UNKNOWN ) {
      return s_speciesIsSimulated [ species ];
   } else {
      return false;
   }

}

const std::string& Genex6::C1AdsorptionSimulator::getAdsorpedSpeciesName ( const CBMGenerics::ComponentManager::SpeciesNamesId species ) const {

   if ( species == ComponentManager::C1 ) {
      return s_c1AdsorpedName;
   } else {
      return s_nullString;
   }

}

const std::string& Genex6::C1AdsorptionSimulator::getExpelledSpeciesName ( const CBMGenerics::ComponentManager::SpeciesNamesId species ) const {

   if ( species == ComponentManager::C1 ) {
      return s_c1ExpelledName;
   } else {
      return s_nullString;
   }

}

const std::string& Genex6::C1AdsorptionSimulator::getFreeSpeciesName ( const CBMGenerics::ComponentManager::SpeciesNamesId species ) const {

   if ( species == ComponentManager::C1 ) {
      return s_c1FreeName;
   } else {
      return s_nullString;
   }

}

Genex6::AdsorptionSimulator* Genex6::allocateC1AdsorptionSimulator ( DataAccess::Interface::ProjectHandle* projectHandle, 
                                                                     const SpeciesManager& speciesManager,
                                                                     const bool applyAdsorption,
                                                                     const bool isManaged ) {
   ( void ) applyAdsorption;
   return new Genex6::C1AdsorptionSimulator ( speciesManager, projectHandle, isManaged );
}
