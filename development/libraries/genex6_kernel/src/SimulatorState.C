#include "SimulatorState.h"
#include "SpeciesManager.h"
#include "SpeciesProperties.h"
#include "ChemicalModel.h"
#include "GeneralParametersHandler.h"
#include <map>
#include <math.h>
#include <string.h>


using namespace CBMGenerics;
namespace Genex6
{
SimulatorState::SimulatorState(const SpeciesManager * inSpeciesManager, const int in_numberOfSpecies, const double in_referenceTime) : 
   SimulatorStateBase(in_referenceTime)
{

   int i;

   for(i = Genex6::Constants::TOTAL_OM; i < Genex6::Constants::NUMBER_OF_ORGANIC_MATTER_TYPES; ++i) {
      m_lumpedOMConcentration[i] = 0.0;
   }

   for(i = 0; i < CBMGenerics::GenexResultManager::NumberOfResults; ++ i) {
      m_shaleGasResultsByResultId[i] = 0.0;
   }

   SetCumQuantity(OilExpelledMassCum);
   SetCumQuantity(OilExpelledVolumeCum);
   SetCumQuantity(HcGasExpelledVolumeCum);
   SetCumQuantity(C614SatPlusAromExpVolCum);
   SetCumQuantity(AromaticsExpelledVolumeCum);
   SetCumQuantity(SaturatesExpelledVolumeCum);
   SetCumQuantity(WetGasExpelledVolumeCum);
   resetIntervalCumulative ();
   
   m_numberOfSpecies = in_numberOfSpecies;
   m_SpeciesResults = new SpeciesResult[in_numberOfSpecies];

   m_TotalRetainedOM = m_MobilOMConc = 0.0;

   m_thickness = m_concki = 0.0;

   m_VLSRTemperature = 0.0;
   m_VLReferenceTemperature = 0.0;

   m_liquidComponents.zero ();
   m_vapourComponents.zero ();

   m_irreducibleWaterSaturation = 1.0;
   m_hcSaturation = 0.0;

   m_retainedVapourVolume = 0.0;
   m_retainedLiquidVolume = 0.0;
   m_effectivePorosity = 0.0;
   m_totalGasFromOtgc = 0.0;

   m_AtomHR = m_AtomCR = m_AtomOR = 0.0;
   m_HC = m_OC = 0.0;

   subSurfaceDensities.zero ();


   m_h2sFromGenex = 0.0;
   m_h2sFromOtgc = 0.0;

   m_speciesManager = inSpeciesManager;
}

SimulatorState::SimulatorState(const SimulatorState& in_State) : SimulatorStateBase()
{
   m_referenceTime = in_State.m_referenceTime;
   m_numberOfSpecies = in_State.m_numberOfSpecies;

   memcpy(m_lumpedOMConcentration, in_State.m_lumpedOMConcentration, 
          Genex6::Constants::NUMBER_OF_ORGANIC_MATTER_TYPES * sizeof(double));

   m_SpeciesResults = new SpeciesResult[m_numberOfSpecies];
   for(int i = 0; i <  m_numberOfSpecies; ++ i) {
      m_SpeciesResults[i] = in_State.m_SpeciesResults[i];
   }
   memcpy(m_ResultsByResultId, in_State.m_ResultsByResultId, 
          CBMGenerics::GenexResultManager::NumberOfResults * sizeof(double));
   
   memcpy(m_shaleGasResultsByResultId, in_State.m_shaleGasResultsByResultId, 
          CBMGenerics::GenexResultManager::NumberOfResults * sizeof(double));
   
   memcpy(m_CumQuantitiesById, in_State.m_CumQuantitiesById, 
           SimulatorState::NumberOfResults * sizeof(double));

   m_InitialKerogenConcentration = in_State.m_InitialKerogenConcentration;
   m_TotalRetainedOM = in_State.m_TotalRetainedOM;
   m_MobilOMConc = in_State.m_MobilOMConc;

   m_thickness = in_State.m_thickness;
   m_concki = in_State.m_concki;

   m_h2sFromGenex = in_State.m_h2sFromGenex;
   m_h2sFromOtgc = in_State.m_h2sFromOtgc;

   m_speciesManager = in_State.m_speciesManager;
   m_immobileSpecies = in_State.m_immobileSpecies;
}

SimulatorState::~SimulatorState()
{
   if(m_SpeciesResults != 0) { 
      delete [] m_SpeciesResults; 
      m_SpeciesResults = 0;
   }
   m_numberOfSpecies = 0;
}
void SimulatorState::SetSpeciesTimeStepVariablesToZero()
{
   s_ExmTot                      = 0.0;
   s_OilExpelledMassInst         = 0.0;
   s_OilExpelledVolumeInst       = 0.0;
   s_HcGasExpelledVolumeInst     = 0.0;
   s_WetGasExpelledVolumeInst    = 0.0;
   s_C614SatPlusAromExpVolInst   = 0.0;
   s_AromaticsExpelledVolumeInst = 0.0;
   s_SaturatesExpelledVolumeInst = 0.0;

   using namespace CBMGenerics;
   int i, j;

   for(i = FIRST_RESULT_ID, j = 0; i < LAST_RESULT_ID; ++i, ++j) {
      s_GroupResults[j] = 0.0;
   }

   m_AtomHR = m_AtomCR = m_AtomOR = 0.0;
   m_HC = m_OC = 0.0;


}
double SimulatorState::GetExmTot()
{
   return s_ExmTot;
}
double SimulatorState::GetOilExpelledMassInst()
{
   return s_OilExpelledMassInst;
}
double SimulatorState::GetOilExpelledVolumeInst()
{
   return s_OilExpelledVolumeInst;
}
double SimulatorState::GetHcGasExpelledVolumeInst()
{
   return s_HcGasExpelledVolumeInst;
}
double SimulatorState::GetWetGasExpelledVolumeInst()
{
   return s_WetGasExpelledVolumeInst;
}
double SimulatorState::GetC614SatPlusAromExpVolInst()
{
   return s_C614SatPlusAromExpVolInst;
}
double SimulatorState::GetAromaticsExpelledVolumeInst()
{
   return s_AromaticsExpelledVolumeInst;
}
double SimulatorState::GetSaturatesExpelledVolumeInst()
{
   return s_SaturatesExpelledVolumeInst;
}
void SimulatorState::SetLumpedConcentrationsToZero()
{
   for(int i = Genex6::Constants::TOTAL_OM; i < Genex6::Constants::NUMBER_OF_ORGANIC_MATTER_TYPES; ++i) {
      m_lumpedOMConcentration[i] = 0.0;
   }
}
void SimulatorState::SetResultsToZero()
{

   int i;
   for(i = 0; i <  m_numberOfSpecies; ++ i) {
      m_SpeciesResults[i].clean();
   }
   for(i = 0; i < CBMGenerics::GenexResultManager::NumberOfResults; ++ i) {
      m_ResultsByResultId[i] = Genex6::Constants::s_undefinedValue;
   }
   m_TotalRetainedOM = m_MobilOMConc = 0.0;
}

double SimulatorState::ComputeWaso()
{
   return (m_lumpedOMConcentration[Genex6::Constants::AROMATIC_IMMOBILE_OM] / 
           m_lumpedOMConcentration[Genex6::Constants::TOTAL_OM]);
}
double SimulatorState::ComputeWbo()
{
   return (m_lumpedOMConcentration[Genex6::Constants::MOBILE_OM] /
           m_lumpedOMConcentration[Genex6::Constants::TOTAL_OM]);
}

void SimulatorState::PrintBenchmarkModelFluxData(ofstream &outputfile) const
{
   outputfile << 0.0 << "," << m_referenceTime << ",";

   for(int id = 0; id < m_numberOfSpecies; ++id) {
      outputfile << m_SpeciesResults[id].GetFlux() << ",";
   }

   using namespace CBMGenerics;

   double ValueExpulsionApiInst = m_ResultsByResultId[CBMGenerics::GenexResultManager::ExpulsionApiInst];
   double ValueExpulsionGasOilRatioInst = m_ResultsByResultId[GenexResultManager::ExpulsionGasOilRatioInst];

   outputfile << ValueExpulsionApiInst << "," << ValueExpulsionGasOilRatioInst << ",";
   outputfile << endl;
}

void SimulatorState::PrintBenchmarkModelConcData(ofstream &outputfile)  const
{
   outputfile << 0.0 << "," << m_referenceTime << ",";             

   for(int id = 0; id < m_numberOfSpecies; ++id) {
      outputfile << scientific << m_SpeciesResults[id].GetConcentration() << ",";
   }
   outputfile << 0.0 << "," << 0.0 << ",";    
   outputfile << endl;

}

void SimulatorState::PrintBenchmarkModelCumExpData(ofstream &outputfile) const
{
   outputfile << 0.0 << "," << m_referenceTime << ",";

   for(int id = 0; id < m_numberOfSpecies; ++ id) {
      outputfile << m_SpeciesResults[id].GetExpelledMass() << ",";
   }

   using namespace CBMGenerics;
   double ValueExpulsionApiCum = m_ResultsByResultId[GenexResultManager::ExpulsionApiCum];
   double ValueExpulsionGasOilRatioCum = m_ResultsByResultId[GenexResultManager::ExpulsionGasOilRatioCum];

   outputfile << 0.0 << "," << ValueExpulsionApiCum << "," << ValueExpulsionGasOilRatioCum << ",";
   outputfile << endl;
}

void SimulatorState::PrintSensitivityResultsOnFile(ofstream &outputfile) const
{
   using namespace CBMGenerics;
   double ValueOilExpelledCum = m_ResultsByResultId[GenexResultManager::OilExpelledCum];
   outputfile << m_referenceTime << "," << GetTotalMass() << "," << ValueOilExpelledCum << "," << m_MobilOMConc << endl;
}

void SimulatorState::mixResults ( SimulatorState * inSimulatorState1, SimulatorState * inSimulatorState2,
                                  const double fraction1, const double fraction2 )
{
   double concentration1, generatedRate1, generatedCum1, expelledMassRes1, expelledMass1, flux1, massTrans1;
   double concentration2, generatedRate2, generatedCum2, expelledMassRes2, expelledMass2, flux2, massTrans2;

   double generatedMass1;
   double generatedMass2;
   double generatedMassRes1;
   double generatedMassRes2;

   double adsorpedMol1, desorpedMol1, freeMol1, expelledMol1, retained1, adsorptionCapacity1, expelledMassSR1;
   double adsorpedMol2, desorpedMol2, freeMol2, expelledMol2, retained2, adsorptionCapacity2, expelledMassSR2;

   double expelledMassTransientSR1;
   double expelledMassTransientSR2;

   int i, id1, id2;
   const Species *curSpecies1, *curSpecies2;
   SpeciesState *curState1, *curState2, *curState;

   for( i = 1; i <= m_numberOfSpecies; ++ i ) {
      const string & speciesName = m_speciesManager->getSpeciesName( i );
      
      concentration1    = 0.0;
      generatedRate1    = 0.0;
      generatedCum1     = 0.0;
      expelledMass1     = 0.0;
      expelledMassRes1  = 0.0;
      generatedMass1    = 0.0;
      generatedMassRes1 = 0.0;
      flux1             = 0.0;
      massTrans1        = 0.0;
      adsorpedMol1      = 0.0;
      desorpedMol1      = 0.0;
      freeMol1          = 0.0;
      expelledMol1      = 0.0;
      retained1         = 0.0;
      adsorptionCapacity1 = 0.0;
      expelledMassSR1 = 0.0;
      expelledMassTransientSR1 = 0.0;

      concentration2    = 0.0;
      generatedRate2    = 0.0;
      generatedCum2     = 0.0;
      expelledMass2     = 0.0;
      expelledMassRes2  = 0.0;
      generatedMass2    = 0.0;
      generatedMassRes2 = 0.0;
      flux2             = 0.0;
      massTrans2        = 0.0;
      adsorpedMol2      = 0.0;
      desorpedMol2      = 0.0;
      freeMol2          = 0.0;
      expelledMol2      = 0.0;
      retained2         = 0.0;
      adsorptionCapacity2 = 0.0;
      expelledMassSR2 = 0.0;
      expelledMassTransientSR2 = 0.0;

      curSpecies1    = 0;
      curSpecies2    = 0;

      id1 = inSimulatorState1->getSpeciesManager()->getSpeciesIdByName ( speciesName );
      if( id1 >= 0 ) {
         if( (curState1 = inSimulatorState1->GetSpeciesStateById (id1)) != 0 ) {
            curSpecies1 = curState1->getSpecies();
         } 
         if( fraction1 > 0 ) {
            Genex6::SpeciesResult &theResult = inSimulatorState1->GetSpeciesResult(id1);
            
            concentration1    = fraction1 * theResult.GetConcentration();
            generatedRate1    = fraction1 * theResult.GetGeneratedRate();
            generatedCum1     = fraction1 * theResult.GetGeneratedCum();
            expelledMassRes1  = fraction1 * theResult.GetExpelledMass();
            generatedMassRes1 = fraction1 * theResult.getGeneratedMass();

            flux1             = fraction1 * theResult.GetFlux();
            
            if ( curState1 != 0 ) {
               expelledMass1  = fraction1 * curState1->GetExpelledMass();
               generatedMass1 = fraction1 * curState1->getGeneratedMass ();
               massTrans1     = fraction1 * curState1->getExpelledMassTransient();
               adsorpedMol1   = fraction1 * curState1->getAdsorpedMol();
               desorpedMol1   = fraction1 * curState1->getDesorpedMol();
               freeMol1       = fraction1 * curState1->getFreeMol();
               expelledMol1   = fraction1 * curState1->getExpelledMol();
               retained1      = fraction1 * curState1->getRetained();
               adsorptionCapacity1 = fraction1 * curState1->getAdsorptionCapacity();
               expelledMassSR1     = fraction1 * curState1->getMassExpelledFromSourceRock();
               expelledMassTransientSR1 = fraction1 * curState1->getMassExpelledTransientFromSourceRock();
            } 
         } 
      } 
      id2 = inSimulatorState2->getSpeciesManager()->getSpeciesIdByName ( speciesName );
      if( id2 >= 0 ) {
         if (( curState2 = inSimulatorState2->GetSpeciesStateById (id2)) != 0 ) {
            curSpecies2 = curState2->getSpecies();
         } 
         if( fraction2 > 0 ) {
            Genex6::SpeciesResult &theResult = inSimulatorState2->GetSpeciesResult(id2);
            
            concentration2    = fraction2 * theResult.GetConcentration();
            generatedRate2    = fraction2 * theResult.GetGeneratedRate();
            generatedCum2     = fraction2 * theResult.GetGeneratedCum();
            expelledMassRes2  = fraction2 * theResult.GetExpelledMass();
            generatedMassRes2 = fraction2 * theResult.getGeneratedMass();
            flux2             = fraction2 * theResult.GetFlux();

            if ( curState2 != 0 ) {
               expelledMass2  = fraction2 * curState2->GetExpelledMass();
               generatedMass2 = fraction2 * curState2->getGeneratedMass ();
               massTrans2     = fraction2 * curState2->getExpelledMassTransient();
               adsorpedMol2   = fraction2 * curState2->getAdsorpedMol();
               desorpedMol2   = fraction2 * curState2->getDesorpedMol();
               freeMol2       = fraction2 * curState2->getFreeMol();
               expelledMol2   = fraction2 * curState2->getExpelledMol();
               retained2      = fraction2 * curState2->getRetained();
               adsorptionCapacity2 = fraction2 * curState2->getAdsorptionCapacity();
               expelledMassSR2     = fraction2 * curState2->getMassExpelledFromSourceRock();
               expelledMassTransientSR2     = fraction2 * curState2->getMassExpelledTransientFromSourceRock();
            } 
         } 
      }
      // now mix
      Genex6::SpeciesResult &theResult3 = GetSpeciesResult(i);
      
      theResult3.SetConcentration( concentration1 + concentration2 );
      theResult3.SetGeneratedRate( generatedRate1 + generatedRate2 );
      theResult3.SetGeneratedCum( generatedCum1 + generatedCum2 );
      theResult3.SetExpelledMass( expelledMassRes1 + expelledMassRes2 );
      theResult3.setGeneratedMass( generatedMassRes1 + generatedMassRes2 );
      theResult3.SetFlux( flux1 + flux2 );

      curState = GetSpeciesStateById (i);
      
      if( curState == 0 ) {
         const Species *curSpecies = 0;
         
         curSpecies =  ( id1 < id2 ? curSpecies2 : curSpecies1 );

         if( curSpecies != 0 ) {
            curState = new SpeciesState ( curSpecies ); //inSimulatorState1->getSpecies() );
         }
         AddSpeciesStateById(i, curState);      
      }

      curState->SetExpelledMass( expelledMass1 + expelledMass2, true );
      curState->setGeneratedMass( generatedMass1 + generatedMass2 );
      curState->setExpelledMassTransient( massTrans1 + massTrans2 );
      curState->setAdsorpedMol(adsorpedMol1 + adsorpedMol2);
      curState->setDesorpedMol(desorpedMol1 + desorpedMol2);
      curState->setFreeMol(freeMol1 + freeMol2);
      curState->setExpelledMol(expelledMol1 + expelledMol2);
      curState->setRetained(retained1 + retained2);
      curState->setAdsorptionCapacity(adsorptionCapacity1 + adsorptionCapacity2);
      curState->setMassExpelledFromSourceRock(expelledMassSR1 + expelledMassSR2);
      curState->setMassExpelledTransientFromSourceRock(expelledMassTransientSR1 + expelledMassTransientSR2);
   }
   
   
   for ( i = 0; i < LAST_RESULT_ID - FIRST_RESULT_ID; ++ i ) {
      s_GroupResults[i] = fraction1 * inSimulatorState1->s_GroupResults[i] + fraction2 * inSimulatorState2->s_GroupResults[i];
   }
   
}

void SimulatorState::mixIntervalResults ( SimulatorState * inSimulatorState1, SimulatorState * inSimulatorState2,
                                          const double fraction1, const double fraction2 ) {

   const SpeciesManager* speciesManager1 = inSimulatorState1->getSpeciesManager ();
   const SpeciesManager* speciesManager2 = inSimulatorState2->getSpeciesManager ();

   int i;
   int id, curId;

   for( i = 1; i <= m_numberOfSpecies; ++ i ) {
      curId = m_speciesManager->mapIdToComponentManagerSpecies( i );

      if( curId != CBMGenerics::ComponentManager::UNKNOWN ) {
         id = speciesManager1->mapIdToComponentManagerSpecies( i );
         if ( id != CBMGenerics::ComponentManager::UNKNOWN ) {
            m_intervalCumulativeSpecies [ curId ] += fraction1 * inSimulatorState1->m_intervalCumulativeSpecies [ id ];
         }
         id  = speciesManager2->mapIdToComponentManagerSpecies ( i );
         if ( id != CBMGenerics::ComponentManager::UNKNOWN ) {
            m_intervalCumulativeSpecies [ curId ] += fraction2 * inSimulatorState2->m_intervalCumulativeSpecies [ id ];
         }
      }
   }
#if 0
   for( i = 1; i <= m_numberOfSpecies; ++ i ) {
      const string & speciesName = m_speciesManager->getSpeciesName( i );
      id = speciesManager1->getSpeciesIdByName ( speciesName );

      if ( id >= 0 ) {
         index = speciesManager1->mapIdToComponentManagerSpecies ( id );

         if ( id1 != CBMGenerics::ComponentManager::UNKNOWN ) {
            m_intervalCumulativeSpecies [ i ] += fraction1 * inSimulatorState1->m_intervalCumulativeSpecies [ static_cast<int>(index) ];
         }

      }

   }

   for( i = 1; i <= m_numberOfSpecies; ++ i ) {
      const string & speciesName = m_speciesManager->getSpeciesName( i );
      id = speciesManager2->getSpeciesIdByName ( speciesName );

      if ( id >= 0 ) {
         index = speciesManager2->mapIdToComponentManagerSpecies ( id );

         if ( index != CBMGenerics::ComponentManager::UNKNOWN ) {
            m_intervalCumulativeSpecies [ i ] += fraction2 * inSimulatorState2->m_intervalCumulativeSpecies [ static_cast<int>(index) ];
         }

      }

   }
#endif

}


void SimulatorState::PostProcessTimeStepComputation ( SimulatorState * inSimulatorState1, SimulatorState * inSimulatorState2,
                                                      const double fraction1, const double fraction2 )
{
   int i;
   SetOM(); // might also needs to be mixed

   double OilExpelledMassInst, OilExpelledVolumeInst, HcGasExpelledVolumeInst, WetGasExpelledVolumeInst,
      C614SatPlusAromExpVolInst, AromaticsExpelledVolumeInst, SaturatesExpelledVolumeInst;

   
   if( inSimulatorState1 != 0 && inSimulatorState2 != 0 ) {
      m_referenceTime = inSimulatorState1->m_referenceTime; 
      mixResults( inSimulatorState1, inSimulatorState2, fraction1, fraction2 );
 
      //get instantaneous contributions for particular time step
      OilExpelledMassInst         = fraction1 * inSimulatorState1->GetOilExpelledMassInst() + fraction2 * inSimulatorState2->GetOilExpelledMassInst();
      OilExpelledVolumeInst       = fraction1 * inSimulatorState1->GetOilExpelledVolumeInst() + fraction2 * inSimulatorState2->GetOilExpelledVolumeInst();
      HcGasExpelledVolumeInst     = fraction1 * inSimulatorState1->GetHcGasExpelledVolumeInst() + fraction2 * inSimulatorState2->GetHcGasExpelledVolumeInst();
      WetGasExpelledVolumeInst    = fraction1 * inSimulatorState1->GetWetGasExpelledVolumeInst() + fraction2 *  inSimulatorState2->GetWetGasExpelledVolumeInst();
      C614SatPlusAromExpVolInst   = fraction1 * inSimulatorState1->GetC614SatPlusAromExpVolInst() + fraction2 * inSimulatorState2->GetC614SatPlusAromExpVolInst();
      AromaticsExpelledVolumeInst = fraction1 * inSimulatorState1->GetAromaticsExpelledVolumeInst() + fraction2 * inSimulatorState2->GetAromaticsExpelledVolumeInst();
      SaturatesExpelledVolumeInst = fraction1 * inSimulatorState1->GetSaturatesExpelledVolumeInst() + fraction2 * inSimulatorState2->GetSaturatesExpelledVolumeInst();

      setIrreducibleWaterSaturation ( fraction1 * inSimulatorState1->getIrreducibleWaterSaturation() + fraction2 * inSimulatorState2->getIrreducibleWaterSaturation() );
      setHcSaturation ( fraction1 * inSimulatorState1->getHcSaturation() +  fraction2 * inSimulatorState2->getHcSaturation() );
      setEffectivePorosity ( fraction1 * inSimulatorState1->getEffectivePorosity() +  fraction2 * inSimulatorState2->getEffectivePorosity() );
      setRetainedVapourVolume( fraction1 * inSimulatorState1->getRetainedVapourVolume() + fraction2 * inSimulatorState2->getRetainedVapourVolume() );
      setRetainedLiquidVolume( fraction1 * inSimulatorState1->getRetainedLiquidVolume() + fraction2 * inSimulatorState2->getRetainedLiquidVolume() );
      setVLSRTemperature(  fraction1 * inSimulatorState1->getVLSRTemperature() + fraction2 * inSimulatorState2->getVLSRTemperature() );
      setVLReferenceTemperature(  fraction1 * inSimulatorState1->getVLReferenceTemperature() + fraction2 * inSimulatorState2->getVLReferenceTemperature() );
      setTotalGasFromOtgc(  fraction1 * inSimulatorState1->getTotalGasFromOtgc() + fraction2 * inSimulatorState2->getTotalGasFromOtgc() );
      setH2SFromGenex(  fraction1 * inSimulatorState1->getH2SFromGenex() + fraction2 * inSimulatorState2->getH2SFromGenex() );
      setH2SFromOtgc(  fraction1 * inSimulatorState1->getH2SFromOtgc() + fraction2 * inSimulatorState2->getH2SFromOtgc() );

      setSubSurfaceDensities( inSimulatorState1->getSubSurfaceDensities(), fraction1, inSimulatorState2->getSubSurfaceDensities(), fraction2 );
      setLiquidComponents( inSimulatorState1->getLiquidComponents(), fraction1, inSimulatorState2->getLiquidComponents(), fraction2 );
      setVapourComponents( inSimulatorState1->getVapourComponents(), fraction1, inSimulatorState2->getVapourComponents(), fraction2 );
//  m_thickness = inSimulatorState1->m_thickness; // set up before calling this method

      setCurrentToc( fraction1 * inSimulatorState1->getCurrentToc ()  + fraction2 * inSimulatorState2->getCurrentToc ());
      
      setImmobileRetainedMixed( inSimulatorState1, inSimulatorState2, fraction1, fraction2 );
      mixIntervalResults ( inSimulatorState1, inSimulatorState2, fraction1, fraction2 );
   } else {
      //get instantaneous contributions for particular time step
      OilExpelledMassInst         = GetOilExpelledMassInst();
      OilExpelledVolumeInst       = GetOilExpelledVolumeInst();
      HcGasExpelledVolumeInst     = GetHcGasExpelledVolumeInst();
      WetGasExpelledVolumeInst    = GetWetGasExpelledVolumeInst();
      C614SatPlusAromExpVolInst   = GetC614SatPlusAromExpVolInst();
      AromaticsExpelledVolumeInst = GetAromaticsExpelledVolumeInst();
      SaturatesExpelledVolumeInst = GetSaturatesExpelledVolumeInst();
   }
   //-------------------Complete the simulator state update------------------------------
   
   //Get cumulative quantities from the Simulator State
   double OilExpelledMassCum1          = GetCumQuantity(SimulatorState::OilExpelledMassCum);
   double OilExpelledVolumeCum1        = GetCumQuantity(SimulatorState::OilExpelledVolumeCum);
   double HcGasExpelledVolumeCum1      = GetCumQuantity(SimulatorState::HcGasExpelledVolumeCum);
   double WetGasExpelledVolumeCum1     = GetCumQuantity(SimulatorState::WetGasExpelledVolumeCum);
   double C614SatPlusAromExpVolCum1    = GetCumQuantity(SimulatorState::C614SatPlusAromExpVolCum);
   double AromaticsExpelledVolumeCum1  = GetCumQuantity(SimulatorState::AromaticsExpelledVolumeCum);
   double SaturatesExpelledVolumeCum1  = GetCumQuantity(SimulatorState::SaturatesExpelledVolumeCum);
   
   //add instantaneous to SimulatorState 
   OilExpelledMassCum1         += OilExpelledMassInst;
   OilExpelledVolumeCum1       += OilExpelledVolumeInst;
   HcGasExpelledVolumeCum1     += HcGasExpelledVolumeInst;
   WetGasExpelledVolumeCum1    += WetGasExpelledVolumeInst;
   C614SatPlusAromExpVolCum1   += C614SatPlusAromExpVolInst;
   AromaticsExpelledVolumeCum1 += AromaticsExpelledVolumeInst;
   SaturatesExpelledVolumeCum1 += SaturatesExpelledVolumeInst;
    
   //update Simulator state variables   
   SetCumQuantity(SimulatorState::OilExpelledMassCum,         OilExpelledMassCum1);
   SetCumQuantity(SimulatorState::OilExpelledVolumeCum,       OilExpelledVolumeCum1);
   SetCumQuantity(SimulatorState::HcGasExpelledVolumeCum,     HcGasExpelledVolumeCum1);
   SetCumQuantity(SimulatorState::WetGasExpelledVolumeCum,    WetGasExpelledVolumeCum1);
   SetCumQuantity(SimulatorState::C614SatPlusAromExpVolCum,   C614SatPlusAromExpVolCum1);
   SetCumQuantity(SimulatorState::AromaticsExpelledVolumeCum, AromaticsExpelledVolumeCum1);
   SetCumQuantity(SimulatorState::SaturatesExpelledVolumeCum, SaturatesExpelledVolumeCum1);


   // Increment quantities that are computed for a time interval.
   incrementIntervalQuantity(SimulatorState::OilExpelledMassCum,         OilExpelledMassInst );
   incrementIntervalQuantity(SimulatorState::OilExpelledVolumeCum,       OilExpelledVolumeInst );
   incrementIntervalQuantity(SimulatorState::HcGasExpelledVolumeCum,     HcGasExpelledVolumeInst );
   incrementIntervalQuantity(SimulatorState::WetGasExpelledVolumeCum,    WetGasExpelledVolumeInst );
   incrementIntervalQuantity(SimulatorState::C614SatPlusAromExpVolCum,   C614SatPlusAromExpVolInst );
   incrementIntervalQuantity(SimulatorState::AromaticsExpelledVolumeCum, AromaticsExpelledVolumeInst );
   incrementIntervalQuantity(SimulatorState::SaturatesExpelledVolumeCum, SaturatesExpelledVolumeInst );

   //-------------------------------------------------------------------------------------------------------
   
   using namespace CBMGenerics;
   GenexResultManager & theResultManager = GenexResultManager::getInstance();

   //---------------------------------Complete the output  update-------------------------------------------

   //ExpulsionApiInst, 
   if( theResultManager.IsResultRequired(GenexResultManager::ExpulsionApiInst))
   {
      double ExpulsionApiInst = 0.001;
      double DensOilInst = 0.0;

      if(OilExpelledVolumeInst > Genex6::Constants::FLXVOILZERO) {	
         DensOilInst   = OilExpelledMassInst / OilExpelledVolumeInst;
         ExpulsionApiInst = Genex6::Constants::APIC1 / DensOilInst * Genex6::Constants::APIC2 - Genex6::Constants::APIC3; 
      }

      SetResult(GenexResultManager::ExpulsionApiInst, ExpulsionApiInst);
   }

   //ExpulsionGasOilRatioInst
   //ExpulsionGasOilRatioInst is needed in ExpulsionCondensateGasRatioInst, which might be indepedently selected to be on
   //double ExpulsionGasOilRatioInst = 1e+20;
   //double ExpulsionGasOilRatioInst = 99999.0;
   double ExpulsionGasOilRatioInst = Genex6::Constants::UNDEFINEDVALUE;
   
   if(OilExpelledVolumeInst > Genex6::Constants::FLXVOILZERO) {
     ExpulsionGasOilRatioInst = HcGasExpelledVolumeInst / OilExpelledVolumeInst;
     //ExpulsionGasOilRatioInst = log(HcGasExpelledVolumeInst / OilExpelledVolumeInst);

     if(ExpulsionGasOilRatioInst > Genex6::Constants::GOR_UPPERBOUND) {
        ExpulsionGasOilRatioInst =  Genex6::Constants::GOR_UPPERBOUND; 
     }
   }
   if(theResultManager.IsResultRequired(GenexResultManager::ExpulsionGasOilRatioInst)) {
      SetResult(GenexResultManager::ExpulsionGasOilRatioInst, ExpulsionGasOilRatioInst);
   }

   //ExpulsionApiCum
   if( theResultManager.IsResultRequired(GenexResultManager::ExpulsionApiCum)) {
      double ExpulsionApiCum = 0.001;

      if(OilExpelledVolumeCum1 > Genex6::Constants::FLXVOILZERO) {
         double DensOilCum  = OilExpelledMassCum1 / OilExpelledVolumeCum1;
         ExpulsionApiCum = Genex6::Constants::APIC1 / DensOilCum * Genex6::Constants::APIC2 - Genex6::Constants::APIC3;
      }

      SetResult(GenexResultManager::ExpulsionApiCum, ExpulsionApiCum);
   }

   //ExpulsionGasOilRatioCum
   //ExpulsionGasOilRatioCum is needed in ExpulsionCondensateGasRatioCum, which might be indepedently selected to be on
   
   double ExpulsionGasOilRatioCum = Genex6::Constants::UNDEFINEDVALUE;
   if(OilExpelledVolumeCum1 > Genex6::Constants::CUMVOILZERO) {
      ExpulsionGasOilRatioCum = HcGasExpelledVolumeCum1 / OilExpelledVolumeCum1 ; 

      if(ExpulsionGasOilRatioCum > Genex6::Constants::GOR_UPPERBOUND) {
          ExpulsionGasOilRatioCum = Genex6::Constants::GOR_UPPERBOUND; 
      }    
   }
   if(theResultManager.IsResultRequired(GenexResultManager::ExpulsionGasOilRatioCum)) {  
      SetResult(GenexResultManager::ExpulsionGasOilRatioCum, ExpulsionGasOilRatioCum);
   }

   //ExpulsionCondensateGasRatioInst,
   if(theResultManager.IsResultRequired(GenexResultManager::ExpulsionCondensateGasRatioInst)) {
      double ExpulsionCondensateGasRatioInst = 0.0;
      if(ExpulsionGasOilRatioInst > 10000.0) {
	      ExpulsionCondensateGasRatioInst = 1.0 / ExpulsionGasOilRatioInst ;   
      }   
      SetResult(GenexResultManager::ExpulsionCondensateGasRatioInst, ExpulsionCondensateGasRatioInst);
   }

   //ExpulsionCondensateGasRatioCum,
   if(theResultManager.IsResultRequired(GenexResultManager::ExpulsionCondensateGasRatioCum)) {
      double ExpulsionCondensateGasRatioCum = 0.0;
      if(ExpulsionGasOilRatioCum > 10000.0) {
	      ExpulsionCondensateGasRatioCum = 1.0 / ExpulsionGasOilRatioCum ;    
      }
      SetResult(GenexResultManager::ExpulsionCondensateGasRatioCum, ExpulsionCondensateGasRatioCum);
   }

   //ExpulsionGasWetnessInst
   if(theResultManager.IsResultRequired( GenexResultManager::ExpulsionGasWetnessInst)) {
      double ExpulsionGasWetnessInst = Genex6::Constants::UNDEFINEDVALUE;
      if(HcGasExpelledVolumeInst > Genex6::Constants::CUMVOILZERO) {
	      ExpulsionGasWetnessInst = WetGasExpelledVolumeInst / HcGasExpelledVolumeInst ;     
      }
      SetResult(GenexResultManager::ExpulsionGasWetnessInst, ExpulsionGasWetnessInst);
   }

   //ExpulsionGasWetnessCum
   if( theResultManager.IsResultRequired(GenexResultManager::ExpulsionGasWetnessCum)) {
      double ExpulsionGasWetnessCum = Genex6::Constants::UNDEFINEDVALUE;


      if(HcGasExpelledVolumeCum1 > Genex6::Constants::CUMVOILZERO) {
	      ExpulsionGasWetnessCum = WetGasExpelledVolumeCum1 / HcGasExpelledVolumeCum1;     
      }   

      SetResult(GenexResultManager::ExpulsionGasWetnessCum,  ExpulsionGasWetnessCum);
   }

   //ExpulsionAromaticityInst, 
   if( theResultManager.IsResultRequired(GenexResultManager::ExpulsionAromaticityInst)) {
      double ExpulsionAromaticityInst = 0.0; 
      if(SaturatesExpelledVolumeInst > Genex6::Constants::CUMVOILZERO) {
	      ExpulsionAromaticityInst = AromaticsExpelledVolumeInst / SaturatesExpelledVolumeInst;   
      }
      SetResult(GenexResultManager::ExpulsionAromaticityInst, ExpulsionAromaticityInst);
   }
   //ExpulsionAromaticityCum
   if(theResultManager.IsResultRequired( GenexResultManager::ExpulsionAromaticityCum)) {
      //double ExpulsionAromaticityCum  = 1.0;
      double ExpulsionAromaticityCum  = 0.0;
      if(SaturatesExpelledVolumeCum1 > Genex6::Constants::CUMVOILZERO) {
	      ExpulsionAromaticityCum = AromaticsExpelledVolumeCum1 / SaturatesExpelledVolumeCum1;   
      }  
      SetResult(GenexResultManager::ExpulsionAromaticityCum, ExpulsionAromaticityCum);
   }

   //Kerogen Conversion Ratio
   if( theResultManager.IsResultRequired(GenexResultManager::KerogenConversionRatio)) {
      double KerogenConversionRatio = Genex6::Constants::UNDEFINEDVALUE;
      if(m_thickness > 0.01) {
         double InitialKerogenMass, KerogenConc, preasphalteneConc;

         if( inSimulatorState1 != 0 && inSimulatorState2 != 0 ) {
                                       
            InitialKerogenMass = fraction1 * inSimulatorState1->GetInitialKerogenConcentration() + fraction2 * inSimulatorState2->GetInitialKerogenConcentration();
            KerogenConc =  fraction1 * inSimulatorState1->GetSpeciesConcentrationByName(inSimulatorState1->getSpeciesManager()->getKerogenId ()) + 
               fraction2 * inSimulatorState2->GetSpeciesConcentrationByName(inSimulatorState2->getSpeciesManager()->getKerogenId ());
            preasphalteneConc  = fraction1 * inSimulatorState1->GetSpeciesConcentrationByName(inSimulatorState1->getSpeciesManager()->getPreasphaltId ()) + 
               fraction2 * inSimulatorState2->GetSpeciesConcentrationByName(inSimulatorState2->getSpeciesManager()->getPreasphaltId ());

         } else {

            InitialKerogenMass = GetInitialKerogenConcentration();
            KerogenConc        = GetSpeciesConcentrationByName(m_speciesManager->getKerogenId ());
            preasphalteneConc  = GetSpeciesConcentrationByName(m_speciesManager->getPreasphaltId ());

         }
         KerogenConversionRatio = 1.0 - (((preasphalteneConc + KerogenConc) * m_thickness) / InitialKerogenMass) ;

         //Isolate numerical artifacts...
         if(KerogenConversionRatio < 0.001){
            KerogenConversionRatio = 0.0;
         }else if(KerogenConversionRatio > 1.0) {
            KerogenConversionRatio = 1.0;
         }
      }
      
      SetResult(GenexResultManager::KerogenConversionRatio,  KerogenConversionRatio);
   }
   //Group results
   //   for(i = s_firstResultId; i < GenexResultManager::NumberOfResults; ++i )

   for(i = FIRST_RESULT_ID; i < LAST_RESULT_ID; ++i ) {
      SetResult(i, GetGroupResult(i));
   }

   // Calculate O/C and H/C
   if( inSimulatorState1 != 0 && inSimulatorState2 != 0 ) {
      m_OC = fraction1 * inSimulatorState1->m_OC + fraction2 * inSimulatorState2->m_OC; 
      m_HC = fraction1 * inSimulatorState1->m_HC + fraction2 * inSimulatorState2->m_HC; 
      setCurrentToc ( fraction1 * inSimulatorState1->getCurrentToc() + fraction2 * inSimulatorState2->getCurrentToc() ); 

   } else {
      if( m_AtomCR != 0.0 ) {
         m_OC = m_AtomOR / m_AtomCR;
         m_HC = m_AtomHR / m_AtomCR;
      }  
   } 
   
}
// all prepocessing steps for Genex
void SimulatorState::ComputeFirstTimeInstance(ChemicalModel *theChmod)
{
   //initialise concentrations
   //geological mode
   //Potential maximum Conc(Lpreasphaltene) is initially treated as one, in the following calcs,
   //to allow non-first order reaction of preasphaltene. Real concentrations and fluxes are achieved by
   //scaling these values with Concki, which is now the true initial conc of "preasphaltene" (kerogen at 0.5% VRE)
   //rather than concentration of "kerogen" at 0.2 %VRE.
   //Thus the initial concentration of this kerogen at 0.2% VRE is now greater than one, in inverse proportion
   //to the stoichiometric factor for preasphaltene generated from the kerogen, as follows.
   //Conc(Lkerogen) = 1 / SFmass(Lpreasphalt, Lkerogen)

   const SpeciesManager& speciesManager = theChmod->getSpeciesManager ();

   theChmod->InitializeSpeciesTimeStepVariables();
   SetSpeciesTimeStepVariablesToZero();

   theChmod->ComputeB0();
   theChmod->ComputePseudoActEnergyRadical();

   Species *kerogen = theChmod->GetSpeciesById(speciesManager.getKerogenId ());
   double preasphalteneMassFactor = 0.0;
   double newTotal = 0.0, newKerogenMass = 0.0;  
   if(kerogen) {

      const Species** theSpecies = theChmod->GetSpecies();
      for(int i = 0, speciesId = 1; i < speciesManager.getNumberOfSpecies (); ++ i, ++ speciesId) {
         if(theSpecies[i] == NULL ) continue;

         // speciesId = theSpecies[i]->GetId ();

         SpeciesState *currState = new SpeciesState( theSpecies [ i ]);

         if(speciesId == speciesManager.getKerogenId ()) {
            preasphalteneMassFactor = kerogen->GetMassFactorBySpecies(speciesManager.getPreasphaltId ());
            newKerogenMass = 1.0 / preasphalteneMassFactor;
            
            SetInitialKerogenConcentration(newKerogenMass);
            //Species State
            currState->SetConcentration(newKerogenMass);
            //Species Result
            GetSpeciesResult(speciesId).SetConcentration(newKerogenMass);
            newTotal += newKerogenMass;
         }
#if 0 
         else if(speciesId == speciesManager.getSO4Id ()) {
            GeneralParametersHandler & theHandler = GeneralParametersHandler::getInstance();
            double massfract_SO4 = theHandler.GetParameterById(GeneralParametersHandler::SO4massfract);
            currState->SetConcentration(massfract_SO4);
            //Species Result
            GetSpeciesResult(speciesId).SetConcentration(massfract_SO4);
            newTotal += massfract_SO4;
         }
#endif
         AddSpeciesStateById(speciesId, currState);
      }

      double AromaticOM = 0.0; 
      double Oil = 0.0;

      if(theChmod->isGX5()){
         AromaticOM = kerogen->GetAromaticity() * newKerogenMass;
      }

      UpdateLumpedConcentrations(newTotal, Oil, AromaticOM);
   } else {
     //throw
   }
   //just for initialization 
   PostProcessTimeStepComputation ();
}

void SimulatorState::PostProcessTimeStep(Species& theSpecies,  const double in_dT)
{

   const SpeciesManager& speciesManager = theSpecies.getChemicalModel ().getSpeciesManager ();

   int speciesId = theSpecies.GetId();
   SpeciesProperties * speciesProps = theSpecies.GetSpeciesProperties();
 
   SpeciesState * currentSpeciesState = GetSpeciesStateById(speciesId);
   double concentration = currentSpeciesState->GetConcentration();

   double expelledMass  = currentSpeciesState->GetExpelledMass();
   double generatedMass = currentSpeciesState->getGeneratedMass();
   
   //Compute flux
   //Flx(J, L) = Theta(L) * Conc(L) * SRthicki * ConcKi
   
   double generatedRate = theSpecies.GetPositiveGenRate() * m_thickness * m_concki;
   double flux = theSpecies.GetTheta() * concentration * m_thickness * m_concki;
   double MassExpelledInst = flux * in_dT;
   double VolumeExpelledInst = MassExpelledInst / speciesProps->GetDensity();
   
   //Compute cummulative mass
   //-----cumulative expelled masses until current time
   //-----ExM(J, L) = ExM(J - 1, L) + Flx(J, L) * dT
   expelledMass += MassExpelledInst;
   generatedMass += MassExpelledInst;

   incrementIntervalSpeciesQuantity ( speciesManager.mapIdToComponentManagerSpecies ( speciesId ), MassExpelledInst );

   //Update the Species State
   //  currentSpeciesState->SetConcentration(concentration);
   currentSpeciesState->SetExpelledMass(expelledMass);
   currentSpeciesState->setGeneratedMass ( generatedMass );

   double Oil = 0.0;
   double AromaticOM = 0.0;//initialization, final value only for "preasphalt"
   if(speciesProps->IsOil()) {
      Oil = concentration;
   } else if(theSpecies.isGX5()){
      if(speciesId == speciesManager.getKerogenId ()) {
         AromaticOM = concentration * speciesProps->GetAromaticity();
      } else if(speciesId == speciesManager.getPreasphaltId ()) {
         AromaticOM = 0.0;
      } else {
         AromaticOM = concentration;
      }
   } else if(speciesId == speciesManager.getPrecokeId () || 
             speciesId == speciesManager.getCoke1Id () || 
             speciesId == speciesManager.getCoke2Id () || 
             speciesId == speciesManager.getHetero1Id () ||
             speciesId == speciesManager.getCokeSId ()) {
     AromaticOM = concentration;
   }
   UpdateLumpedConcentrations(concentration, Oil, AromaticOM);

   currentSpeciesState->setExpelledMassTransient ( MassExpelledInst );


   //Set Species Result
   GetSpeciesResult(speciesId).SetConcentration(concentration);
   GetSpeciesResult(speciesId).SetExpelledMass(expelledMass);
   GetSpeciesResult(speciesId).setGeneratedMass ( generatedMass );
   GetSpeciesResult(speciesId).SetFlux(flux);
   GetSpeciesResult(speciesId).SetGeneratedRate(theSpecies.GetPositiveGenRate());

   //Compute the static variables that will be used for the computation of further SourceRockNodeOutput quantities
   //'''''total expelled masses and effective diffusivities
   //ExmTot = ExmTot + ExM(J, L)

   //   Species::s_ExmTot += expelledMass;
   AddExmTot(expelledMass);
   double value = (concentration * m_thickness * m_concki) + expelledMass;
   double generatedValue = (concentration * m_thickness * m_concki) + generatedMass;

   GetSpeciesResult(speciesId).SetGeneratedCum( value );
   
   using namespace CBMGenerics;
   if(speciesProps->IsOil()){

     //2007.093 addition for Volker/Ozkan
     //2007.091 addition for Volker/Ozkan))

      AddOilExpelledMassInst( MassExpelledInst );
      AddOilExpelledVolumeInst( VolumeExpelledInst );

      AddGroupResult(GenexResultManager::OilGeneratedCum, value);
      AddGroupResult(GenexResultManager::OilGeneratedRate, generatedRate);
      AddGroupResult(GenexResultManager::OilExpelledCum, expelledMass);
      AddGroupResult(GenexResultManager::OilExpelledRate, flux );//* in_dT); 

      ComponentManager & theComponentManager = ComponentManager::getInstance();
      if(theComponentManager.isSbearingHCsComponent(speciesManager.mapIdToComponentManagerSpecies (speciesId))) {
         AddGroupResult(GenexResultManager::SbearingHCsGeneratedCum,  value);
         AddGroupResult(GenexResultManager::SbearingHCsGeneratedRate, generatedRate);
         AddGroupResult(GenexResultManager::SbearingHCsExpelledCum,   expelledMass);
         AddGroupResult(GenexResultManager::SbearingHCsExpelledRate,  flux );
      }

      //should use a criterio here, tbd IsC614SaturatesAndAromatics, IsAromatics, IsSaturates
      if(speciesId == speciesManager.getC6to14SatId ()) {

         AddC614SatPlusAromExpVolInst(VolumeExpelledInst);
         AddSaturatesExpelledVolumeInst(VolumeExpelledInst);

      } else if(speciesId == speciesManager.getC6to14AroId ()) {

         AddC614SatPlusAromExpVolInst(VolumeExpelledInst);
         AddAromaticsExpelledVolumeInst(VolumeExpelledInst);

      } else if(speciesId == speciesManager.getC15plusAroId () || speciesId == speciesManager.getC15plusAroSId () ||
                speciesId == speciesManager.getC6to14AroSId ()){

         AddAromaticsExpelledVolumeInst(VolumeExpelledInst);
      } else if(speciesId == speciesManager.getC15plusSatId () || speciesId == speciesManager.getC15plusSatSId () ||
                speciesId == speciesManager.getC6to14SatSId ()) {

         AddSaturatesExpelledVolumeInst(VolumeExpelledInst);
      }

   }
    //should use a criterio here, tbd IsWetGas, IsC614SaturatesAndAromatics, IsAromatics, IsSaturates
   if(speciesProps->IsHCgas()) {
      AddHcGasExpelledVolumeInst(VolumeExpelledInst);

      AddGroupResult(GenexResultManager::HcGasGeneratedCum,  value);
      AddGroupResult(GenexResultManager::HcGasGeneratedRate, generatedRate);
      AddGroupResult(GenexResultManager::HcGasExpelledCum,   expelledMass);
      AddGroupResult(GenexResultManager::HcGasExpelledRate,  flux); 

      //wet gas
      if( speciesId == speciesManager.getC5Id () || speciesId == speciesManager.getC4Id () ||
          speciesId == speciesManager.getC3Id () || speciesId == speciesManager.getC2Id () ) {
         AddWetGasExpelledVolumeInst(VolumeExpelledInst);
        
         AddGroupResult(GenexResultManager::WetGasGeneratedCum,  value);
         AddGroupResult(GenexResultManager::WetGasGeneratedRate, generatedRate);
         AddGroupResult(GenexResultManager::WetGasExpelledCum,   expelledMass);
         AddGroupResult(GenexResultManager::WetGasExpelledRate,  flux); 
      } else {
         //dry gas
         AddGroupResult(GenexResultManager::DryGasGeneratedCum,  generatedValue );
         AddGroupResult(GenexResultManager::DryGasGeneratedRate, generatedRate);
         AddGroupResult(GenexResultManager::DryGasExpelledCum,   expelledMass);
         AddGroupResult(GenexResultManager::DryGasExpelledRate,  flux); 
      }
   } 
   GeneralParametersHandler & theHandler = GeneralParametersHandler::getInstance();
   // add to H/C total and O/C total
   if( speciesProps->GetDiffusionEnergy1() >= theHandler.GetParameterById(GeneralParametersHandler::ActCrit) ) {
      double cmol = concentration / speciesProps->GetMolWeight();
      m_AtomCR += cmol * theSpecies.GetCompositionByElement(speciesManager.getCarbonId ());
      m_AtomHR += cmol * theSpecies.GetCompositionByElement(speciesManager.getHydrogenId ());
      m_AtomOR += cmol * theSpecies.GetCompositionByElement(speciesManager.getOxygenId ());
   }
}

void SimulatorState::postProcessShaleGasTimeStep ( ChemicalModel *chemicalModel, const double deltaT ) {

   // The methods for computing some of the various values in this function can be 
   // found in the function called PostProcessTimeStepComputation in this class.

   const SpeciesManager& speciesManager = chemicalModel->getSpeciesManager ();
   ComponentManager & theComponentManager = ComponentManager::getInstance();

   double expelledGasVolume = 0.0;
   double transientGasVolume = 0.0;
   double expelledWetGasVolume = 0.0;
   double transientWetGasVolume = 0.0;

   double expelledOilVolume = 0.0;
   double transientOilVolume = 0.0;
   double transientOilMass = 0.0;
   double cumulativeSaturatesVolume = 0.0;
   double transientSaturatesVolume = 0.0;
   double cumulativeAromaticsVolume = 0.0;
   double transientAromaticsVolume = 0.0;


   int i;
   int speciesId;

   // Set all values 
   for(i = 0; i < CBMGenerics::GenexResultManager::NumberOfResults; ++ i) {
      m_shaleGasResultsByResultId[i] = 0.0;
   }

   // First stage is to set the results that are sums of species.
   // Also some intermediate values are computed here that will be used in the second stage.
   for ( i = 0, speciesId = 1; i < speciesManager.getNumberOfSpecies (); ++i, ++speciesId ) {

      if ( GetSpeciesStateById ( speciesId ) != 0 ) {

         SpeciesProperties * speciesProps = chemicalModel->GetSpeciesById ( speciesId )->GetSpeciesProperties();

         double cumulativeExpelledMass   = GetSpeciesStateById ( speciesId )->getMassExpelledFromSourceRock ();
         double cumulativeExpelledVolume = cumulativeExpelledMass / speciesProps->GetDensity();
         double transientExpelledMass    = GetSpeciesStateById ( speciesId )->getMassExpelledTransientFromSourceRock ();
         double transientExpelledVolume  = transientExpelledMass / speciesProps->GetDensity();

         if ( speciesProps->IsOil ()) {
            addShaleGasResult ( GenexResultManager::OilExpelledCum,  cumulativeExpelledMass );
            addShaleGasResult ( GenexResultManager::OilExpelledRate, transientExpelledMass / deltaT );

            expelledOilVolume += cumulativeExpelledVolume;
            transientOilVolume += transientExpelledVolume;
            transientOilMass += transientExpelledMass;

            if ( theComponentManager.isSbearingHCsComponent(speciesManager.mapIdToComponentManagerSpecies (speciesId))) {
               addShaleGasResult ( GenexResultManager::SbearingHCsExpelledCum,  cumulativeExpelledMass );
               addShaleGasResult ( GenexResultManager::SbearingHCsExpelledRate, transientExpelledMass / deltaT );
            }

            if ( speciesId == speciesManager.getC6to14SatId ()) {
               cumulativeSaturatesVolume += cumulativeExpelledVolume;
               transientSaturatesVolume += transientExpelledVolume;
            } else if ( speciesId == speciesManager.getC6to14AroId ()) {
               cumulativeAromaticsVolume += cumulativeExpelledVolume;
               transientAromaticsVolume += transientExpelledVolume;
            } else if ( speciesId == speciesManager.getC15plusSatId () or
                        speciesId == speciesManager.getC15plusSatSId () or
                        speciesId == speciesManager.getC6to14SatSId ()) {

               cumulativeSaturatesVolume += cumulativeExpelledVolume;
               transientSaturatesVolume += transientExpelledVolume;
            } else if ( speciesId == speciesManager.getC15plusAroId () or
                        speciesId == speciesManager.getC15plusAroSId () or
                        speciesId == speciesManager.getC6to14AroSId ()){

               cumulativeAromaticsVolume += cumulativeExpelledVolume;
               transientAromaticsVolume += transientExpelledVolume;
            }

         }

         if ( speciesProps->IsHCgas ()) {
            addShaleGasResult ( GenexResultManager::HcGasExpelledCum,  cumulativeExpelledMass );
            addShaleGasResult ( GenexResultManager::HcGasExpelledRate, transientExpelledMass / deltaT );


            expelledGasVolume += cumulativeExpelledVolume;
            transientGasVolume += transientExpelledVolume;

            if ( speciesId == speciesManager.getC1Id ()) {
               addShaleGasResult ( GenexResultManager::DryGasExpelledCum,  cumulativeExpelledMass );
               addShaleGasResult ( GenexResultManager::DryGasExpelledRate, transientExpelledMass / deltaT );
            } else {
               addShaleGasResult ( GenexResultManager::WetGasExpelledCum,  cumulativeExpelledMass );
               addShaleGasResult ( GenexResultManager::WetGasExpelledRate, transientExpelledMass / deltaT );

               expelledWetGasVolume += cumulativeExpelledVolume;
               transientWetGasVolume += transientExpelledVolume;

            }

         }

      }

   }

   // The second stage is to compute the more complex results.

   // Expulsion API cumulative.
   if ( expelledOilVolume > Genex6::Constants::FLXVOILZERO ) {

      double oilDensity = getShaleGasResult ( GenexResultManager::OilExpelledCum ) / expelledOilVolume;
      double oilApi = Genex6::Constants::APIC1 / oilDensity * Genex6::Constants::APIC2 - Genex6::Constants::APIC3; 

      setShaleGasResult ( GenexResultManager::ExpulsionApiCum, oilApi );
   } else {
      // Should the value be Genex6::Constants::UNDEFINEDVALUE here rather than 0.001?
      setShaleGasResult ( GenexResultManager::ExpulsionApiCum, 0.001 );
   }

   if ( transientOilVolume > Genex6::Constants::FLXVOILZERO ) {

      double oilDensity = transientOilMass / transientOilVolume;
      double oilApi = Genex6::Constants::APIC1 / oilDensity * Genex6::Constants::APIC2 - Genex6::Constants::APIC3; 

      setShaleGasResult ( GenexResultManager::ExpulsionApiInst, oilApi );
   } else {
      // Should the value be Genex6::Constants::UNDEFINEDVALUE here rather than 0.001?
      setShaleGasResult ( GenexResultManager::ExpulsionApiInst, 0.001 );
   }

   if ( cumulativeSaturatesVolume > Genex6::Constants::CUMVOILZERO ) {
      setShaleGasResult ( GenexResultManager::ExpulsionAromaticityCum, cumulativeAromaticsVolume / cumulativeSaturatesVolume );
   } else {
      setShaleGasResult ( GenexResultManager::ExpulsionAromaticityCum, Genex6::Constants::UNDEFINEDVALUE );
   }

   if ( transientSaturatesVolume > 0.0 ) { //Genex6::Constants::CUMVOILZERO ) {
      setShaleGasResult ( GenexResultManager::ExpulsionAromaticityInst, transientAromaticsVolume / transientSaturatesVolume );
   } else {
      setShaleGasResult ( GenexResultManager::ExpulsionAromaticityInst, Genex6::Constants::UNDEFINEDVALUE );
   }

   if ( expelledOilVolume != 0.0 ) {
      double gor = expelledGasVolume / expelledOilVolume;

      if ( gor > Genex6::Constants::GOR_UPPERBOUND ) {
         gor = Genex6::Constants::GOR_UPPERBOUND;
      }

      setShaleGasResult ( GenexResultManager::ExpulsionGasOilRatioCum, gor );
   } else {
      setShaleGasResult ( GenexResultManager::ExpulsionGasOilRatioCum, Genex6::Constants::UNDEFINEDVALUE );
   }

   if ( transientOilVolume != 0.0 ) {
      double gor = transientGasVolume / transientOilVolume;

      if ( gor > Genex6::Constants::GOR_UPPERBOUND ) {
         gor = Genex6::Constants::GOR_UPPERBOUND;
      }

      setShaleGasResult ( GenexResultManager::ExpulsionGasOilRatioInst, gor );
   } else {
      setShaleGasResult ( GenexResultManager::ExpulsionGasOilRatioInst, Genex6::Constants::UNDEFINEDVALUE );
   }

   if ( getShaleGasResult ( GenexResultManager::ExpulsionGasOilRatioCum ) != 0.0 and
        getShaleGasResult ( GenexResultManager::ExpulsionGasOilRatioCum ) != Genex6::Constants::UNDEFINEDVALUE ) {
      double cgr = 0.0;

      if ( getShaleGasResult ( GenexResultManager::ExpulsionGasOilRatioCum ) > 10000.0 ) {
         cgr = 1.0 / getShaleGasResult ( GenexResultManager::ExpulsionGasOilRatioCum );
      }

      setShaleGasResult ( GenexResultManager::ExpulsionCondensateGasRatioCum, cgr );
   } else {
      setShaleGasResult ( GenexResultManager::ExpulsionCondensateGasRatioCum, Genex6::Constants::UNDEFINEDVALUE );
   }

   if ( getShaleGasResult ( GenexResultManager::ExpulsionGasOilRatioInst ) != 0.0 and
        getShaleGasResult ( GenexResultManager::ExpulsionGasOilRatioInst ) != Genex6::Constants::UNDEFINEDVALUE ) {
      double cgr = 0.0;

      if ( getShaleGasResult ( GenexResultManager::ExpulsionGasOilRatioInst ) > 10000.0 ) {
         cgr = 1.0 / getShaleGasResult ( GenexResultManager::ExpulsionGasOilRatioInst );
      }

      setShaleGasResult ( GenexResultManager::ExpulsionCondensateGasRatioInst, cgr );
   } else {
      setShaleGasResult ( GenexResultManager::ExpulsionCondensateGasRatioInst, Genex6::Constants::UNDEFINEDVALUE );
   }

   if ( expelledGasVolume > Genex6::Constants::CUMVOILZERO ) {
      setShaleGasResult ( GenexResultManager::ExpulsionGasWetnessCum, expelledWetGasVolume / expelledGasVolume );
   } else {
      setShaleGasResult ( GenexResultManager::ExpulsionGasWetnessCum, Genex6::Constants::UNDEFINEDVALUE );
   }

   if ( transientGasVolume > Genex6::Constants::CUMVOILZERO ) {
      setShaleGasResult ( GenexResultManager::ExpulsionGasWetnessInst, transientWetGasVolume / transientGasVolume );
   } else {
      setShaleGasResult ( GenexResultManager::ExpulsionGasWetnessInst, Genex6::Constants::UNDEFINEDVALUE );
   }

   // The third step
   // Copy remaining values from list, mainly the generated values and the kerogen-conversion ratio.
   // Having all the optional results here simplifies the copying of the results to the output maps.
   setShaleGasResult ( GenexResultManager::KerogenConversionRatio, GetResult ( GenexResultManager::KerogenConversionRatio ));
   setShaleGasResult ( GenexResultManager::OilGeneratedCum, GetResult ( GenexResultManager::OilGeneratedCum ));
   setShaleGasResult ( GenexResultManager::OilGeneratedRate, GetResult ( GenexResultManager::OilGeneratedRate ));
   setShaleGasResult ( GenexResultManager::HcGasGeneratedCum, GetResult ( GenexResultManager::HcGasGeneratedCum ));
   setShaleGasResult ( GenexResultManager::HcGasGeneratedRate, GetResult ( GenexResultManager::HcGasGeneratedRate ));
   setShaleGasResult ( GenexResultManager::DryGasGeneratedCum, GetResult ( GenexResultManager::DryGasGeneratedCum ));
   setShaleGasResult ( GenexResultManager::DryGasGeneratedRate, GetResult ( GenexResultManager::DryGasGeneratedRate ));
   setShaleGasResult ( GenexResultManager::WetGasGeneratedCum,  GetResult ( GenexResultManager::WetGasGeneratedCum ));
   setShaleGasResult ( GenexResultManager::WetGasGeneratedRate, GetResult ( GenexResultManager::WetGasGeneratedRate ));
   setShaleGasResult ( GenexResultManager::SbearingHCsGeneratedCum, GetResult ( GenexResultManager::SbearingHCsGeneratedCum ));
   setShaleGasResult ( GenexResultManager::SbearingHCsGeneratedRate, GetResult ( GenexResultManager::SbearingHCsGeneratedRate ));

}


double SimulatorState::ComputeKerogenTransformatioRatio ( const SpeciesManager& speciesManager,
                                                          int aSimulationType) 
{
   double kerogenTransformationRatio = 
      1.0 - (GetSpeciesConcentrationByName( speciesManager.getKerogenId ()) + 
                GetSpeciesConcentrationByName( speciesManager.getPreasphaltId ()));
   if(!(aSimulationType & Genex6::Constants::SIMGENEX5)) {
      if(kerogenTransformationRatio < 0.0) kerogenTransformationRatio = 0.0;
   }
   return kerogenTransformationRatio;
}

double SimulatorState::ComputeDiffusionConcDependence(const double in_Waso) 
{
   GeneralParametersHandler & theHandler = GeneralParametersHandler::getInstance();
     
   static double WboMin = theHandler.GetParameterById(GeneralParametersHandler::WboMin);
   double DiffusionConcDependence = ComputeWbo() * pow((1.0 - in_Waso), 2.0) + WboMin;

   return DiffusionConcDependence;
}


void SimulatorState::setImmobileSpecies ( const ImmobileSpecies& immobiles ) {
   m_immobileSpecies = immobiles;
}

void SimulatorState::setImmobileDensitiesMixed ( SimulatorState * aState1,  SimulatorState * aState2, const double f1, const double f2 ) {
   unsigned int i;
   double density1, density2;

   for ( i = 0; i < ImmobileSpecies::NUM_IMMOBILES; ++i ) {
      ImmobileSpecies::SpeciesId id = ImmobileSpecies::SpeciesId ( i );
      if( aState1 != 0 && f1 != 0 ) {
         density1 = aState1->getImmobileSpecies().getDensity( id ) * f1;
      } else {
         density1 = 0;
      }
      if( aState2 != 0 && f2 != 0 ) {
         density2 = aState2->getImmobileSpecies().getDensity( id ) * f2;
      } else {
         density2 = 0;
      }

      m_immobileSpecies.setDensity( id, density1 + density2 );
   }
}
void SimulatorState::setImmobileRetainedMixed ( SimulatorState * aState1,  SimulatorState * aState2, const double f1, const double f2 ) {
   unsigned int i;
   double retained1, retained2;

   for ( i = 0; i < ImmobileSpecies::NUM_IMMOBILES; ++i ) {
      ImmobileSpecies::SpeciesId id = ImmobileSpecies::SpeciesId ( i );
      if( aState1 != 0 && f1 != 0 ) {
         retained1 = aState1->getImmobileSpecies().getRetained( id ) * f1;
      } else {
         retained1 = 0;
      }
      if( aState2 != 0 && f2 != 0 ) {
         retained2 = aState2->getImmobileSpecies().getRetained( id ) * f2;
      } else {
         retained2 = 0;
      }

      m_immobileSpecies.setRetained( id, retained1 + retained2 );
   }
}

PVTComponents SimulatorState::getGeneratedComponents () const {

   PVTComponents generated;

   return generated;
}


void SimulatorState::setSubSurfaceDensities ( const PVTPhaseValues& densities1, const double scale1, const PVTPhaseValues& densities2, const double scale2 ) {
   subSurfaceDensities.setValues ( densities1, scale1, densities2, scale2 );
}

void SimulatorState::setSubSurfaceDensities ( const PVTPhaseValues& densities ) {
   subSurfaceDensities = densities;
}

const PVTPhaseValues& SimulatorState::getSubSurfaceDensities () const {
   return subSurfaceDensities;
}

void SimulatorState::setLiquidComponents ( const PVTComponents& liquid1, const double scale1, const PVTComponents& liquid2, const double scale2 ) {
   m_liquidComponents.setComponents( liquid1, scale1, liquid2, scale2 );
}

void SimulatorState::setLiquidComponents ( const PVTComponents& liquid ) {

   m_liquidComponents = liquid;
}

void SimulatorState::setVapourComponents ( const PVTComponents& vapour1, const double scale1, const PVTComponents& vapour2, const double scale2 ) {
   m_vapourComponents.setComponents( vapour1, scale1, vapour2, scale2 );
}

void SimulatorState::setVapourComponents ( const PVTComponents& vapour ) {
   m_vapourComponents = vapour;
}

void SimulatorState::addLiquidComponents ( const PVTComponents& liquid ) {

   m_liquidComponents += liquid;
}

void SimulatorState::addVapourComponents ( const PVTComponents& vapour ) {
   m_vapourComponents += vapour;
}

void SimulatorState::setRetainedVolumes ( const double retainedVapour,
                                          const double retainedLiquid ) {

   m_retainedVapourVolume = retainedVapour;
   m_retainedLiquidVolume = retainedLiquid;

}

void SimulatorState::setEffectivePorosity ( const double effectivePorosity ) {
   m_effectivePorosity = effectivePorosity;
}

void SimulatorState::setHcSaturation ( const double hcSaturation ) {
   m_hcSaturation = hcSaturation;
}

void SimulatorState::setIrreducibleWaterSaturation ( const double irreducibleWaterSaturation ) {
   m_irreducibleWaterSaturation = irreducibleWaterSaturation;
}

void SimulatorState::setRetainedVapourVolume ( const double retainedVapourVolume ) {
   m_retainedVapourVolume = retainedVapourVolume;
}

void SimulatorState::setRetainedLiquidVolume ( const double retainedLiquidVolume ) {
   m_retainedLiquidVolume = retainedLiquidVolume;
}

void SimulatorState::incrementTotalGasFromOtgc ( const double increment ) {
   m_totalGasFromOtgc += increment;
}

void SimulatorState::setVLReferenceTemperature ( const double vl ) {
   m_VLReferenceTemperature = vl;
}

double SimulatorState::getVLReferenceTemperature () const {
   return m_VLReferenceTemperature;
}


void SimulatorState::setVLSRTemperature ( const double vl ) {
   m_VLSRTemperature = vl;
}


double SimulatorState::getVLSRTemperature () const {
   return m_VLSRTemperature;
}

void SimulatorState::addH2SFromGenex ( const double h2s ) {
   m_h2sFromGenex += h2s;
}

void SimulatorState::setH2SFromGenex ( const double h2s ) {
   m_h2sFromGenex = h2s;
}

void SimulatorState::addH2SFromOtgc ( const double h2s ) {
   m_h2sFromOtgc += h2s;
}

void SimulatorState::setH2SFromOtgc ( const double h2s ) {
   m_h2sFromOtgc = h2s;
}

void SimulatorState::setTotalGasFromOtgc ( const double totalGasFromOTGC ) {
   m_totalGasFromOtgc = totalGasFromOTGC;
}

void SimulatorState::incrementIntervalQuantity ( const CumullativeQuantityId id,
                                                 const double                increment ) {
   m_intervalCumulativeQuantities [id] += increment;
}

void SimulatorState::incrementIntervalSpeciesQuantity ( const int    id,
                                                        const double increment ) {
   if( id != CBMGenerics::ComponentManager::UNKNOWN ) {
      m_intervalCumulativeSpecies [id] += increment;
   }
}

void SimulatorState::resetIntervalCumulative () {

   int i;

   m_intervalCumulativeQuantities [OilExpelledMassCum] = 0.0;
   m_intervalCumulativeQuantities [OilExpelledVolumeCum] = 0.0;
   m_intervalCumulativeQuantities [HcGasExpelledVolumeCum] = 0.0;
   m_intervalCumulativeQuantities [C614SatPlusAromExpVolCum] = 0.0;
   m_intervalCumulativeQuantities [AromaticsExpelledVolumeCum] = 0.0;
   m_intervalCumulativeQuantities [SaturatesExpelledVolumeCum] = 0.0;
   m_intervalCumulativeQuantities [WetGasExpelledVolumeCum] = 0.0;

   for ( i = 0; i < CBMGenerics::ComponentManager::NumberOfOutputSpecies; ++i ) {
      m_intervalCumulativeSpecies [ i ] = 0.0;
   } 

}

double SimulatorState::getIntervalCumulative ( const CumullativeQuantityId id ) const {
   return m_intervalCumulativeQuantities [ int ( id )];
}


double SimulatorState::getIntervalSpeciesCumulative ( const CBMGenerics::ComponentManager::SpeciesNamesId id ) const {
   return m_intervalCumulativeSpecies [ int ( id )];
}



}
