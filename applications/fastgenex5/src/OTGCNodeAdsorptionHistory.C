#include "OTGCNodeAdsorptionHistory.h"

#include <iomanip>

#include "PhysicalConstants.h"
#include "Constants.h"
#include "ComponentManager.h"

#include "SourceRockNodeInput.h"
#include "SimulatorState.h"
#include "SpeciesState.h"

#include "EosPack.h"

OTGCNodeAdsorptionHistory::~OTGCNodeAdsorptionHistory () {

   HistoryItemList::iterator histIter;

   for ( histIter = m_history.begin (); histIter != m_history.end (); ++histIter ) {
      delete (*histIter);
   }

   m_history.clear ();
}

OTGCNodeAdsorptionHistory::HistoryItem::HistoryItem () {
   m_time = 0.0;
   m_thickness = 0.0;
   m_temperature = 0.0;
   m_pressure = 0.0;
   m_porosity = 0.0;
   m_permeability = 0.0;
   m_toc = 0.0;
   m_vre = 0.0;
   m_VLSRTemperature = 0.0;
   m_VLReferenceTemperature = 0.0;
   m_c1adsorped = 0.0;
   m_adsorpedFraction = 0.0;
   m_c1desorped = 0.0;
   m_adsorptionCapacity = 0.0;
   m_bitumen = 0.0;
   m_gasRetainedSTSCF = 0.0;
   m_oilRetainedSTBarrels = 0.0;
   m_gasRetainedSTBCF = 0.0;
   m_oilRetainedSTMBarrels = 0.0;
   m_hcLiquidSaturation = 0.0;
   m_hcVapourSaturation = 0.0;
   m_subSurfaceLiquidDensity = 0.0;
   m_subSurfaceVapourDensity = 0.0;
   m_irreducibleWaterSat = 0.0;
   m_hcSaturation = 0.0;
}


void OTGCNodeAdsorptionHistory::collect ( Genex5::SourceRockNode* node ) {

   const Genex5::SourceRockNodeInput* nodeInput = node->getTheLastInput ();
   Genex5::SimulatorState*            simulatorState = const_cast<Genex5::SimulatorState*>(node->getSimulatorState ());

   if ( nodeInput != 0 and simulatorState != 0 ) {

      const double meanBulkDensity = node->getSGMeanBulkDensity ();

      // Converts cubic-metres to barrels.
      const double CubicMetresToBarrel = 6.2898107704;

      // The volume of a mole of methane at surface conditions (mol/m^3).
      const double VolumeMoleMethaneAtSurfaceConditions = 42.306553;

      // Converts cubic-metres to cubic-feet.
      const double CubicMetresToCubicFeet = 35.314666721;

      // Converts kg to US ton.
      const double KilogrammeToUSTon = 1.0 / 907.18474;

      const double SCFGasVolumeConversionFactor = CubicMetresToCubicFeet / ( KilogrammeToUSTon * meanBulkDensity );

      // Converts km^2 to acres.
      const double AcresPerKilometreSquared = 247.10538147;
      // const double KilometreSquaredToAcre = 247.10538147;

      // (kg/m^2)->(kg/m^2)/density = m^3/m^2.
      // Conversion to ft^3 -> ft^3/m^2.
      // Conversion to billion (1.0e9) ft^3 (divide by 1.0e9)
      // Conversion to bcf/km^2 multiply by (1.0e3)^2=1.0e6.
      const double BCFGasVolumeConversionFactor = ( CubicMetresToCubicFeet * 1.0e6 / 1.0e9 ) / AcresPerKilometreSquared;

      // Convert the m^3/m^2 to billion cubic feet / km^2.
      // const double BCFGasVolumeConversionFactor = CubicMetresToCubicFeet / 1000.0;

      const double BarrelsOilVolumeConversionFactor = CubicMetresToBarrel / ( KilogrammeToUSTon * meanBulkDensity );

      // given mass/area: kg/m2-> (kg/m2)/density_oil (m3/m2)-> bbl/m3*(kg/m2)/density_oil (bbl/m2) 1e6 *(kg/m2)/density_oil/(1e3^2) (mbbl/km^2)
      //  1e6 *(kg/m2)/density_oil/(1e3^2)/km2-acre -> mbbl/acre
      // This include other implicit scaling values (that have cancelled and so do not appear).
      
      // (kg/m^2)-> (kg/m^2)/density -> (kg/m^2).(m^3/kg) -> (m^3/m^2) (conversion m^3->barrel) -> barrel/m^2. 
      // barrel/m^2 (conversion to million barrels * 1.0e-6) MMbarrel/m^2 ( conversion to million-barrel/km^2 = / 1.0e-6)
      // This include other implicit scaling values (that have cancelled and so do not appear).
      const double MBarrelsOilVolumeConversionFactor = CubicMetresToBarrel / AcresPerKilometreSquared; // * 1.0e-6 / 1.0e-6;


      HistoryItem* hist = new HistoryItem;
      Genex5::SpeciesState* c1State = simulatorState->GetSpeciesStateByName ( "C1" );
      Genex5::SpeciesState* speciesState;

      int id;

      // Standard conditions.
      double StandardTemperature = 15.5555556 + Genex5::Constants::s_TCabs; //Kelvin
      double StandardPressure    = 101325.353; //Pa


      double oilRetained;
      double gasRetained;

      double gasRetainedBcf;
      double oilRetainedMbbl;

      double gorm = Genex5::PVTCalc::getInstance ().computeGorm ( simulatorState->getVapourComponents (), simulatorState->getLiquidComponents ());

      double thickness    = node->getThickness ();

      Genex5::PVTComponentMasses phaseMasses;
      Genex5::PVTPhaseValues     densities;
      Genex5::PVTPhaseValues     viscosities;

      Genex5::PVTComponents masses;

      masses = simulatorState->getVapourComponents ();
      masses ( pvtFlash::COX ) = 0.0;
      Genex5::PVTCalc::getInstance ().compute ( StandardTemperature, StandardPressure, masses, phaseMasses, densities, viscosities, true, gorm );

      // Could optimise this a bit.
      gasRetainedBcf = phaseMasses.sum ( pvtFlash::VAPOUR_PHASE ) / densities ( pvtFlash::VAPOUR_PHASE );
      oilRetainedMbbl = phaseMasses.sum ( pvtFlash::LIQUID_PHASE ) / densities ( pvtFlash::LIQUID_PHASE );

      gasRetained = phaseMasses.sum ( pvtFlash::VAPOUR_PHASE ) / ( thickness * densities ( pvtFlash::VAPOUR_PHASE ));
      oilRetained = phaseMasses.sum ( pvtFlash::LIQUID_PHASE ) / ( thickness * densities ( pvtFlash::LIQUID_PHASE ));

      masses = simulatorState->getLiquidComponents ();
      masses ( pvtFlash::COX ) = 0.0;
      Genex5::PVTCalc::getInstance ().compute ( StandardTemperature, StandardPressure, masses, phaseMasses, densities, viscosities, true, gorm );

      gasRetainedBcf += phaseMasses.sum ( pvtFlash::VAPOUR_PHASE ) / ( densities ( pvtFlash::VAPOUR_PHASE ));
      oilRetainedMbbl += phaseMasses.sum ( pvtFlash::LIQUID_PHASE ) / densities ( pvtFlash::LIQUID_PHASE );

      gasRetained += phaseMasses.sum ( pvtFlash::VAPOUR_PHASE ) / ( thickness * densities ( pvtFlash::VAPOUR_PHASE ));
      oilRetained += phaseMasses.sum ( pvtFlash::LIQUID_PHASE ) / ( thickness * densities ( pvtFlash::LIQUID_PHASE ));

      hist->m_thickness = thickness;
      hist->m_gasRetainedSTSCF = gasRetained * SCFGasVolumeConversionFactor;
      hist->m_oilRetainedSTBarrels = oilRetained * BarrelsOilVolumeConversionFactor;

      hist->m_gasRetainedSTBCF = gasRetainedBcf * BCFGasVolumeConversionFactor;
      hist->m_oilRetainedSTMBarrels = oilRetainedMbbl * MBarrelsOilVolumeConversionFactor;
 
      hist->m_subSurfaceLiquidDensity = simulatorState->getSubSurfaceDensities ()( pvtFlash::LIQUID_PHASE );
      hist->m_subSurfaceVapourDensity = simulatorState->getSubSurfaceDensities ()( pvtFlash::VAPOUR_PHASE );

      hist->m_time = nodeInput->GetCurrentTime ();
      hist->m_temperature = nodeInput->GetTemperatureKelvin () - Genex5::Constants::s_TCabs;
      hist->m_pressure = nodeInput->getPorePressure ();
      hist->m_porosity = nodeInput->getPorosity ();
      hist->m_permeability = nodeInput->getPermeability ();
      hist->m_c1adsorped = SCFGasVolumeConversionFactor * c1State->getAdsorpedMol () / VolumeMoleMethaneAtSurfaceConditions;
      hist->m_c1desorped = SCFGasVolumeConversionFactor * c1State->getDesorpedMol () / VolumeMoleMethaneAtSurfaceConditions;
      hist->m_adsorptionCapacity = SCFGasVolumeConversionFactor * c1State->getAdsorptionCapacity ();

      hist->m_toc = simulatorState->getCurrentToc ();
      hist->m_vre = nodeInput->getVre ();
      hist->m_VLSRTemperature = simulatorState->getVLSRTemperature ();
      hist->m_VLReferenceTemperature = simulatorState->getVLReferenceTemperature ();

      if ( c1State->getAdsorptionCapacity () != 0.0 ) {
         hist->m_adsorpedFraction = ( c1State->getAdsorpedMol () / VolumeMoleMethaneAtSurfaceConditions ) / c1State->getAdsorptionCapacity ();
      } else {
         hist->m_adsorpedFraction = 0.0;
      }

      hist->m_bitumen = simulatorState->getImmobileSpecies ().getRetainedVolume ( node->getThickness ());

      hist->m_irreducibleWaterSat = simulatorState->getIrreducibleWaterSaturation ();
      hist->m_hcSaturation = simulatorState->getHcSaturation ();

      hist->m_hcLiquidSaturation = simulatorState->getRetainedLiquidVolume () / simulatorState->getUsablePorosity ();
      hist->m_hcVapourSaturation = simulatorState->getRetainedVapourVolume () / simulatorState->getUsablePorosity ();

      // Get species-expelled-from-source-rock and species-retained-in-source-rock.
      for ( id = 0; id < pvtFlash::NUM_COMPONENTS; ++id ) {
         speciesState = simulatorState->GetSpeciesStateByName ( CBMGenerics::ComponentManager::getInstance ().GetSpeciesName ( id ));

         if ( speciesState != 0 ) {
            hist->m_expelledMasses ( pvtFlash::ComponentId ( id )) = speciesState->getMassExpelledFromSourceRock ();
            hist->m_retainedMasses ( pvtFlash::ComponentId ( id )) = speciesState->getRetained ();
            hist->m_genex5ExpelledMasses ( pvtFlash::ComponentId ( id )) = speciesState->GetExpelledMass ();
         } else {

            cout << " unknown species: " << id << "  " << pvtFlash::ComponentIdNames [ id ] << endl;

            hist->m_expelledMasses ( pvtFlash::ComponentId ( id )) = 0.0;
            hist->m_retainedMasses ( pvtFlash::ComponentId ( id )) = 0.0;
         }

      }

      m_history.push_back ( hist );
   }

}

void OTGCNodeAdsorptionHistory::write ( std::ostream& str ) {

   HistoryItemList::iterator histIter;
   int id;

   //******** Start output of header ********//

   str << setw ( 21 ) << "Time-Ma"
       << setw ( 21 ) << "Thickness-m"
       << setw ( 21 ) << "Temperature-C"
       << setw ( 21 ) << "Pressure-Pa"
       << setw ( 21 ) << "Porosity-frac"
       << setw ( 21 ) << "Permeability-mD"
       << setw ( 21 ) << "C1Adsorped-SCF/ton"
       << setw ( 21 ) << "C1Desorped-SCF/ton"
       << setw ( 21 ) << "AdsorpCap-scf/ton"
       << setw ( 21 ) << "Pyrobitumen-frac"
       << setw ( 21 ) << "OilVolume-BBL/ton"
       << setw ( 21 ) << "GasVolume-SCF/ton"
       << setw ( 21 ) << "OilVolume-MMBBL/acre"
       << setw ( 21 ) << "GasVolume-BCF/acre"
       << setw ( 21 ) << "IWS"
       << setw ( 21 ) << "HcSat";

   for ( id = 0; id < pvtFlash::NUM_COMPONENTS; ++id ) {
      str << setw ( 30 ) << pvtFlash::ComponentIdNames [ id ] + "Expel-kg/m^2";
   }

   for ( id = 0; id < pvtFlash::NUM_COMPONENTS; ++id ) {
      str << setw ( 30 ) << pvtFlash::ComponentIdNames [ id ] + "Retain-kg/m^2";
   }

   for ( id = 0; id < pvtFlash::NUM_COMPONENTS; ++id ) {
      str << setw ( 30 ) << pvtFlash::ComponentIdNames [ id ] + "Gx5Expel-kg/m^2";
   }

   str << setw ( 21 ) << "FracOfAdCap";
   str << setw ( 21 ) << "HcVapourSat";
   str << setw ( 21 ) << "HcLiquidSat";
   str << setw ( 21 ) << "VapourDens_kg/m^3";
   str << setw ( 21 ) << "LiquidDens_kg/m^3";
   str << setw ( 21 ) << "TOC";
   str << setw ( 21 ) << "VLSRTemp_cc";
   str << setw ( 21 ) << "VLRefTemp_cc";
   str << setw ( 21 ) << "VRe";
   str << std::endl;

   //******** End output of header ********//

   for ( histIter = m_history.begin (); histIter != m_history.end (); ++histIter ) {
      const HistoryItem* hist = *histIter;

      str << std::setw ( 21 ) << hist->m_time
          << std::setw ( 21 ) << hist->m_thickness
          << std::setw ( 21 ) << hist->m_temperature
          << std::setw ( 21 ) << hist->m_pressure
          << std::setw ( 21 ) << hist->m_porosity
          << std::setw ( 21 ) << hist->m_permeability
          << std::setw ( 21 ) << hist->m_c1adsorped
          << std::setw ( 21 ) << hist->m_c1desorped
          << std::setw ( 21 ) << hist->m_adsorptionCapacity
          << std::setw ( 21 ) << hist->m_bitumen / hist->m_porosity
          << std::setw ( 21 ) << hist->m_oilRetainedSTBarrels
          << std::setw ( 21 ) << hist->m_gasRetainedSTSCF
          << std::setw ( 21 ) << hist->m_oilRetainedSTMBarrels
          << std::setw ( 21 ) << hist->m_gasRetainedSTBCF
          << std::setw ( 21 ) << hist->m_irreducibleWaterSat
          << std::setw ( 21 ) << hist->m_hcSaturation;

      for ( id = 0; id < pvtFlash::NUM_COMPONENTS; ++id ) {
         str << setw ( 30 ) << hist->m_expelledMasses ( pvtFlash::ComponentId ( id ));
      }

      for ( id = 0; id < pvtFlash::NUM_COMPONENTS; ++id ) {
         str << setw ( 30 ) << hist->m_retainedMasses ( pvtFlash::ComponentId ( id ));
      }

      for ( id = 0; id < pvtFlash::NUM_COMPONENTS; ++id ) {
         str << setw ( 30 ) << hist->m_genex5ExpelledMasses ( pvtFlash::ComponentId ( id ));
      }

      str << setw ( 21 ) << hist->m_adsorpedFraction;
      str << setw ( 21 ) << hist->m_hcVapourSaturation;
      str << setw ( 21 ) << hist->m_hcLiquidSaturation;

      str << setw ( 21 ) << hist->m_subSurfaceVapourDensity;
      str << setw ( 21 ) << hist->m_subSurfaceLiquidDensity;
      str << setw ( 21 ) << hist->m_toc;
      str << setw ( 21 ) << hist->m_VLSRTemperature;
      str << setw ( 21 ) << hist->m_VLReferenceTemperature;
      str << setw ( 21 ) << hist->m_vre;

      str << std::endl;
   }

}

Genex5::NodeAdsorptionHistory* allocateOTGCNodeAdsorptionHistory () {
   return new OTGCNodeAdsorptionHistory;
}
