#include "GenexHistory.h"

#include <iomanip>

#include "PhysicalConstants.h"
#include "Constants.h"
#include "ComponentManager.h"

#include "Input.h"
#include "SimulatorState.h"
#include "SpeciesState.h"

#include "EosPack.h"

Genex6::GenexHistory::GenexHistory ( const SpeciesManager&                      speciesManager,
                                     DataAccess::Interface::ProjectHandle* projectHandle ) :
   NodeAdsorptionHistory ( speciesManager, projectHandle ) {

   mapComponentManagerSpeciesIdToOutputOrder();
}


Genex6::GenexHistory::~GenexHistory () {

   HistoryItemList::iterator histIter;

   for ( histIter = m_history.begin (); histIter != m_history.end (); ++histIter ) {
      delete (*histIter);
   }

   m_history.clear ();
}

Genex6::GenexHistory::HistoryItem::HistoryItem () {
   int i;

   m_time = 0;
   m_temperature = 0;
   m_vre = 0;
   m_pressure = 0;
   m_ves = 0;
   m_toc = 0;

   m_kerogenConversionRatio = 0;
   
   m_InstExpApi = 0;
   m_CumExpApi = 0;
   
   m_InstExpGOR = 0;
   m_CumExpGOR = 0;
   
   m_InstExpCGR = 0;
   m_CumExpCGR = 0;
   
   m_InstExpGasWetness = 0;
   m_CumExpGasWetness = 0;
   
   m_InstExpArom = 0;
   m_CumExpArom = 0;

   m_HC = 0;
   m_OC = 0;
   
   for(i = 0; i <  CBMGenerics::ComponentManager::NumberOfOutputSpecies; ++ i ) {
      m_speciesGeneratedRate[i] = 0;
      m_speciesGeneratedCum[i] = 0;
      m_speciesExpelledRate[i] = 0;
      m_speciesExpelledCum[i] = 0;
   }

   for(i = 0; i < NumberOfGroups; ++ i ) {
      m_fractionGeneratedRate[i] = 0;
      m_fractionGeneratedCum[i] = 0;
      m_fractionExpelledRate[i] = 0;
      m_fractionExpelledCum[i] = 0;
   }
}

void Genex6::GenexHistory::mapComponentManagerSpeciesIdToOutputOrder () {
   int i;

   for( i = 0; i < CBMGenerics::ComponentManager::NumberOfOutputSpecies; ++ i ) {
      SpeciesOutputOrder[SpeciesOutputId[i]] = i;
   }
}

void Genex6::GenexHistory::collect ( Genex6::SourceRockNode* node ) {

   const Genex6::Input* nodeInput = node->getLastInput ();
   Genex6::SimulatorState * simulatorState = & node->getPrincipleSimulatorState ();
   
   HistoryItem* hist = new HistoryItem;
   Genex6::SpeciesResult* speciesResult;

   int speciesGenexId;
   if ( nodeInput != 0 and simulatorState != 0 ) {

      hist->m_time = nodeInput->GetTime ();
      hist->m_temperature = nodeInput->GetTemperatureKelvin () - Genex6::Constants::s_TCabs;
      hist->m_vre = nodeInput->getVre ();
      hist->m_pressure = nodeInput->getPorePressure ();
      hist->m_ves = nodeInput->GetPressure ();
      hist->m_toc = simulatorState->getCurrentToc ();
 
      for ( int id = 0; id < pvtFlash::NUM_COMPONENTS; ++id ) {

         CBMGenerics::ComponentManager::SpeciesNamesId componentManagerId = CBMGenerics::ComponentManager::SpeciesNamesId ( id );

         speciesGenexId = getSpeciesManager ().mapComponentManagerSpeciesToId ( componentManagerId );
         if( speciesGenexId < 0 ) {
            hist->m_speciesGeneratedRate[ SpeciesOutputOrder[componentManagerId] ] = 0.0;
            hist->m_speciesGeneratedCum[ SpeciesOutputOrder[componentManagerId] ] = 0.0;
            hist->m_speciesExpelledRate[ SpeciesOutputOrder[componentManagerId] ] = 0.0;
            hist->m_speciesExpelledCum[ SpeciesOutputOrder[componentManagerId] ] = 0.0;
         } else {
            speciesResult = &simulatorState->GetSpeciesResult(speciesGenexId);
          // speciesResult = &simulatorState->GetSpeciesResult( getSpeciesManager ().mapComponentManagerSpeciesToId ( componentManagerId ));

            // double value = speciesResult->GetConcentration() *  simulatorState->GetThickness() * simulatorState->GetConcki() + speciesResult->GetExpelledMass();
            
            hist->m_speciesGeneratedRate[ SpeciesOutputOrder[componentManagerId] ] = speciesResult->GetGeneratedRate();
            hist->m_speciesGeneratedCum[ SpeciesOutputOrder[componentManagerId] ] = speciesResult->GetGeneratedCum(); //value;
            hist->m_speciesExpelledRate[ SpeciesOutputOrder[componentManagerId] ] = speciesResult->GetFlux();
            hist->m_speciesExpelledCum[ SpeciesOutputOrder[componentManagerId] ]  = speciesResult->GetExpelledMass();
         }
      }         
      
      hist->m_fractionGeneratedRate[Oil] = simulatorState->GetGroupResult(GenexResultManager::OilGeneratedRate);
      hist->m_fractionGeneratedRate[HcGas] = simulatorState->GetGroupResult(GenexResultManager::HcGasGeneratedRate);
      hist->m_fractionGeneratedRate[DryGas] = simulatorState->GetGroupResult(GenexResultManager::DryGasGeneratedRate);
      hist->m_fractionGeneratedRate[WetGas] = simulatorState->GetGroupResult(GenexResultManager::WetGasGeneratedRate);
      hist->m_fractionGeneratedRate[SbearingHCs] = simulatorState->GetGroupResult(GenexResultManager::SbearingHCsGeneratedRate);
      
      hist->m_fractionGeneratedCum[Oil] = simulatorState->GetGroupResult(GenexResultManager::OilGeneratedCum);
      hist->m_fractionGeneratedCum[HcGas] = simulatorState->GetGroupResult(GenexResultManager::HcGasGeneratedCum);
      hist->m_fractionGeneratedCum[DryGas] = simulatorState->GetGroupResult(GenexResultManager::DryGasGeneratedCum);
      hist->m_fractionGeneratedCum[WetGas] = simulatorState->GetGroupResult(GenexResultManager::WetGasGeneratedCum);
      hist->m_fractionGeneratedCum[SbearingHCs] = simulatorState->GetGroupResult(GenexResultManager::SbearingHCsGeneratedCum);

      hist->m_fractionExpelledRate[Oil] = simulatorState->GetGroupResult(GenexResultManager::OilExpelledRate);
      hist->m_fractionExpelledRate[HcGas] = simulatorState->GetGroupResult(GenexResultManager::HcGasExpelledRate);
      hist->m_fractionExpelledRate[DryGas] = simulatorState->GetGroupResult(GenexResultManager::DryGasExpelledRate);
      hist->m_fractionExpelledRate[WetGas] = simulatorState->GetGroupResult(GenexResultManager::WetGasExpelledRate);
      hist->m_fractionExpelledRate[SbearingHCs] = simulatorState->GetGroupResult(GenexResultManager::SbearingHCsExpelledRate);

      hist->m_fractionExpelledCum[Oil] = simulatorState->GetGroupResult(GenexResultManager::OilExpelledCum);
      hist->m_fractionExpelledCum[HcGas] = simulatorState->GetGroupResult(GenexResultManager::HcGasExpelledCum);
      hist->m_fractionExpelledCum[DryGas] = simulatorState->GetGroupResult(GenexResultManager::DryGasExpelledCum);
      hist->m_fractionExpelledCum[WetGas] = simulatorState->GetGroupResult(GenexResultManager::WetGasExpelledCum);
      hist->m_fractionExpelledCum[SbearingHCs] = simulatorState->GetGroupResult(GenexResultManager::SbearingHCsExpelledCum);

      hist->m_kerogenConversionRatio = simulatorState->GetResult(GenexResultManager::KerogenConversionRatio);

      hist->m_InstExpApi = simulatorState->GetResult(GenexResultManager::ExpulsionApiInst);
      hist->m_CumExpApi = simulatorState->GetResult(GenexResultManager::ExpulsionApiCum);

      hist->m_InstExpGOR = simulatorState->GetResult(GenexResultManager::ExpulsionGasOilRatioInst);
      hist->m_CumExpGOR = simulatorState->GetResult(GenexResultManager::ExpulsionGasOilRatioCum);

      hist->m_InstExpCGR = simulatorState->GetResult(GenexResultManager::ExpulsionCondensateGasRatioInst);
      hist->m_CumExpCGR = simulatorState->GetResult(GenexResultManager::ExpulsionCondensateGasRatioCum);

      hist->m_InstExpGasWetness = simulatorState->GetResult(GenexResultManager::ExpulsionGasWetnessInst);
      hist->m_CumExpGasWetness = simulatorState->GetResult(GenexResultManager::ExpulsionGasWetnessCum);

      hist->m_InstExpArom = simulatorState->GetResult(GenexResultManager::ExpulsionAromaticityInst);
      hist->m_CumExpArom = simulatorState->GetResult(GenexResultManager::ExpulsionAromaticityCum);

      hist->m_HC = simulatorState->getHC();
      hist->m_OC = simulatorState->getOC();

      m_history.push_back ( hist );
   }

}

void Genex6::GenexHistory::writeComponentsNames ( std::ostream& str ) {

   CBMGenerics::ComponentManager::SpeciesNamesId componentManagerId;

   for ( int id = 0; id < pvtFlash::NUM_COMPONENTS; ++id ) {
      componentManagerId = CBMGenerics::ComponentManager::SpeciesNamesId ( id );  
      str << setw ( 30 ) << pvtFlash::ComponentIdNames [  Genex6::SpeciesOutputId[componentManagerId] ];
   }
}

void Genex6::GenexHistory::writeGroupsNames ( std::ostream& str ) {
   
   str << setw ( 30 ) << Genex6::SpeciesGroupsNames[Oil];
   str << setw ( 30 ) << Genex6::SpeciesGroupsNames[HcGas];
   str << setw ( 30 ) << Genex6::SpeciesGroupsNames[DryGas];
   str << setw ( 30 ) << Genex6::SpeciesGroupsNames[WetGas];
   str << setw ( 30 ) << Genex6::SpeciesGroupsNames[SbearingHCs];
}
void Genex6::GenexHistory::writeHeaderName( std::ostream& str, int strSize, const string & strName ) {

   int id;
   for ( id = 0; id < strSize / 2 - 1; ++id ) {
      str << setw ( 30 ) << " ";
   }
   str << setw ( 30 ) << strName;

   for ( id = strSize / 2; id < strSize; ++id ) {
      str << setw ( 30 ) << " ";
   }
}
void Genex6::GenexHistory::write ( std::ostream& str ) {

   HistoryItemList::iterator histIter;
   int id;

   //******** Start output of first header row ********//
   str << setw ( 21 ) << " "
       << setw ( 21 ) << " "
       << setw ( 21 ) << " "
       << setw ( 21 ) << " "
       << setw ( 21 ) << " ";

   writeHeaderName( str, pvtFlash::NUM_COMPONENTS, "Species Generated Rate" );
   writeHeaderName( str, NumberOfGroups, "Fraction Generated Rate" );

   writeHeaderName( str, pvtFlash::NUM_COMPONENTS, "Species Generated Cumulative" );
   writeHeaderName( str, NumberOfGroups, "Fraction Generated Cumulative" );

   writeHeaderName( str, pvtFlash::NUM_COMPONENTS, "Species Expelled Rate" );
   writeHeaderName( str, NumberOfGroups, "Fraction Expelled Rate" );

   writeHeaderName( str, pvtFlash::NUM_COMPONENTS, "Species Expelled Cumulative" );
   writeHeaderName( str, NumberOfGroups, "Fraction Expelled Cumulative" );

   str << setw ( 30 ) << "Kerogen Conversion Ratio";

   str << setw ( 21 ) << "InstExpApi";
   str << setw ( 21 ) << "CumExpApi";

   str << setw ( 21 ) << "InstExpGOR";
   str << setw ( 21 ) << "CumExpGOR";

   str << setw ( 21 ) << "InstExpCGR";
   str << setw ( 21 ) << "CumExpCGR";

   str << setw ( 21 ) << "InstExpGasWetness";
   str << setw ( 21 ) << "CumExpGasWetness";
 
   str << setw ( 21 ) << "InstExpArom";
   str << setw ( 21 ) << "CumExpArom";

   str << setw ( 21 ) << "TOC";
   str << setw ( 21 ) << "OoCtot";
   str << setw ( 21 ) << "HoCtot";

   str << std::endl;

   //******** Start output of second header row ********//

   str << setw ( 21 ) << "Time"
       << setw ( 21 ) << "Temperature"
       << setw ( 21 ) << "VRE"
       << setw ( 21 ) << "Pressure"
       << setw ( 21 ) << "VES";

   //------- Generated Rate -------//
   writeComponentsNames( str );
   writeGroupsNames( str );

   //------- Generated Cumulative -------//
   writeComponentsNames( str );
   writeGroupsNames( str );

   //------- Expelled Rate -------//
   writeComponentsNames( str );
   writeGroupsNames( str );

   //------- Expelled Cumulative -------//
   writeComponentsNames( str );
   writeGroupsNames( str );

   //---------------------------------//
   str << setw ( 30 ) << " ";

   str << setw ( 21 ) << " ";
   str << setw ( 21 ) << " ";

   str << setw ( 21 ) << " ";
   str << setw ( 21 ) << " ";

   str << setw ( 21 ) << " ";
   str << setw ( 21 ) << " ";

   str << setw ( 21 ) << " ";
   str << setw ( 21 ) << " ";
 
   str << setw ( 21 ) << " ";
   str << setw ( 21 ) << " ";

   str << setw ( 21 ) << " ";
   str << setw ( 21 ) << " ";
   str << setw ( 21 ) << " ";

   str << std::endl;

   //******** Start output of units header row ********//

   str << setw ( 21 ) << "Ma"
       << setw ( 21 ) << "C"
       << setw ( 21 ) << "-"
       << setw ( 21 ) << "MPa"
       << setw ( 21 ) << "MPa";

   //------- Generated Rate -------//
   for ( id = 0; id < pvtFlash::NUM_COMPONENTS + NumberOfGroups; ++id ) {
      str << setw ( 30 ) << "kg/m2/Ma";
   }

   //------- Generated Cumulative -------//
   for ( id = 0; id < pvtFlash::NUM_COMPONENTS + NumberOfGroups; ++id ) {
      str << setw ( 30 ) << "kg/m2";
   }

   //------- Expelled Rate -------//
   for ( id = 0; id < pvtFlash::NUM_COMPONENTS + NumberOfGroups; ++id ) {
      str << setw ( 30 ) << "kg/m2/Ma";
   }

   //------- Expelled Cumulative -------//
   for ( id = 0; id < pvtFlash::NUM_COMPONENTS + NumberOfGroups; ++id ) {
      str << setw ( 30 ) << "kg/m2";
   }

   //---------------------------------//
   str << setw ( 30 ) << "kg/kg";

   str << setw ( 21 ) << "degreesAPI";
   str << setw ( 21 ) << "degreesAPI";

   str << setw ( 21 ) << "m3/m3";
   str << setw ( 21 ) << "m3/m3";

   str << setw ( 21 ) << "m3/m3";
   str << setw ( 21 ) << "m3/m3";

   str << setw ( 21 ) << "m3/m3";
   str << setw ( 21 ) << "m3/m3";
 
   str << setw ( 21 ) << "m3/m3";
   str << setw ( 21 ) << "m3/m3";

   str << setw ( 21 ) << "%";
   str << setw ( 21 ) << "-";
   str << setw ( 21 ) << "-";

   str << std::endl;

   //******** End output of header ********//

   for ( histIter = m_history.begin (); histIter != m_history.end (); ++histIter ) {
      const HistoryItem* hist = *histIter;

      str << std::setw ( 21 ) << - hist->m_time
          << std::setw ( 21 ) << hist->m_temperature
          << std::setw ( 21 ) << hist->m_vre
          << std::setw ( 21 ) << hist->m_pressure
          << std::setw ( 21 ) << hist->m_ves;
 
      for ( id = 0; id < pvtFlash::NUM_COMPONENTS; ++id ) {
         str << setw ( 30 ) << hist->m_speciesGeneratedRate [ id ];
      }
      for ( id = 0; id < NumberOfGroups; ++id ) {
         str << setw ( 30 ) << hist->m_fractionGeneratedRate [ id ];
      }

      for ( id = 0; id < pvtFlash::NUM_COMPONENTS; ++id ) {
         str << setw ( 30 ) << hist->m_speciesGeneratedCum [ id ];
      }
      for ( id = 0; id < NumberOfGroups; ++id ) {
         str << setw ( 30 ) << hist->m_fractionGeneratedCum [ id ];
      }

      for ( id = 0; id < pvtFlash::NUM_COMPONENTS; ++id ) {
         str << setw ( 30 ) << hist->m_speciesExpelledRate [ id ];
      }
      for ( id = 0; id < NumberOfGroups; ++id ) {
         str << setw ( 30 ) << hist->m_fractionExpelledRate [ id ];
      }

      for ( id = 0; id < pvtFlash::NUM_COMPONENTS; ++id ) {
         str << setw ( 30 ) << hist->m_speciesExpelledCum [ id ];
      }
      for ( id = 0; id < NumberOfGroups; ++id ) {
         str << setw ( 30 ) << hist->m_fractionExpelledCum [ id ];
      }

      str << setw ( 30 ) << hist->m_kerogenConversionRatio;

      str << setw ( 21 ) << hist->m_InstExpApi;
      str << setw ( 21 ) << hist->m_CumExpApi;

      str << setw ( 21 ) << hist->m_InstExpGOR;
      str << setw ( 21 ) << hist->m_CumExpGOR;

      str << setw ( 21 ) << hist->m_InstExpCGR;
      str << setw ( 21 ) << hist->m_CumExpCGR;

      str << setw ( 21 ) << hist->m_InstExpGasWetness;
      str << setw ( 21 ) << hist->m_CumExpGasWetness;
 
      str << setw ( 21 ) << hist->m_InstExpArom;
      str << setw ( 21 ) << hist->m_CumExpArom;

      str << setw ( 21 ) << hist->m_toc;
      str << setw ( 21 ) << hist->m_OC;
      str << setw ( 21 ) << hist->m_HC;
        

      str << std::endl;
   }

}

Genex6::NodeAdsorptionHistory* Genex6::allocateGenexHistory ( const SpeciesManager&                      speciesManager,
                                                                           DataAccess::Interface::ProjectHandle* projectHandle ) {
   return new GenexHistory ( speciesManager, projectHandle );
}
