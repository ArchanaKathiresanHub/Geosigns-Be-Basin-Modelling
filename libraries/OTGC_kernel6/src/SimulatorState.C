#include "SimulatorState.h"
#include "SpeciesState.h"
#include "SpeciesProperties.h"
#include "Species.h"
#include "ChemicalModel.h"
#include <map>
#include <iostream>
#include <iomanip>

using namespace Genex6; 
using namespace std; 

namespace OTGC6
{
SimulatorState::SimulatorState() : SimulatorStateBase()
{
}

SimulatorState::SimulatorState(const double inReferenceTime, 
                               const Species ** SpeciesInChemicalModel,
                               const std::map<string, double> &initSpeciesConcs ) : SimulatorStateBase()
{
   SetReferenceTime( inReferenceTime );

   m_isInitialized = false;
 
   double conc;

   std::map<string, double>::const_iterator itConcsEnd = initSpeciesConcs.end();

   for(int i = 0; i < Genex6::SpeciesManager::numberOfSpecies; ++ i) {

      if(SpeciesInChemicalModel[i] != NULL) {
         const string speciesName = SpeciesInChemicalModel[i]->GetName();
         std::map<string, double>::const_iterator itSpeciesConc = initSpeciesConcs.find(speciesName);
         conc = 0.0;

         if(itSpeciesConc != itConcsEnd) {
             conc = itSpeciesConc->second;
         }

         Genex6::SpeciesState *stateToAdd = new Genex6::SpeciesState( SpeciesInChemicalModel[i], conc);

         AddSpeciesStateById(SpeciesInChemicalModel[i]->GetId(), stateToAdd);
      }

   }
//    if(Genex6::SpeciesManager::LSO4 > 0) {
//       GetSpeciesStateById(Genex6::SpeciesManager::LSO4)->SetConcentration(2);
//       GetSpeciesResult(Genex6::SpeciesManager::LSO4).SetConcentration(2);
//    }
}


SimulatorState::SimulatorState(const double inReferenceTime, 
                               const Species ** SpeciesInChemicalModel,
                               const double  *  initSpeciesConcs ) : SimulatorStateBase()
{
   SetReferenceTime( inReferenceTime );

   m_isInitialized = false;
 
   for(int i = 0; i < Genex6::SpeciesManager::numberOfSpecies; ++ i) {

      if(SpeciesInChemicalModel[i] != NULL) {
         Genex6::SpeciesState *stateToAdd = new Genex6::SpeciesState( SpeciesInChemicalModel[i], initSpeciesConcs[i] );

         AddSpeciesStateById( SpeciesInChemicalModel[i]->GetId(), stateToAdd );
      } 

   }
//    if(Genex6::SpeciesManager::LSO4 > 0) {
//       GetSpeciesStateById(Genex6::SpeciesManager::LSO4)->SetConcentration(2);
//       GetSpeciesResult(Genex6::SpeciesManager::LSO4).SetConcentration(2);
//    }
}

void SimulatorState::initSpeciesUltimateMass( const SpeciesManager& speciesManager )
{

   unsigned int i;

   for ( i = 1; i <= speciesManager.getNumberOfSpecies (); ++i ) {
      m_UltimateMassesBySpeciesName[ i - 1 ] = GetSpeciesConcentrationByName( i );  
   }

}
}
