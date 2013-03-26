#include "C1AdsorptionSimulator.h"

#include <iostream>
#include <iomanip>
using namespace std;

#include "Interface/SGDensitySample.h"

#include "NumericFunctions.h"

#include "EosPack.h"
#include "PhysicalConstants.h"
#include "Constants.h"

#include "SpeciesState.h"
#include "SpeciesManager.h"

using namespace CBMGenerics;

const double Genex6::C1AdsorptionSimulator::AdsorptionPorosityThreshold = 0.1;

const std::string Genex6::C1AdsorptionSimulator::s_nullString = "";
const std::string Genex6::C1AdsorptionSimulator::s_c1Name = ComponentManager::getInstance ().GetSpeciesName ( ComponentManager::C1 );
const std::string Genex6::C1AdsorptionSimulator::s_c1AdsorpedName = ComponentManager::getInstance ().GetSpeciesName ( ComponentManager::C1 ) + "Adsorped";
const std::string Genex6::C1AdsorptionSimulator::s_c1ExpelledName = ComponentManager::getInstance ().GetSpeciesName ( ComponentManager::C1 ) + "AdsorpedExpelled";
const std::string Genex6::C1AdsorptionSimulator::s_c1FreeName = ComponentManager::getInstance ().GetSpeciesName ( ComponentManager::C1 ) + "AdsorpedFree";

const bool Genex6::C1AdsorptionSimulator::s_speciesIsSimulated  [ ComponentManager::NumberOfOutputSpecies ] =
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
     false, // C15_AT
     false, // C6_14BT
     false, // C6_14DBT
     false, // C6_14BP
     false, // C15_AROS
     false, // C15_SATS
     false, // C6_14SATS
     false  // C6_14AROS 
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

   double LithoPressurePhaseDensity [ pvtFlash::N_PHASES ];
   double molarPhaseViscosity  [ pvtFlash::N_PHASES ];
   double molarComponentMasses [ pvtFlash::NUM_COMPONENTS ];
   double molarPhaseComponentMasses [ pvtFlash::N_PHASES ][ pvtFlash::NUM_COMPONENTS ];
   double molarPhaseDensity [ pvtFlash::N_PHASES ];

   const unsigned int i = sourceRockInput.getI ();
   const unsigned int j = sourceRockInput.getJ ();

   const double temperature = sourceRockInput.GetTemperatureKelvin ();
   const double porePressure = NumericFunctions::Maximum <double>( SurfacePressure, sourceRockInput.getPorePressure ());
   const double porosity = sourceRockInput.getPorosity ();

   double adsorptionCapacity;
   const double& temperatureKelvin = temperature;
   const double  temperatureCelcius = temperature - Constants::s_TCabs;

   //bool output = ( i == 0 and j == 0 );
   bool output = false;

   double freeGasMol;
   double adsorpedGasMol;
   double expelledGas;
   double porosityCapacity;
   double c1MolarVolume;
   double excessCapacityMol;
   double expelledGasMol;
   double gasCapacity;
   double gasCapacityMol;

   double adsorpedGasMolNext;
   double adsorpedTransient = 0.0;
   double desorpedTransient = 0.0;

   for ( k = 0; k < pvtFlash::NUM_COMPONENTS; ++k ) {
      molarComponentMasses [ k ] = 0.0;
      molarPhaseComponentMasses [ 0 ][ k ] = 0.0;
      molarPhaseComponentMasses [ 1 ][ k ] = 0.0;
   }

   molarPhaseDensity [ 0 ] = 0.0;
   molarPhaseDensity [ 1 ] = 0.0;

   LithoPressurePhaseDensity [ 0 ] = 0.0;
   LithoPressurePhaseDensity [ 1 ] = 0.0;

   SpeciesState* c1State = simulatorState->GetSpeciesStateById ( m_speciesManager.getC1Id ());

   molarComponentMasses [ pvtFlash::C1 ] = 16.043; // In g/mol.

   // Standard Conditions
   // Temperature = 288.70556 K
   // Pressure = 101.325 kPa

   molarComponentMasses [ pvtFlash::COX ] = 0.0;
   molarComponentMasses [ pvtFlash::H2S ] = 0.0;

   pvtFlash::EosPack::getInstance ().computeWithLumping ( temperatureKelvin, porePressure, molarComponentMasses, molarPhaseComponentMasses, molarPhaseDensity, molarPhaseViscosity );

   // Adsorption Capacity in cm^3 of Methane (STP) / gram of rock
   adsorptionCapacity = getAdsorptionCapacity ( i, j, temperatureCelcius, 1.0e-6 * porePressure, simulatorState->getCurrentToc (), ComponentManager::C1 );

   // For testing/? purposes only.
   double vl;
   vl = getVL ( i, j, getVLReferenceTemperature (), simulatorState->getCurrentToc (), ComponentManager::C1 );
   simulatorState->setVLReferenceTemperature ( vl );
   vl = getVL ( i, j, temperatureCelcius, simulatorState->getCurrentToc (), ComponentManager::C1 );
   simulatorState->setVLSRTemperature ( vl );

   if ( output ) {
      cout << " adsorption: " << adsorptionCapacity << "  " << simulatorState->getCurrentToc () << endl;
   }

   if ( output ) {
      cout << " c1 state: " 
           << c1State->getExpelledMassTransient () << "  "
           << c1State->getFreeMol () << "  "
           << c1State->getAdsorpedMol () << "  "
           << c1State->getRetained () << "  "
           << c1State->getExpelledMol () << "  "
           << c1State->getDesorpedMol () << "  "
           << endl;

   }

   // Methane adsorption capacity in vol methane (STP) / vol rock
   gasCapacity = inorganicDensity * 1000.0 * adsorptionCapacity / 1.0e6;
   expelledGas = c1State->getExpelledMassTransient () / thickness; // In kg / m2 / thickness = kg / m3.
   freeGasMol = c1State->getFreeMol ();
   adsorpedGasMol = c1State->getAdsorpedMol ();

   c1State->setAdsorptionCapacity ( gasCapacity );

   // Molar Volume at subsurface conditions
   c1MolarVolume  = 1.0e-3 * molarPhaseComponentMasses [ 0 ][ pvtFlash::C1 ] / molarPhaseDensity [ 0 ]; // m3 / mol

   // Convert to moles/m3
   expelledGasMol = expelledGas * 1000.0 / molarComponentMasses [ pvtFlash::C1 ]; // In kg * 1000 / m3 / g / mol = mol / m3 
   gasCapacityMol = gasCapacity * 42.306553; // In mol / m3 
   porosityCapacity = porosity / c1MolarVolume;

   excessCapacityMol = gasCapacityMol - adsorpedGasMol;

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
         double retainedMols = 1000.0 * retained / thickness / molarComponentMasses [ pvtFlash::C1 ];
         double readsorpedMols =  NumericFunctions::Minimum<double>( retainedMols, gasCapacityMol - adsorpedGasMol );

         retainedMols -= readsorpedMols;
         retained = retainedMols / 1000.0 * thickness * molarComponentMasses [ pvtFlash::C1 ];
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

#if 0
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
   c1State->setAdsorpedMass ( adsorpedGasMol * thickness / 1000.0 * molarComponentMasses [ pvtFlash::C1 ]);
   c1State->setTransientAdsorpedMass ( adsorpedTransient * thickness / 1000.0 * molarComponentMasses [ pvtFlash::C1 ]);
   c1State->setTransientDesorpedMass ( desorpedTransient * thickness / 1000.0 * molarComponentMasses [ pvtFlash::C1 ]);
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
   return new Genex6::C1AdsorptionSimulator ( speciesManager, projectHandle, isManaged );
}
