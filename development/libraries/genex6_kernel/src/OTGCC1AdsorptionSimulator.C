//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 
#include "OTGCC1AdsorptionSimulator.h"

// std library
#include <iostream>
#include <iomanip>

#include "SGDensitySample.h"

#include "ConstantsGenex.h"
#include "ChemicalModel.h"

#include "SimulatorState.h"

#include "../../OTGC_kernel6/src/SimulatorState.h"
#include "SpeciesState.h"

#include "ImmobileSpecies.h"

#include "PVTCalculator.h"

// utilitites library
#include "NumericFunctions.h"
#include "ConstantsMathematics.h"

// CBMGenerics library
#include "ComponentManager.h"
typedef CBMGenerics::ComponentManager::SpeciesNamesId ComponentId;

using Utilities::Maths::CelciusToKelvin;
using namespace std;
using namespace CBMGenerics;

const double Genex6::OTGCC1AdsorptionSimulator::AdsorptionPorosityThreshold = 0.1;

const std::string Genex6::OTGCC1AdsorptionSimulator::s_nullString = "";
const std::string Genex6::OTGCC1AdsorptionSimulator::s_c1Name         = ComponentManager::getInstance ().getSpeciesName ( ComponentManager::C1 );
const std::string Genex6::OTGCC1AdsorptionSimulator::s_c1AdsorpedName = ComponentManager::getInstance ().getSpeciesName ( ComponentManager::C1 ) + "Adsorped";
const std::string Genex6::OTGCC1AdsorptionSimulator::s_c1ExpelledName = ComponentManager::getInstance ().getSpeciesName ( ComponentManager::C1 ) + "AdsorpedExpelled";
const std::string Genex6::OTGCC1AdsorptionSimulator::s_c1FreeName     = ComponentManager::getInstance ().getSpeciesName ( ComponentManager::C1 ) + "AdsorpedFree";

const bool Genex6::OTGCC1AdsorptionSimulator::s_speciesIsSimulated  [ ComponentManager::NUMBER_OF_SPECIES ] =
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

Genex6::OTGCC1AdsorptionSimulator::OTGCC1AdsorptionSimulator ( DataAccess::Interface::ProjectHandle* projectHandle, 
                                                               const SpeciesManager& speciesManager,
                                                               const bool applyOTGC,
                                                               const bool isManaged ) :
   Genex6::AdsorptionSimulator ( projectHandle, isManaged ),
   m_speciesManager ( speciesManager ),
   m_c1AdsorptionSimulator ( speciesManager, projectHandle, true )
{

   const char *OTGCDIR;
   bool containsSulphur = speciesManager.isSulphur(); //projectHandle->containsSulphur ();

#if 0
   if ( containsSulphur ) {
      OTGCDIR = getenv("OTGC6DIR"); 
   } else {
      OTGCDIR = getenv("OTGCDIR"); 
   }
#endif

   OTGCDIR = getenv("OTGCDIR"); 

   int runType = ( containsSulphur ? Genex6::Constants::SIMOTGC : (Genex6::Constants::SIMOTGC | Genex6::Constants::SIMOTGC5) );

   //   cout << "Adsorption contains Sulphur -  " << ( containsSulphur ? "yes" : "no" ) << ";" << " Apply OTGC - " << ( applyOTGC ? "yes" : "no" ) << endl;

   if ( applyOTGC ) {

      if( OTGCDIR != 0 ) {

         if( containsSulphur) {
            // H/C = 1.8, S/C = 0.035
            m_otgcSimulator = new Genex6::Simulator(OTGCDIR, runType, "TypeII_GX6", 1.8, 0.035);
         } else {
            m_otgcSimulator = new Genex6::Simulator(OTGCDIR, runType);
         }

#if 0
         if ( containsSulphur ) {
            m_otgcSimulator = new Genex6::Simulator(OTGCDIR, Constants::SIMOTGC );
         } else {
            m_otgcSimulator = new Genex6::Simulator(OTGCDIR, Constants::SIMOTGC | Constants::SIMOTGC5 );
         }
#endif

      }
      else
      {
         //should throw instead...
         std::string s = "OTGCDIR environment variable is not set.";
         m_otgcSimulator = 0;

         throw s;
      }

   } else {
      m_otgcSimulator = 0;
   }

   m_irreducibleWaterSaturation = new IrreducibleWaterSaturationFunction ( projectHandle );
   m_applyOtgc = m_otgcSimulator != 0 and applyOTGC;
}


Genex6::OTGCC1AdsorptionSimulator::~OTGCC1AdsorptionSimulator () {

   // Set the adsorption function to null so that it is not deallocated twice.
   m_c1AdsorptionSimulator.setAdsorptionFunction ( 0 );

   if ( m_otgcSimulator != 0 ) {
      delete m_otgcSimulator;
      m_otgcSimulator = 0;
   }

}

void Genex6::OTGCC1AdsorptionSimulator::setAdsorptionFunction ( Genex6::AdsorptionFunction* newAdsorptionFunction ) {

   Genex6::AdsorptionSimulator::setAdsorptionFunction ( newAdsorptionFunction );
   m_c1AdsorptionSimulator.setAdsorptionFunction ( newAdsorptionFunction );

}

void Genex6::OTGCC1AdsorptionSimulator::compute ( const Genex6::Input&              sourceRockInput,
                                                        Genex6::SimulatorStateBase* baseState ) {

   SimulatorState* simulatorState = dynamic_cast<SimulatorState*>( baseState );

   if ( simulatorState == 0 ) {
      // Error?
   }

   const double LiquidVolumeTolerance = 1.0e-10;
   const double VapourVolumeTolerance = 1.0e-10;

   const double thickness = simulatorState->GetThickness ();
   const double meanBulkDensity = getProjectHandle ()->getSGDensitySample ()->getDensity ();

   // Pa.
   const double SurfacePressure = 1.01325e5;
   
   if ( thickness < 1.0e-2 ) {
      return;
   }

   // unsigned int k;

   Genex6::PVTComponents      componentMasses;
   Genex6::PVTComponentMasses phaseComponentMasses;
   Genex6::PVTPhaseValues     phaseDensity;
   Genex6::PVTPhaseValues     phaseViscosity;

   Genex6::PVTComponents      otgcComponentMasses;

   const double startTime = sourceRockInput.GetPreviousTime ();
   const double endTime   = sourceRockInput.GetTime ();

   const unsigned int i = sourceRockInput.getI ();
   const unsigned int j = sourceRockInput.getJ ();

   const double temperature = sourceRockInput.GetTemperatureKelvin ();
   const double porePressure = NumericFunctions::Maximum <double>( SurfacePressure, sourceRockInput.getPorePressure ());

   const double previousTemperatureCelsius = sourceRockInput.getPreviousTemperatureCelsius ();
   const double previousPorePressure = NumericFunctions::Maximum <double>( SurfacePressure, sourceRockInput.getPreviousPorePressure ());

   const double porosity = sourceRockInput.getPorosity ();
   const double permeability = sourceRockInput.getPermeability ();

   // This is the porosiy available after the bitumen has been added to the existing porosity.
   double effectivePorosity;

   const double& temperatureKelvin = temperature;
   const double  temperatureCelsius = temperature - CelciusToKelvin;

   double expelled;
   double liquidVolume = 0.0;
   double vapourVolume = 0.0;
   double bitumenVolume = 0.0;

   double hcSaturation;
   double irreducibleWaterSaturation;
   double retainedVapourVolume;
   double retainedLiquidVolume;
   double excessHcVolume = 0.0;

   double hcGasBeforeOtgc = 0.0;
   double hcGasAfterOtgc;

   int s;
   bool output = false;
   //bool output = ( i == 0 and j == 0 );
   // bool output = (( startTime == 0.0 or endTime == 0.0 ) and i == 10 and j == 10 );
   // bool output = (( endTime == 150.0 or startTime == 150.0 or startTime == 80.0 or endTime == 80.0 or startTime == 0.0 or endTime == 0.0 ) and i == 10 and j == 10 );

   // Genex6::SpeciesState* speciesState;

   std::map<string, double> retainedSpeciesConcentrations;
   std::map<string, double>::iterator speciesConcentrationIter;

   std::vector<std::string> nullArray;

   //   const SpeciesManager& speciesManager = m_genexSimulator->getSpeciesManager ();
   Genex6::ImmobileSpecies& immobiles = simulatorState->getImmobileSpecies ();

   double totalRetainedMass = 0.0;

   if ( output ) {
      cout << endl;
      cout << "**************** BEGIN ****************" << endl;
      cout << " Current time (interval): " << startTime << "  " << endTime << endl;
   }

   if ( output ) {
      Genex6::SpeciesState* speciesState;

      cout << endl;
      cout << " Input cauldron properties: " << endl;
      cout << "       porosity      : " << porosity << endl;
      cout << "       temperature   : " << temperature << endl;
      cout << "       pore-pressure : " << porePressure << endl;
      cout << "       permeability  : " << permeability << endl;
      cout << "       thickness     : " << thickness << endl;
      cout << "       bulk-density  : " << meanBulkDensity << endl;
      cout << endl;


      speciesState = simulatorState->GetSpeciesStateById ( m_speciesManager.getC1Id ());
      cout << " immobiles START: " << i << "  " << j << "  "<< immobiles.image () << endl;
      cout << "C1 before: " << speciesState->GetExpelledMass () << "  " 
           << speciesState->getAdsorpedMol () << "  "
           << speciesState->getExpelledMol () << "  "
           << speciesState->getRetained () << "  "
           << endl;
   }

   m_c1AdsorptionSimulator.compute ( sourceRockInput, simulatorState );

   if ( output ) {
      cout << "-------------------------------- BEGIN --------------------------------" << endl;
      cout.flags (ios::scientific );
      cout.precision ( 17 );
   }


   for ( s = 1; s <= m_speciesManager.getNumberOfSpecies (); ++s ) {

      const SpeciesState* speciesState = simulatorState->GetSpeciesStateById ( s );

      if ( speciesState == 0 ) {
         continue;
      }

      const std::string& speciesName = speciesState->getSpecies ()->GetName ();

      int speciesId = speciesState->getSpecies ()->GetId ();

      if ( output ) {
         cout << setw ( 20 ) << speciesName << "   " << setw ( 4 ) << speciesId << "  ";
      }

      CBMGenerics::ComponentManager::SpeciesNamesId componentId = m_speciesManager.mapIdToComponentManagerSpecies ( speciesId );

      if ( componentId == CBMGenerics::ComponentManager::H2S ) {
         // H2S is not added to the retained components. for passing to otgc.
         // Only its transient value is added to current state.

         simulatorState->addH2SFromGenex ( speciesState->getExpelledMassTransient ());
      } else if ( componentId != CBMGenerics::ComponentManager::UNKNOWN ) {

         double speciesRetained = speciesState->getRetained () + speciesState->GetExpelledMass () - speciesState->getPreviousExpelledMass ();

         if ( m_c1AdsorptionSimulator.speciesIsSimulated ( componentId )) {
            // If the species is being modelled in the adsorption then also remove what was adsorped over the time-step.
            speciesRetained -= speciesState->getTransientAdsorpedMass ();
            // And add what was desorped over the time-step.
            speciesRetained += speciesState->getTransientDesorpedMass ();
         }

         if ( CBMGenerics::ComponentManager::getInstance ().isGas ( componentId )) {
            hcGasBeforeOtgc += speciesRetained;
         }

         retainedSpeciesConcentrations [ speciesName ] = speciesRetained;

         if ( output ) {
            cout << "   mob: "
                 << setw ( 25 ) << speciesState->getRetained () << "  "
                 << setw ( 25 ) << speciesState->GetExpelledMass () << "  "
                 << setw ( 25 ) << speciesState->getPreviousExpelledMass ();
         }

      } else {

         Genex6::ImmobileSpecies::SpeciesId id;
         
         id = immobiles.getId ( speciesName );

         if ( id != Genex6::ImmobileSpecies::UNKNOWN ) {
            retainedSpeciesConcentrations [ speciesName ] = immobiles.getRetained ( id );

            if ( output ) {
               cout << " immob:  " << setw ( 20 ) << immobiles.getName ( id ) << "  "
                    << setw ( 25 ) << immobiles.getRetained ( id );
            }

         }

      }

      if ( output ) {
         cout << "   " << endl;
      }

   }

   totalRetainedMass = 0.0;

   for ( speciesConcentrationIter = retainedSpeciesConcentrations.begin (); speciesConcentrationIter != retainedSpeciesConcentrations.end (); ++speciesConcentrationIter ) {
      totalRetainedMass += speciesConcentrationIter->second;
   }
   
   if ( totalRetainedMass > 0.0 ) {

      for ( speciesConcentrationIter = retainedSpeciesConcentrations.begin (); speciesConcentrationIter != retainedSpeciesConcentrations.end (); ++speciesConcentrationIter ) {

         if ( output ) {
            cout << " before fractions: " << setw ( 13 ) << speciesConcentrationIter->first << "  " << setw ( 10 ) << speciesConcentrationIter->second << endl;
         }

         speciesConcentrationIter->second /= totalRetainedMass;
      }

   }

   if ( output ) {
      cout << " totalRetainedMass " << totalRetainedMass << endl;
   }

   if ( output ) {
      cout << "   " << endl;
      cout << "   " << endl;
      cout << "   " << endl;
   }

   if ( m_applyOtgc and m_otgcSimulator != 0 ) {
      OTGC6::SimulatorState otgcState ( startTime, m_otgcSimulator->getSpeciesInChemicalModel (), retainedSpeciesConcentrations );

      m_otgcSimulator->computeInterval ( otgcState, previousTemperatureCelsius, temperatureCelsius, previousPorePressure, porePressure, startTime, endTime );
      otgcState.GetSpeciesStateConcentrations ( &m_otgcSimulator->getChemicalModel (), retainedSpeciesConcentrations );
   }


   hcGasAfterOtgc = 0.0;

   if ( totalRetainedMass > 0.0 ) {

      for ( speciesConcentrationIter = retainedSpeciesConcentrations.begin (); speciesConcentrationIter != retainedSpeciesConcentrations.end (); ++speciesConcentrationIter ) {
         speciesConcentrationIter->second *= totalRetainedMass;

         if ( output ) {
            cout << " after fractions: " << setw ( 13 ) << speciesConcentrationIter->first << "  " << setw ( 10 ) << speciesConcentrationIter->second << endl;
         }

         int speciesId = CBMGenerics::ComponentManager::getInstance ().getSpeciesIdByName ( speciesConcentrationIter->first );

         if ( speciesId != -1 and CBMGenerics::ComponentManager::getInstance ().isGas ( CBMGenerics::ComponentManager::SpeciesNamesId ( speciesId ))) {
            hcGasAfterOtgc += speciesConcentrationIter->second;
         }

      }

   }

   if ( m_applyOtgc and m_otgcSimulator != 0 ) {
      simulatorState->incrementTotalGasFromOtgc ( NumericFunctions::Maximum ( 0.0, hcGasAfterOtgc - hcGasBeforeOtgc ));
   }

   if ( output ) {
      cout << " gas generated by otgc: " << hcGasAfterOtgc - hcGasBeforeOtgc << endl;
   }

   // Set the new masses back to the species-state and the immobile-species-state.
   for ( speciesConcentrationIter = retainedSpeciesConcentrations.begin (); speciesConcentrationIter != retainedSpeciesConcentrations.end (); ++speciesConcentrationIter ) {

      const std::string& speciesName = speciesConcentrationIter->first;
      double speciesMass = speciesConcentrationIter->second;

      int speciesId = m_speciesManager.getSpeciesIdByName ( speciesName );

      ComponentId pvtId = m_speciesManager.mapIdToPvtComponents ( speciesId );

      if ( output ) {
         cout << setw ( 20 ) << speciesName << "   " << setw ( 4 ) << speciesId;
      }

      if ( pvtId != ComponentId::UNKNOWN ) {
         // Get all the mobile species for later pvt-flash computation.
         componentMasses ( pvtId ) = speciesMass;
         // speciesState = simulatorState->GetSpeciesStateById ( speciesId );

         if ( output ) {
            cout << "  mob retained: " << setw ( 25 ) << speciesMass;
         }

      } else {

         Genex6::ImmobileSpecies::SpeciesId id;
         
         id = immobiles.getId ( speciesName );

         if ( id != Genex6::ImmobileSpecies::UNKNOWN ) {
            immobiles.setRetained ( id, speciesMass );

            if ( output ) {
               cout << " immob retained:  " << setw ( 25 ) << immobiles.getRetained ( id );
            }

         }

      }

      if ( output ) {
         cout << endl;
      }

   }

   simulatorState->addH2SFromOtgc ( componentMasses ( ComponentId::H2S ));

   componentMasses ( ComponentId::COX ) = 0.0;
   componentMasses ( ComponentId::H2S ) = 0.0;

   if ( output ) {
      cout << " pre-pvt components " << endl << componentMasses.image () << endl;
   }

   Genex6::PVTCalc::getInstance ().compute ( temperatureKelvin, porePressure, componentMasses, phaseComponentMasses, phaseDensity, phaseViscosity );

   liquidVolume = phaseComponentMasses.sum ( PhaseId::LIQUID ) / ( thickness * phaseDensity ( PhaseId::LIQUID ));
   vapourVolume = phaseComponentMasses.sum ( PhaseId::VAPOUR ) / ( thickness * phaseDensity ( PhaseId::VAPOUR ));

   bitumenVolume = immobiles.getRetainedVolume ( thickness );
   effectivePorosity = NumericFunctions::Maximum ( 0.0, porosity - bitumenVolume );

   if ( effectivePorosity == 0.0 ) {
      hcSaturation = 0.0;
   } else {
      hcSaturation = ( liquidVolume + vapourVolume ) / effectivePorosity;
   }

   irreducibleWaterSaturation = m_irreducibleWaterSaturation->evaluate ( permeability );

   retainedVapourVolume = vapourVolume;
   retainedLiquidVolume = liquidVolume;

   if ( hcSaturation > 1.0 - irreducibleWaterSaturation ) {
      excessHcVolume = ( hcSaturation - ( 1.0 - irreducibleWaterSaturation )) * effectivePorosity;

      if ( excessHcVolume > vapourVolume ) {
         retainedVapourVolume = 0.0;
         excessHcVolume = excessHcVolume - vapourVolume;
         retainedLiquidVolume = liquidVolume - excessHcVolume;
      } else {
         retainedVapourVolume = vapourVolume - excessHcVolume;
      }

      hcSaturation = 1.0 - irreducibleWaterSaturation;
   }

   if ( output ) {
      cout << " intermediate volumes:" 
           << excessHcVolume << "  " 
           << liquidVolume << "  "
           << retainedLiquidVolume << "  "
           << vapourVolume << "  "
           << retainedVapourVolume << "  "
           << hcSaturation << "  " 
           << irreducibleWaterSaturation << "  "
           << effectivePorosity << "  "
           << endl;
   } 

   Genex6::PVTComponents vapourComponents;
   Genex6::PVTComponents liquidComponents;

   phaseComponentMasses.getPhaseComponents ( PhaseId::LIQUID, liquidComponents );
   phaseComponentMasses.getPhaseComponents ( PhaseId::VAPOUR, vapourComponents );


   if ( output ) {
      cout << " phase component sums: " << phaseComponentMasses.sum ( PhaseId::LIQUID ) << "  " << phaseComponentMasses.sum ( PhaseId::VAPOUR ) << endl;
   }

   if ( output ) {
      cout << endl << " liquid scaling factors: " << retainedLiquidVolume << "  " << liquidVolume << "  " 
           << phaseComponentMasses.sum ( PhaseId::LIQUID ) << endl;
      cout << " all liquid compounds: " << endl << phaseComponentMasses.image () << endl;
      cout << " retained liquids:     " << liquidComponents.image () << endl;
   }

   // for ( s = 0; s < numberOfSpeciesInOtgc; ++s  ) {
   for ( s = 1; s <= m_speciesManager.getNumberOfSpecies (); ++s ) {

      SpeciesState* speciesState = simulatorState->GetSpeciesStateById ( s );

      if ( speciesState == 0 ) {
         continue;
      }

      const std::string& name = speciesState->getSpecies ()->GetName ();

      int componentId = CBMGenerics::ComponentManager::getInstance ().getSpeciesIdByName ( name );

      if ( componentId != -1 and speciesState != 0 ) {
         expelled = 0.0;

         if ( std::fabs ( liquidVolume  ) > LiquidVolumeTolerance ) {
            expelled = liquidComponents ( ComponentId ( componentId )) * ( 1.0 - retainedLiquidVolume / liquidVolume );
         }

         if ( std::fabs ( vapourVolume ) > VapourVolumeTolerance ) {
            expelled += vapourComponents ( ComponentId ( componentId )) * ( 1.0 - retainedVapourVolume / vapourVolume );
         }

         if ( output ) {
            cout << " species: " << name << "  " << componentId << "  " << ", before:  " << speciesState->getMassExpelledFromSourceRock () << "  " 
                 << retainedLiquidVolume << "  " << liquidVolume << "  " << retainedVapourVolume << "  " << vapourVolume << "  ";
         }

         speciesState->setMassExpelledFromSourceRock ( speciesState->getMassExpelledFromSourceRock () + expelled );
         speciesState->setMassExpelledTransientFromSourceRock ( expelled );

         if ( output ) {
            cout << ", after: " << speciesState->getMassExpelledFromSourceRock () << "  " << expelled << endl;
         }

      }

   }

   if ( std::fabs ( liquidVolume ) > LiquidVolumeTolerance ) {
      liquidComponents *= retainedLiquidVolume / liquidVolume;
   } else {
      liquidComponents.zero ();
   }

   if ( std::fabs ( vapourVolume ) > VapourVolumeTolerance ) {
      vapourComponents *= retainedVapourVolume / vapourVolume;
   } else {
      vapourComponents.zero ();
   }

   if ( output ) {
      cout << endl << " vapour scaling factors: " << retainedVapourVolume << "  " << vapourVolume << endl;
      cout << " all vapour compounds: " << endl << phaseComponentMasses.image () << endl;
      cout << " retained vapour:     " << vapourComponents.image () << endl;
      cout << " retained liquid:     " << liquidComponents.image () << endl;
   }

   simulatorState->setSubSurfaceDensities ( phaseDensity );
   simulatorState->setRetainedVolumes ( retainedVapourVolume, retainedLiquidVolume );
   simulatorState->setEffectivePorosity ( effectivePorosity );

   simulatorState->setLiquidComponents ( liquidComponents );
   simulatorState->setVapourComponents ( vapourComponents );

   for ( s = 1; s <= m_speciesManager.getNumberOfSpecies (); ++s ) {

      SpeciesState* speciesState = simulatorState->GetSpeciesStateById ( s );

      if ( speciesState == 0 ) {
         continue;
      }

      int speciesId = speciesState->getSpecies ()->GetId ();

      double retained = 0.0;

      ComponentId pvtId = m_speciesManager.mapIdToPvtComponents ( speciesId );

      if ( pvtId != ComponentId::UNKNOWN and speciesState != 0 ) {
         expelled = 0.0;

         if ( std::fabs ( liquidVolume  ) > LiquidVolumeTolerance ) {
            retained = simulatorState->getLiquidComponents ()( pvtId );
         }

         if ( std::fabs ( vapourVolume ) > VapourVolumeTolerance ) {
            retained += simulatorState->getVapourComponents ()( pvtId );
         }

         speciesState->setRetained ( retained );

         if ( output ) {
            cout << " species retained: " << speciesState->getSpecies ()->GetName () << "  " << speciesState->getRetained () << endl;
            // cout << " species retained: " << allSpeciesInOtgc [ s ]->GetName () << "  " << speciesState->getRetained () << endl;
         }

      }

   }

   simulatorState->setHcSaturation ( hcSaturation );
   simulatorState->setIrreducibleWaterSaturation ( irreducibleWaterSaturation );

   if ( output ) {

      cout << " Values: " << hcSaturation << "  " 
           << irreducibleWaterSaturation << "  "
           << vapourVolume << "  "
           << liquidVolume << "  "
           << excessHcVolume << "  "
           << endl;

      cout << endl << " volumes: " << porosity << "  " 
           << vapourVolume << "  " << liquidVolume << "  " << bitumenVolume << "  "
           << vapourVolume + liquidVolume + bitumenVolume  << "  " 
           << porosity - ( vapourVolume + liquidVolume + bitumenVolume )  << "  " 
           << endl;

   }

   if ( output ) {
      SpeciesState* speciesState;
      speciesState = simulatorState->GetSpeciesStateById ( m_speciesManager.getC1Id ());
      cout << " immobiles END: " << i << "  " << j << "  "<< immobiles.image () << endl;
      cout << "C1 after: " << speciesState->GetExpelledMass () << "  " 
           << speciesState->getAdsorpedMol () << "  "
           << speciesState->getExpelledMol () << "  "
           << endl;
   }

   if ( output ) {
      cout << "   " << endl;
      cout << "   " << endl;
      cout << "   " << endl;
   }

}


bool Genex6::OTGCC1AdsorptionSimulator::speciesIsSimulated ( const ComponentManager::SpeciesNamesId species ) const {

   if ( species != ComponentManager::UNKNOWN ) {
      return s_speciesIsSimulated [ species ];
   } else {
      return false;
   }

}

const std::string& Genex6::OTGCC1AdsorptionSimulator::getAdsorpedSpeciesName ( const CBMGenerics::ComponentManager::SpeciesNamesId species ) const {

   if ( species == ComponentManager::C1 ) {
      return s_c1AdsorpedName;
   } else {
      return s_nullString;
   }

}

const std::string& Genex6::OTGCC1AdsorptionSimulator::getExpelledSpeciesName ( const CBMGenerics::ComponentManager::SpeciesNamesId species ) const {

   if ( species == ComponentManager::C1 ) {
      return s_c1ExpelledName;
   } else {
      return s_nullString;
   }

}

const std::string& Genex6::OTGCC1AdsorptionSimulator::getFreeSpeciesName ( const CBMGenerics::ComponentManager::SpeciesNamesId species ) const {

   if ( species == ComponentManager::C1 ) {
      return s_c1FreeName;
   } else {
      return s_nullString;
   }

}

Genex6::AdsorptionSimulator* Genex6::allocateOTGCC1AdsorptionSimulator ( DataAccess::Interface::ProjectHandle* projectHandle,
                                                                         const SpeciesManager &speciesManager,
                                                                         const bool applyAdsorption,
                                                                         const bool isManaged ) {
   return new Genex6::OTGCC1AdsorptionSimulator ( projectHandle, speciesManager, applyAdsorption, isManaged );
}

