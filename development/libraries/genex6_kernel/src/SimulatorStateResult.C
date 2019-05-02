#include "SimulatorStateResult.h"
#include "SimulatorState.h"

using namespace Genex6;

SimulatorStateResult::SimulatorStateResult( SimulatorState & in_State, const bool snapshot ) {

   m_numberOfSpecies = in_State.getNumberOfSpecies();

   m_SpeciesResults = new SpeciesResult[m_numberOfSpecies];
   for(int i = 0; i <  m_numberOfSpecies; ++ i) {
      m_SpeciesResults[i] = in_State.GetSpeciesResult(i + 1);
   }

   m_HC = in_State.getHC();
   m_OC = in_State.getOC();

   if( snapshot ) {
      m_ResultsByResultId = new double[CBMGenerics::GenexResultManager::NumberOfResults];
      for(int i = 0; i <  CBMGenerics::GenexResultManager::NumberOfResults; ++ i) {
         m_ResultsByResultId[i] = in_State.GetResult(i);
      }
   } else {
      m_ResultsByResultId = 0;
   }
   m_thickness = in_State.GetThickness();
   m_currentToc = in_State.getCurrentToc();
}

SimulatorStateResult::~SimulatorStateResult() {

   clean();

}

void SimulatorStateResult::clean () {

   if( m_SpeciesResults != 0 ) {
      delete [] m_SpeciesResults;
      m_SpeciesResults = 0;
   }
  
   if( m_ResultsByResultId != 0 ) {
      delete [] m_ResultsByResultId;
      m_ResultsByResultId = 0;
   }
}

SpeciesResult & SimulatorStateResult::getSpeciesResult( int i ) const {

   return m_SpeciesResults[i];
}

double * SimulatorStateResult::getResultsByResults() const {

   return m_ResultsByResultId;
}
