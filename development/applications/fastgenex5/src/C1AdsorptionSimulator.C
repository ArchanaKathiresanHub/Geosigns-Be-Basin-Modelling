#include "C1AdsorptionSimulator.h"

#include <iostream>
#include <iomanip>
using namespace std;

#include "NumericFunctions.h"

#include "EosPack.h"
#include "PhysicalConstants.h"
#include "Constants.h"

#include "AdsorptionProjectHandle.h"

#include "SpeciesState.h"

using namespace CBMGenerics;
using namespace Genex5;

const double C1AdsorptionSimulator::AdsorptionPorosityThreshold = 0.1;

const std::string C1AdsorptionSimulator::s_nullString = "";
const std::string C1AdsorptionSimulator::s_c1Name = ComponentManager::getInstance ().GetSpeciesName ( ComponentManager::C1 );
const std::string C1AdsorptionSimulator::s_c1AdsorpedName = ComponentManager::getInstance ().GetSpeciesName ( ComponentManager::C1 ) + "Adsorped";
const std::string C1AdsorptionSimulator::s_c1ExpelledName = ComponentManager::getInstance ().GetSpeciesName ( ComponentManager::C1 ) + "AdsorpedExpelled";
const std::string C1AdsorptionSimulator::s_c1FreeName = ComponentManager::getInstance ().GetSpeciesName ( ComponentManager::C1 ) + "AdsorpedFree";

const bool C1AdsorptionSimulator::s_speciesIsSimulated  [ ComponentManager::NumberOfOutputSpecies ] =
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
     false  // N2
   };

C1AdsorptionSimulator::C1AdsorptionSimulator ( AdsorptionProjectHandle* projectHandle ) :
   m_adsorptionProjectHandle ( projectHandle )
{
}

// void C1AdsorptionSimulator::compute ( const double                thickness,
//                                       const double                inorganicDensity,
//                                       const SourceRockNodeInput&  sourceRockInput,
//                                             SourceRockNodeOutput& sourceRockOutput,
//                                             SimulatorState*       simulatorState ) {

//    // Pa.
//    const double SurfacePressure = 1.01325e5;

//    if ( thickness < 1.0e-3 ) {
//       return;
//    }

//    const unsigned int i = sourceRockInput.getI ();
//    const unsigned int j = sourceRockInput.getJ ();
   
//    const double temperature = sourceRockInput.GetTemperatureKelvin ();
//    const double porePressure = NumericFunctions::Maximum <double>( SurfacePressure, sourceRockInput.getPorePressure ());
//    const double lithostaticPressure = sourceRockInput.getLithostaticPressure ();
//    const double porosity = sourceRockInput.getPorosity ();

//    const double& temperatureKelvin = temperature;
//    const double  temperatureCelcius = temperature - Constants::s_TCabs;

//    unsigned int k;

//    double adsorptionCapacity;
//    double c1Expelled;

   

//    SpeciesState* c1State = simulatorState->GetSpeciesStateByName ( "C1" );

//    c1ExpelledTransient = c1State->getExpelledMassTransient ();
//    c1Expelled = c1State->GetExpelledMass ();

//    // Adsorption Capacity in cm^3 of Methane (STP) / gram of rock
//    adsorptionCapacity = getAdsorptionCapacity ( i, j, temperatureCelcius, 1.0e-6 * porePressure, ComponentManager::C1 );

//    // Methane adsorption capacity in vol methane (STP) / vol rock
//    gasCapacity = inorganicDensity * 1000.0 * adsorptionCapacity / 1.0e6;
//    expelledGas = c1ExpelledTransient / thickness; // In kg / m2 / thickness = kg / m3.
//    freeGasMol = c1State->getFreeMol ();
//    adsorpedGasMol = c1State->getAdsorpedMol ();

//    // Convert to moles/m3
//    expelledGasMol = expelledGas * 1000.0 / molarComponentMasses [ pvtFlash::C1 ]; // In kg * 1000 / m3 / g / mol = mol / m3 
//    gasCapacityMol = gasCapacity * 42.306553; // In mol / m3 

//    adsorpedTransient = 0.0;
//    desorpedTransient = 0.0;

//    if ( adsorptionCapacity >= c1Adsorped ) {
//       double c1AdsorpedNew = NumericFunctions::Minimum<double>( adsorptionCapacity, c1Adsorped + c1ExpelledTransient );
//       adsorpedTransient = c1AdsorpedNew - c1Adsorped;
//       c1Adsorped = c1AdsorpedNew;
//    } else {
//       desorpedTransient = c1Adsorped - adsorptionCapacity;
//       c1Adsorped += desorpedTransient;
//    }

//    c1Expelled += desorpedTransient - adsorpedTransient;
//    c1State->SetExpelledMass ( c1Expelled );

//    c1State->setAdsorpedMol ( adsorpedGasMol );
//    c1State->setFreeMol ( freeGasMol );
//    c1State->setExpelledMol ( c1State->getExpelledMol () + expelledGasMol );

// }

void C1AdsorptionSimulator::compute ( const double                thickness,
                                      const double                inorganicDensity,
                                      const SourceRockNodeInput&  sourceRockInput,
                                            SourceRockNodeOutput& sourceRockOutput,
                                            SimulatorState*       simulatorState ) {

   // Pa.
   const double SurfacePressure = 1.01325e5;
   
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
   const double lithostaticPressure = sourceRockInput.getLithostaticPressure ();
   const double porosity = sourceRockInput.getPorosity ();

   double adsorptionCapacity;
   const double& temperatureKelvin = temperature;
   const double  temperatureCelcius = temperature - Constants::s_TCabs;

   bool output = false;
//    bool output = ( i == 10 and j == 10 );

   double freeGasMol;
   double adsorpedGasMol;
   double expelledGas;
   double porosityCapacity;
   double c1MolarVolume;
   double excessCapacityMol;
   double expelledGasMol;
   double gasCapacity;
   double gasCapacityMol;

   double c1ExpelledMass;
   double c1ExpelledMol;
   double c1ExpelledTransient;
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

   SpeciesState* c1State = simulatorState->GetSpeciesStateByName ( "C1" );

   molarComponentMasses [ pvtFlash::C1 ] = 16.043; // In g/mol.

   // Standard Conditions
   // Temperature = 288.70556 K
   // Pressure = 101.325 kPa

   molarComponentMasses [ pvtFlash::COX ] = 0.0;
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

   // kg/m2
   c1ExpelledMass = c1State->GetExpelledMass ();

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

   // How much was expelled for this time-step (moles).
   c1ExpelledMol = desorpedTransient - adsorpedTransient;

   // How much was expelled for this time-step (kg/m2).
   c1ExpelledMass += c1ExpelledMol * thickness / 1000.0 * molarComponentMasses [ pvtFlash::C1 ];

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

   c1State->SetExpelledMass ( c1ExpelledMass, false );
   c1State->setAdsorpedMol ( adsorpedGasMol );
   c1State->setDesorpedMol ( desorpedTransient + c1State->getDesorpedMol ());
   c1State->setFreeMol ( freeGasMol );
   c1State->setExpelledMol ( c1State->getExpelledMol () + expelledGasMol );

}


bool C1AdsorptionSimulator::speciesIsSimulated ( const ComponentManager::SpeciesNamesId species ) const {
   return s_speciesIsSimulated [ species ];
}

const std::string& C1AdsorptionSimulator::getAdsorpedSpeciesName ( const CBMGenerics::ComponentManager::SpeciesNamesId species ) const {

   if ( species == ComponentManager::C1 ) {
      return s_c1AdsorpedName;
   } else {
      return s_nullString;
   }

}

const std::string& C1AdsorptionSimulator::getExpelledSpeciesName ( const CBMGenerics::ComponentManager::SpeciesNamesId species ) const {

   if ( species == ComponentManager::C1 ) {
      return s_c1ExpelledName;
   } else {
      return s_nullString;
   }

}

const std::string& C1AdsorptionSimulator::getFreeSpeciesName ( const CBMGenerics::ComponentManager::SpeciesNamesId species ) const {

   if ( species == ComponentManager::C1 ) {
      return s_c1FreeName;
   } else {
      return s_nullString;
   }

}

Genex5::AdsorptionSimulator* allocateC1AdsorptionSimulator ( AdsorptionProjectHandle* projectHandle, bool applyAdsorption ) {
   return new C1AdsorptionSimulator ( projectHandle );
}
