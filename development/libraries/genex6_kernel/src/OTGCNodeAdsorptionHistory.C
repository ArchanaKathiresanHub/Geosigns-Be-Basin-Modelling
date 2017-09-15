//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 
#include "OTGCNodeAdsorptionHistory.h"

#include <iomanip>

#include "Interface/SGDensitySample.h"

#include "ConstantsGenex.h"

#include "Input.h"
#include "SimulatorState.h"
#include "SpeciesState.h"

#include "EosPack.h"

// utilitites library
#include "ConstantsMathematics.h"
#include "ConstantsNumerical.h"
#include "ConstantsPhysics.h"


// CBMGenerics library
#include "ComponentManager.h"


using Utilities::Maths::CelciusToKelvin;
using Utilities::Maths::CubicMetresToCubicFeet;
using Utilities::Maths::CubicMetresToBarrel;
using Utilities::Maths::GorConversionFactor;
using Utilities::Maths::CgrConversionFactor;
using Utilities::Maths::KilometreSquaredToAcres;
using Utilities::Maths::KilogrammeToUSTon;
using Utilities::Numerical::CauldronNoDataValue;
using Utilities::Physics::StandardPressure;
using Utilities::Physics::StandardTemperatureGenexK;
typedef CBMGenerics::ComponentManager::SpeciesNamesId ComponentId;


Genex6::OTGCNodeAdsorptionHistory::OTGCNodeAdsorptionHistory ( const SpeciesManager&                      speciesManager,
                                                               DataAccess::Interface::ProjectHandle* projectHandle ) :
   NodeAdsorptionHistory ( speciesManager, projectHandle ) {
}


Genex6::OTGCNodeAdsorptionHistory::~OTGCNodeAdsorptionHistory () {

   HistoryItemList::iterator histIter;

   for ( histIter = m_history.begin (); histIter != m_history.end (); ++histIter ) {
      delete (*histIter);
   }

   m_history.clear ();
}

Genex6::OTGCNodeAdsorptionHistory::HistoryItem::HistoryItem () {
   m_time = 0.0;
   m_thickness = 0.0;
   m_temperature = 0.0;
   m_pressure = 0.0;
   m_porosity = 0.0;
   m_effectivePorosity = 0.0;
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
   m_waterSaturation = 0.0;
   m_subSurfaceLiquidDensity = 0.0;
   m_subSurfaceVapourDensity = 0.0;
   m_irreducibleWaterSat = 0.0;
   m_hcSaturation = 0.0;
   m_retainedOilApiSr = 0.0;
   m_retainedCondensateApiSr = 0.0;
   m_retainedGorSr = 0.0;
   m_retainedCgrSr = 0.0;
   m_overChargeFactor = 0.0;

   m_hOverC = 0.0;
   m_oOverC = 0.0;

   m_h2sRisk = 0.0;
}


void Genex6::OTGCNodeAdsorptionHistory::collect ( Genex6::SourceRockNode* node ) {

   const Genex6::Input* nodeInput = node->getLastInput ();
   Genex6::SimulatorState* simulatorState = &node->getPrincipleSimulatorState ();

   if ( nodeInput != 0 and simulatorState != 0 ) {
      using namespace Constants;

      const double meanBulkDensity = getProjectHandle ()->getSGDensitySample ()->getDensity ();

      const double SCFGasVolumeConversionFactor = CubicMetresToCubicFeet / ( KilogrammeToUSTon * meanBulkDensity );

      // (kg/m^2)->(kg/m^2)/density = m^3/m^2.
      // Conversion to ft^3 -> ft^3/m^2.
      // Conversion to billion (1.0e9) ft^3 (divide by 1.0e9)
      // Conversion to bcf/km^2 multiply by (1.0e3)^2=1.0e6.
      const double BCFGasVolumeConversionFactor = ( CubicMetresToCubicFeet * 1.0e6 / 1.0e9 ) / KilometreSquaredToAcres;

      // Convert the m^3/m^2 to billion cubic feet / km^2.
      // const double BCFGasVolumeConversionFactor = CubicMetresToCubicFeet / 1000.0;

      const double BarrelsOilVolumeConversionFactor = CubicMetresToBarrel / ( KilogrammeToUSTon * meanBulkDensity );

      // given mass/area: kg/m2-> (kg/m2)/density_oil (m3/m2)-> bbl/m3*(kg/m2)/density_oil (bbl/m2) 1e6 *(kg/m2)/density_oil/(1e3^2) (mbbl/km^2)
      //  1e6 *(kg/m2)/density_oil/(1e3^2)/km2-acre -> mbbl/acre
      // This include other implicit scaling values (that have cancelled and so do not appear).
      
      // (kg/m^2)-> (kg/m^2)/density -> (kg/m^2).(m^3/kg) -> (m^3/m^2) (conversion m^3->barrel) -> barrel/m^2. 
      // barrel/m^2 (conversion to million barrels * 1.0e-6) MMbarrel/m^2 ( conversion to million-barrel/km^2 = / 1.0e-6)
      // This include other implicit scaling values (that have cancelled and so do not appear).
      const double MBarrelsOilVolumeConversionFactor = CubicMetresToBarrel / KilometreSquaredToAcres; // * 1.0e-6 / 1.0e-6;


      HistoryItem* hist = new HistoryItem;
      Genex6::SpeciesState* c1State = simulatorState->GetSpeciesStateById ( getSpeciesManager ().getC1Id ());
      Genex6::SpeciesState* speciesState;

      int id;

      double oilRetained;
      double gasRetained;

      double gasRetainedBcf;
      double oilRetainedMbbl;

      double gasVolume;
      double oilVolume;
      double gasExpansionRation;

      double gorm = Genex6::PVTCalc::getInstance ().computeGorm ( simulatorState->getVapourComponents (), simulatorState->getLiquidComponents ());

      const double thickness = simulatorState->GetThickness ();
      // double thickness    = node->getThickness ();

      Genex6::PVTComponentMasses phaseMasses;
      Genex6::PVTPhaseValues     densities;
      Genex6::PVTPhaseValues     viscosities;

      Genex6::PVTComponents masses;

      masses = simulatorState->getVapourComponents ();
      masses ( ComponentId::COX ) = 0.0;
      masses ( ComponentId::H2S ) = 0.0;

      Genex6::PVTCalc::getInstance ().compute ( StandardTemperatureGenexK, StandardPressure, masses, phaseMasses, densities, viscosities, true, gorm );

      // Could optimise this a bit.
      gasRetainedBcf = phaseMasses.sum ( PhaseId::VAPOUR ) / densities ( PhaseId::VAPOUR );
      oilRetainedMbbl = phaseMasses.sum ( PhaseId::LIQUID ) / densities ( PhaseId::LIQUID );

      gasRetained = phaseMasses.sum ( PhaseId::VAPOUR ) / ( thickness * densities ( PhaseId::VAPOUR ));
      oilRetained = phaseMasses.sum ( PhaseId::LIQUID ) / ( thickness * densities ( PhaseId::LIQUID ));

      masses = simulatorState->getLiquidComponents ();
      masses ( ComponentId::COX ) = 0.0;
      masses ( ComponentId::H2S ) = 0.0;
      Genex6::PVTCalc::getInstance ().compute ( StandardTemperatureGenexK, StandardPressure, masses, phaseMasses, densities, viscosities, true, gorm );

      gasRetainedBcf += phaseMasses.sum ( PhaseId::VAPOUR ) / ( densities ( PhaseId::VAPOUR ));
      oilRetainedMbbl += phaseMasses.sum ( PhaseId::LIQUID ) / densities ( PhaseId::LIQUID );

      gasRetained += phaseMasses.sum ( PhaseId::VAPOUR ) / ( thickness * densities ( PhaseId::VAPOUR ));
      oilRetained += phaseMasses.sum ( PhaseId::LIQUID ) / ( thickness * densities ( PhaseId::LIQUID ));

      hist->m_thickness = thickness;
      hist->m_gasRetainedSTSCF = gasRetained * SCFGasVolumeConversionFactor;
      hist->m_oilRetainedSTBarrels = oilRetained * BarrelsOilVolumeConversionFactor;

      hist->m_gasRetainedSTBCF = gasRetainedBcf * BCFGasVolumeConversionFactor;
      hist->m_oilRetainedSTMBarrels = oilRetainedMbbl * MBarrelsOilVolumeConversionFactor;
 
      hist->m_subSurfaceLiquidDensity = simulatorState->getSubSurfaceDensities ()( PhaseId::LIQUID );
      hist->m_subSurfaceVapourDensity = simulatorState->getSubSurfaceDensities ()( PhaseId::VAPOUR );

      hist->m_time = nodeInput->GetTime ();
      hist->m_temperature = nodeInput->GetTemperatureKelvin () - CelciusToKelvin;
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

      hist->m_bitumen = simulatorState->getImmobileSpecies ().getRetainedVolume ( thickness );
      hist->m_effectivePorosity = simulatorState->getEffectivePorosity ();

      hist->m_irreducibleWaterSat = simulatorState->getIrreducibleWaterSaturation ();
      hist->m_hcSaturation = simulatorState->getHcSaturation ();

      hist->m_hcLiquidSaturation = simulatorState->getRetainedLiquidVolume () / simulatorState->getEffectivePorosity ();
      hist->m_hcVapourSaturation = simulatorState->getRetainedVapourVolume () / simulatorState->getEffectivePorosity ();
      hist->m_waterSaturation = 1.0 - ( hist->m_hcVapourSaturation + hist->m_hcLiquidSaturation );

      hist->m_hOverC = simulatorState->getHC ();
      hist->m_oOverC = simulatorState->getOC ();
      hist->m_h2sRisk = simulatorState->getH2SFromGenex () + simulatorState->getH2SFromOtgc ();

      // Get species-expelled-from-source-rock and species-retained-in-source-rock.
      for ( id = 0; id < ComponentId::NUMBER_OF_SPECIES; ++id ) {

         CBMGenerics::ComponentManager::SpeciesNamesId componentManagerId = CBMGenerics::ComponentManager::SpeciesNamesId ( id );

         speciesState = simulatorState->GetSpeciesStateById ( getSpeciesManager ().mapComponentManagerSpeciesToId ( componentManagerId ));

         if ( speciesState != 0 ) {
            hist->m_expelledMasses ( ComponentId ( id )) = speciesState->getMassExpelledFromSourceRock ();
            hist->m_retainedMasses ( ComponentId ( id )) = speciesState->getRetained ();
            hist->m_genex5ExpelledMasses ( ComponentId ( id )) = speciesState->GetExpelledMass ();
         } else {
            hist->m_expelledMasses ( ComponentId ( id )) = 0.0;
            hist->m_retainedMasses ( ComponentId ( id )) = 0.0;
         }

      }

      node->computeHcVolumes ( gasVolume, oilVolume, gasExpansionRation, 
                               hist->m_retainedGorSr, hist->m_retainedCgrSr,
                               hist->m_retainedOilApiSr, hist->m_retainedCondensateApiSr );

      if ( hist->m_retainedGorSr != CauldronNoDataValue ) {
         hist->m_retainedGorSr = GorConversionFactor * hist->m_retainedGorSr;
      }

      if ( hist->m_retainedCgrSr != CauldronNoDataValue ) {
         hist->m_retainedCgrSr = CgrConversionFactor * hist->m_retainedCgrSr;
      }

      node->computeOverChargeFactor ( hist->m_overChargeFactor );


      m_history.push_back ( hist );
   }

}

void Genex6::OTGCNodeAdsorptionHistory::write ( std::ostream& str ) {

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

   for ( id = 0; id < ComponentId::NUMBER_OF_SPECIES; ++id ) {
      str << setw ( 30 ) << CBMGenerics::ComponentManager::getInstance().getSpeciesNameHistory( id ) + "Expel-kg/m^2";
   }

   for ( id = 0; id < ComponentId::NUMBER_OF_SPECIES; ++id ) {
      str << setw ( 30 ) << CBMGenerics::ComponentManager::getInstance().getSpeciesNameHistory( id ) + "Retain-kg/m^2";
   }

   for ( id = 0; id < ComponentId::NUMBER_OF_SPECIES; ++id ) {
      str << setw ( 30 ) << CBMGenerics::ComponentManager::getInstance().getSpeciesNameHistory( id ) + "Gx5Expel-kg/m^2";
   }

   str << setw ( 21 ) << "FracOfAdCap";
   str << setw ( 21 ) << "HcVapourSat";
   str << setw ( 21 ) << "HcLiquidSat";
   str << setw ( 21 ) << "VapourDens-kg/m^3";
   str << setw ( 21 ) << "LiquidDens-kg/m^3";
   str << setw ( 21 ) << "TOC";
   str << setw ( 21 ) << "VLSRTemp-cc";
   str << setw ( 21 ) << "VLRefTemp-cc";
   str << setw ( 21 ) << "VRe";

   str << setw ( 25 ) << "RetainedOilApiSR-degApi";
   str << setw ( 31 ) << "RetainedCondensateApiSR-degApi";
   str << setw ( 24 ) << "RetainedGOR_SR-scf/bbl";
   str << setw ( 24 ) << "RetainedCGR_SR-bbl/mcf";
   str << setw ( 24 ) << "OverChargeFactor-frac";

   str << setw ( 24 ) << "WaterSaturation-frac";
   str << setw ( 24 ) << "EffectivePorosity-frac";

   str << setw ( 24 ) << "H/C";
   str << setw ( 24 ) << "O/C";
   str << setw ( 24 ) << "H2SRisk-kg/m^2";


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

      for ( id = 0; id < ComponentId::NUMBER_OF_SPECIES; ++id ) {
         str << setw ( 30 ) << hist->m_expelledMasses.m_components [ id ];
      }

      for ( id = 0; id < ComponentId::NUMBER_OF_SPECIES; ++id ) {
         str << setw ( 30 ) << hist->m_retainedMasses.m_components [ id ];
      }

      for ( id = 0; id < ComponentId::NUMBER_OF_SPECIES; ++id ) {
         str << setw ( 30 ) << hist->m_genex5ExpelledMasses.m_components [ id ];
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

      str << setw ( 25 ) << hist->m_retainedOilApiSr;
      str << setw ( 31 ) << hist->m_retainedCondensateApiSr;
      str << setw ( 24 ) << hist->m_retainedGorSr;
      str << setw ( 24 ) << hist->m_retainedCgrSr;
      str << setw ( 24 ) << hist->m_overChargeFactor;

      str << setw ( 24 ) << hist->m_waterSaturation;
      str << setw ( 24 ) << hist->m_effectivePorosity;

      str << setw ( 24 ) << hist->m_hOverC;
      str << setw ( 24 ) << hist->m_oOverC;
      str << setw ( 24 ) << hist->m_h2sRisk;

      str << std::endl;
   }

}

Genex6::NodeAdsorptionHistory* Genex6::allocateOTGCNodeAdsorptionHistory ( const SpeciesManager&                      speciesManager,
                                                                           DataAccess::Interface::ProjectHandle* projectHandle ) {
   return new OTGCNodeAdsorptionHistory ( speciesManager, projectHandle );
}
