#include "OTGCC1AdsorptionSimulator.h"

#include <iostream>
#include <iomanip>
using namespace std;

#include "NumericFunctions.h"

#include "PhysicalConstants.h"
#include "Constants.h"

#include "SimulatorState.h"

#include "AdsorptionProjectHandle.h"

#include "OTGC_kernel/src/SpeciesState.h"
#include "OTGC_kernel/src/SimulatorState.h"
#include "SpeciesState.h"

#include "ImmobileSpecies.h"

#include "PVTCalculator.h"

using namespace CBMGenerics;

const double OTGCC1AdsorptionSimulator::AdsorptionPorosityThreshold = 0.1;

const std::string OTGCC1AdsorptionSimulator::s_nullString = "";
const std::string OTGCC1AdsorptionSimulator::s_c1Name = ComponentManager::getInstance ().GetSpeciesName ( ComponentManager::C1 );
const std::string OTGCC1AdsorptionSimulator::s_c1AdsorpedName = ComponentManager::getInstance ().GetSpeciesName ( ComponentManager::C1 ) + "Adsorped";
const std::string OTGCC1AdsorptionSimulator::s_c1ExpelledName = ComponentManager::getInstance ().GetSpeciesName ( ComponentManager::C1 ) + "AdsorpedExpelled";
const std::string OTGCC1AdsorptionSimulator::s_c1FreeName = ComponentManager::getInstance ().GetSpeciesName ( ComponentManager::C1 ) + "AdsorpedFree";

const bool OTGCC1AdsorptionSimulator::s_speciesIsSimulated  [ ComponentManager::NumberOfOutputSpecies ] =
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

OTGCC1AdsorptionSimulator::OTGCC1AdsorptionSimulator ( AdsorptionProjectHandle* projectHandle, bool applyOTGC ) :
   m_c1AdsorptionSimulator ( projectHandle )
{

   const char *OTGC5DIR = getenv("OTGCDIR"); 

   if(OTGC5DIR != 0)
   {
      m_otgcSimulator = new OTGC::Simulator(OTGC5DIR);
   }
   else
   {
      //should throw instead...
      std::cout<<" MeSsAgE WARNING: OTGCDIR environment variable is not set. No OTGC functionality is available"<<std::endl;
      m_otgcSimulator = 0;
   }

   m_irreducibleWaterSaturation = new IrreducibleWaterSaturationFunction ( projectHandle );
   m_applyOtgc = m_otgcSimulator != 0 and applyOTGC;
}


OTGCC1AdsorptionSimulator::~OTGCC1AdsorptionSimulator () {

   // Set the adsorption function to null so that it is not deallocated twice.
   m_c1AdsorptionSimulator.setAdsorptionFunction ( 0 );

   if ( m_otgcSimulator != 0 ) {
      delete m_otgcSimulator;
      m_otgcSimulator = 0;
   }

}

void OTGCC1AdsorptionSimulator::setAdsorptionFunction ( Genex5::AdsorptionFunction* newAdsorptionFunction ) {

   Genex5::AdsorptionSimulator::setAdsorptionFunction ( newAdsorptionFunction );
   m_c1AdsorptionSimulator.setAdsorptionFunction ( newAdsorptionFunction );

}

void OTGCC1AdsorptionSimulator::compute ( const double                        thickness,
                                          const double                        meanBulkDensity,
                                          const Genex5::SourceRockNodeInput&  sourceRockInput,
                                                Genex5::SourceRockNodeOutput& sourceRockOutput,
                                                Genex5::SimulatorState*       simulatorState ) {

   const double LiquidVolumeTolerance = 1.0e-10;
   const double VapourVolumeTolerance = 1.0e-10;

   // Pa.
   const double SurfacePressure = 1.01325e5;
   
   if ( thickness < 1.0e-2 ) {
      return;
   }

   unsigned int k;

   Genex5::PVTComponents      componentMasses;
   Genex5::PVTComponentMasses phaseComponentMasses;
   Genex5::PVTPhaseValues     phaseDensity;
   Genex5::PVTPhaseValues     phaseViscosity;

   Genex5::PVTComponents      otgcComponentMasses;

   const double startTime = sourceRockInput.GetPreviousTime ();
   const double endTime   = sourceRockInput.GetCurrentTime ();

   const unsigned int i = sourceRockInput.getI ();
   const unsigned int j = sourceRockInput.getJ ();

   const double temperature = sourceRockInput.GetTemperatureKelvin ();
   const double porePressure = NumericFunctions::Maximum <double>( SurfacePressure, sourceRockInput.getPorePressure ());

   const double previousTemperatureCelsius = sourceRockInput.getPreviousTemperatureCelsius ();
   const double previousPorePressure = NumericFunctions::Maximum <double>( SurfacePressure, sourceRockInput.getPreviousPorePressure ());

   const double porosity = sourceRockInput.getPorosity ();
   const double permeability = sourceRockInput.getPermeability ();

   // This is the porosiy available after the bitumen has been added to the existing porosity.
   double usablePorosity;

   const double& temperatureKelvin = temperature;
   const double  temperatureCelsius = temperature - Genex5::Constants::s_TCabs;

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

   bool output = false;
//    bool output = ( i == 44 and j == 3 );
//    bool output = ( i == 10 and j == 10 );
//    bool output = ( i == 1 and j == 1 );
//    bool output = ( i == 8 and j == 3 );
//    bool output = ( i == 3 and j == 8 );
//    bool output = (( startTime == 0.0 or endTime == 0.0 ) and i == 10 and j == 10 );
//    bool output = (( endTime == 150.0 or startTime == 150.0 or startTime == 80.0 or endTime == 80.0 or startTime == 0.0 or endTime == 0.0 ) and i == 10 and j == 10 );

   Genex5::SpeciesState* speciesState;

   std::map<string, double> speciesConcentrations;
   std::map<string, double> retainedSpeciesConcentrations;
   std::map<string, double>::iterator speciesConcentrationIter;

   std::vector<std::string> nullArray;

   // What to do if the otgc-simulator is null?
   const std::vector<std::string>& speciesNames = ( m_otgcSimulator != 0 ? m_otgcSimulator->getSpeciesInChemicalModel () : nullArray );
   Genex5::ImmobileSpecies& immobiles = simulatorState->getImmobileSpecies ();

   unsigned int species;
   double totalRetainedMass = 0.0;

   if ( output ) {
      cout << endl;
      cout << "**************** BEGIN ****************" << endl;
      cout << " Current time (interval): " << startTime << "  " << endTime << endl;
   }

   if ( output ) {

      cout << endl;
      cout << " Input cauldron properties: " << endl;
      cout << "       porosity      : " << porosity << endl;
      cout << "       temperature   : " << temperature << endl;
      cout << "       pore-pressure : " << porePressure << endl;
      cout << "       permeability  : " << permeability << endl;
      cout << "       thickness     : " << thickness << endl;
      cout << "       bulk-density  : " << meanBulkDensity << endl;
      cout << endl;


      speciesState = simulatorState->GetSpeciesStateByName ( "C1" );
      cout << " immobiles START: " << i << "  " << j << "  "<< immobiles.image () << endl;
      cout << "C1 before: " << speciesState->GetExpelledMass () << "  " 
           << speciesState->getAdsorpedMol () << "  "
           << speciesState->getExpelledMol () << "  "
           << speciesState->getRetained () << "  "
           << endl;
   }

   m_c1AdsorptionSimulator.compute ( thickness, meanBulkDensity, sourceRockInput, sourceRockOutput, simulatorState );

   if ( output ) {
      cout << "-------------------------------- BEGIN --------------------------------" << endl;
      cout.flags (ios::scientific );
      cout.precision ( 6 );
   }

   // Collect all modelled species for OTGC.
   for ( species = 0; species < speciesNames.size (); ++species ) {
      const std::string& name = speciesNames [ species ];

      int speciesId = CBMGenerics::ComponentManager::getInstance ().GetSpeciedIdByName ( name );

      if ( output ) {
         cout << name << "   ";
      }

      speciesState = simulatorState->GetSpeciesStateByName ( name );

      if ( speciesId != -1 ) {
         double speciesRetained = speciesState->getRetained () + speciesState->GetExpelledMass () - speciesState->getPreviousExpelledMass ();

         CBMGenerics::ComponentManager::SpeciesNamesId id = CBMGenerics::ComponentManager::SpeciesNamesId ( speciesId );

         if ( CBMGenerics::ComponentManager::getInstance ().isGas ( id )) {
            hcGasBeforeOtgc += speciesRetained;
         }

         retainedSpeciesConcentrations [ name ] = speciesRetained;

         if ( output ) {
            cout << "  mob: " << speciesState->getRetained ();
         }

      } else {

         Genex5::ImmobileSpecies::SpeciesId id;
         
         id = immobiles.getId ( name );

         if ( id != Genex5::ImmobileSpecies::UNKNOWN ) {
            retainedSpeciesConcentrations [ name ] = immobiles.getRetained ( id );

            if ( output ) {
               cout << " immob:  " << immobiles.getName ( id ) << "  " << immobiles.getRetained ( id );
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
         speciesConcentrationIter->second /= totalRetainedMass;

         if ( output ) {
            cout << " before fractions: " << setw ( 13 ) << speciesConcentrationIter->first << "  " << setw ( 10 ) << speciesConcentrationIter->second << endl;
         }

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
      OTGC::SimulatorState otgcState ( startTime, speciesNames, retainedSpeciesConcentrations );

      m_otgcSimulator->computeInterval ( otgcState, previousTemperatureCelsius, temperatureCelsius, previousPorePressure, porePressure, startTime, endTime );
      otgcState.GetSpeciesStateConcentrations ( retainedSpeciesConcentrations );
   }

   hcGasAfterOtgc = 0.0;

   if ( totalRetainedMass > 0.0 ) {

      for ( speciesConcentrationIter = retainedSpeciesConcentrations.begin (); speciesConcentrationIter != retainedSpeciesConcentrations.end (); ++speciesConcentrationIter ) {

         if ( output ) {
            cout << " after fractions: " << setw ( 13 ) << speciesConcentrationIter->first << "  " << setw ( 10 ) << speciesConcentrationIter->second << endl;
         }

         speciesConcentrationIter->second *= totalRetainedMass;

         int speciesId = CBMGenerics::ComponentManager::getInstance ().GetSpeciedIdByName ( speciesConcentrationIter->first );

         if ( speciesId != -1 and CBMGenerics::ComponentManager::getInstance ().isGas ( CBMGenerics::ComponentManager::SpeciesNamesId ( speciesId ))) {
            hcGasAfterOtgc += speciesConcentrationIter->second;
         }

      }

   }

   simulatorState->incrementTotalGasFromOtgc ( NumericFunctions::Maximum ( 0.0, hcGasAfterOtgc - hcGasBeforeOtgc ));

   // Set the new masses back to the species-state and the immobile-species-state.
   for ( speciesConcentrationIter = retainedSpeciesConcentrations.begin (); speciesConcentrationIter != retainedSpeciesConcentrations.end (); ++speciesConcentrationIter ) {

      const std::string& speciesName = speciesConcentrationIter->first;
      double speciesMass = speciesConcentrationIter->second;

      int speciesId = CBMGenerics::ComponentManager::getInstance ().GetSpeciedIdByName ( speciesName );

      if ( output ) {
         cout << speciesName << "   " << speciesId;
      }

      if ( speciesId != -1 ) {
         // Get all the mobile species for later pvt-flash computation.
         componentMasses ( pvtFlash::ComponentId ( speciesId )) = speciesMass;
         speciesState = simulatorState->GetSpeciesStateByName ( speciesName );

         if ( output ) {
            cout << "  mob retained: " << speciesMass;
         }

      } else {

         Genex5::ImmobileSpecies::SpeciesId id;
         
         id = immobiles.getId ( speciesName );

         if ( id != Genex5::ImmobileSpecies::UNKNOWN ) {
            immobiles.setRetained ( id, speciesMass );

            if ( output ) {
               cout << " immob retained:  " << immobiles.getRetained ( id );
            }

         }

      }

      if ( output ) {
         cout << endl;
      }

   }

   componentMasses ( pvtFlash::COX ) = 0.0;

   if ( output ) {
      cout << " pre-pvt components " << endl << componentMasses.image () << endl;
   }

   Genex5::PVTCalc::getInstance ().compute ( temperatureKelvin, porePressure, componentMasses, phaseComponentMasses, phaseDensity, phaseViscosity );

   liquidVolume = phaseComponentMasses.sum ( pvtFlash::LIQUID_PHASE ) / ( thickness * phaseDensity ( pvtFlash::LIQUID_PHASE ));
   vapourVolume = phaseComponentMasses.sum ( pvtFlash::VAPOUR_PHASE ) / ( thickness * phaseDensity ( pvtFlash::VAPOUR_PHASE ));

   bitumenVolume = immobiles.getRetainedVolume ( thickness );
   usablePorosity = NumericFunctions::Maximum ( 0.0, porosity - bitumenVolume );

   if ( usablePorosity == 0.0 ) {
      hcSaturation = 0.0;
   } else {
      hcSaturation = ( liquidVolume + vapourVolume ) / usablePorosity;
   }

   irreducibleWaterSaturation = m_irreducibleWaterSaturation->evaluate ( permeability );

   retainedVapourVolume = vapourVolume;
   retainedLiquidVolume = liquidVolume;

   if ( hcSaturation > 1.0 - irreducibleWaterSaturation ) {
      excessHcVolume = ( hcSaturation - ( 1.0 - irreducibleWaterSaturation )) * usablePorosity;

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
           << usablePorosity << "  "
           << endl;
   } 

   Genex5::PVTComponents vapourComponents;
   Genex5::PVTComponents liquidComponents;

   phaseComponentMasses.getPhaseComponents ( pvtFlash::LIQUID_PHASE, liquidComponents );
   phaseComponentMasses.getPhaseComponents ( pvtFlash::VAPOUR_PHASE, vapourComponents );


   if ( output ) {
      cout << " phase component sums: " << phaseComponentMasses.sum ( pvtFlash::LIQUID_PHASE ) << "  " << phaseComponentMasses.sum ( pvtFlash::VAPOUR_PHASE ) << endl;
   }

   if ( output ) {
      cout << endl << " liquid scaling factors: " << retainedLiquidVolume << "  " << liquidVolume << "  " 
           << phaseComponentMasses.sum ( pvtFlash::LIQUID_PHASE ) << endl;
      cout << " all liquid compounds: " << endl << phaseComponentMasses.image () << endl;
      cout << " retained liquids:     " << liquidComponents.image () << endl;
   }

   for ( species = 0; species < speciesNames.size (); ++species ) {
      const std::string& name = speciesNames [ species ];

      int speciesId = CBMGenerics::ComponentManager::getInstance ().GetSpeciedIdByName ( name );

      speciesState = simulatorState->GetSpeciesStateByName ( name );

      if ( speciesId != -1 and speciesState != 0 ) {
         expelled = 0.0;

         if ( std::fabs ( liquidVolume  ) > LiquidVolumeTolerance ) {
            expelled = liquidComponents ( pvtFlash::ComponentId ( speciesId )) * ( 1.0 - retainedLiquidVolume / liquidVolume );
         }

         if ( std::fabs ( vapourVolume ) > VapourVolumeTolerance ) {
            expelled += vapourComponents ( pvtFlash::ComponentId ( speciesId )) * ( 1.0 - retainedVapourVolume / vapourVolume );
         }

         if ( output ) {
            cout << " species: " << name << ", before:  " << speciesState->getMassExpelledFromSourceRock () << "  " 
                 << retainedLiquidVolume << "  " << liquidVolume << "  " << retainedVapourVolume << "  " << vapourVolume << "  ";
         }

         speciesState->setMassExpelledFromSourceRock ( speciesState->getMassExpelledFromSourceRock () + expelled );

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
   }


   simulatorState->setSubSurfaceDensities ( phaseDensity );
   simulatorState->setRetainedVolumes ( retainedVapourVolume, retainedLiquidVolume );
   simulatorState->setUsablePorosity ( usablePorosity );

   simulatorState->setLiquidComponents ( liquidComponents );
   simulatorState->setVapourComponents ( vapourComponents );

   for ( species = 0; species < speciesNames.size (); ++species ) {
      const std::string& name = speciesNames [ species ];
      double retained = 0.0;

      int speciesId = CBMGenerics::ComponentManager::getInstance ().GetSpeciedIdByName ( name );

      speciesState = simulatorState->GetSpeciesStateByName ( name );

      if ( speciesId != -1 and speciesState != 0 ) {
         expelled = 0.0;

         if ( std::fabs ( liquidVolume  ) > LiquidVolumeTolerance ) {
            retained = simulatorState->getLiquidComponents ()( pvtFlash::ComponentId ( speciesId ));
         }

         if ( std::fabs ( vapourVolume ) > VapourVolumeTolerance ) {
            retained += simulatorState->getVapourComponents ()( pvtFlash::ComponentId ( speciesId ));
         }

         speciesState->setRetained ( retained );

         if ( output ) {
            cout << " species retained: " << name << "  " << speciesState->getRetained () << endl;
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
      speciesState = simulatorState->GetSpeciesStateByName ( "C1" );
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


bool OTGCC1AdsorptionSimulator::speciesIsSimulated ( const ComponentManager::SpeciesNamesId species ) const {
   return s_speciesIsSimulated [ species ];
}

const std::string& OTGCC1AdsorptionSimulator::getAdsorpedSpeciesName ( const CBMGenerics::ComponentManager::SpeciesNamesId species ) const {

   if ( species == ComponentManager::C1 ) {
      return s_c1AdsorpedName;
   } else {
      return s_nullString;
   }

}

const std::string& OTGCC1AdsorptionSimulator::getExpelledSpeciesName ( const CBMGenerics::ComponentManager::SpeciesNamesId species ) const {

   if ( species == ComponentManager::C1 ) {
      return s_c1ExpelledName;
   } else {
      return s_nullString;
   }

}

const std::string& OTGCC1AdsorptionSimulator::getFreeSpeciesName ( const CBMGenerics::ComponentManager::SpeciesNamesId species ) const {

   if ( species == ComponentManager::C1 ) {
      return s_c1FreeName;
   } else {
      return s_nullString;
   }

}

Genex5::AdsorptionSimulator* allocateOTGCC1AdsorptionSimulator ( AdsorptionProjectHandle* projectHandle, bool applyAdsorption ) {
   return new OTGCC1AdsorptionSimulator ( projectHandle, applyAdsorption );
}

