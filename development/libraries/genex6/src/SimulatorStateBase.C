#include "SimulatorStateBase.h"
#include "ChemicalModel.h"

#include <map>

namespace Genex6
{
SimulatorStateBase::SimulatorStateBase ( const double in_referenceTime)
{
   m_maxprecokeTransformationRatio = 0.0;
   m_maxcoke2TransformationRatio = 0.0;
   m_referenceTime = in_referenceTime;
   m_timeStep = 0;
   m_isInitialized = false;

   m_tocAtVre05Set = false;
   m_tocAtVre05 = 0.0;
   m_initialToc = 0.0;
   m_currentToc = 0.0;
   m_InorganicDensity = 0.0;

   for(int i = 0; i < Genex6::SpeciesManager::numberOfSpecies; ++ i) {
      m_SpeciesStateBySpeciesName[i] = NULL;
      m_UltimateMassesBySpeciesName[i] = 0.0;
   }

}
SimulatorStateBase::~SimulatorStateBase()
{
   clearSpeciesState();
}

void SimulatorStateBase::clearSpeciesState()
{
   for(int i = 0; i < Genex6::SpeciesManager::numberOfSpecies; ++ i) {
      delete m_SpeciesStateBySpeciesName[i];
      m_SpeciesStateBySpeciesName[i] = NULL;
   }
}
double SimulatorStateBase::GetSpeciesConcentrationByName(const int in_SpeciesId) const
{
   double ret = 0.0;
   if(m_SpeciesStateBySpeciesName[in_SpeciesId - 1] != NULL) {
      ret = m_SpeciesStateBySpeciesName[in_SpeciesId - 1]->GetConcentration();
   }
      
   return ret;
}
SpeciesState * SimulatorStateBase::GetSpeciesStateById(const int in_SpeciesId)
{
   return  m_SpeciesStateBySpeciesName[in_SpeciesId - 1];
}
void SimulatorStateBase::AddSpeciesStateById(const int in_SpeciesId, SpeciesState *theSpeciesState)
{
   if(m_SpeciesStateBySpeciesName[in_SpeciesId - 1] != NULL) {
      delete m_SpeciesStateBySpeciesName[in_SpeciesId - 1];
   }
   m_SpeciesStateBySpeciesName[in_SpeciesId - 1] = theSpeciesState;
}
void SimulatorStateBase::GetSpeciesStateConcentrations( const ChemicalModel* chemicalModel,
                                                        std::map<string, double> &currentSpeciesConcs) const
{
   currentSpeciesConcs.clear();

   for(int i = 0; i < Genex6::SpeciesManager::numberOfSpecies; ++ i) {
      std::string theSpeciesName = chemicalModel->GetSpeciesNameById(i+1);

      if(!theSpeciesName.empty()) {
         currentSpeciesConcs.insert(make_pair( theSpeciesName, m_SpeciesStateBySpeciesName[i]->GetConcentration()));
      }

   }

}
double SimulatorStateBase::getSpeciesUltimateMassByName(const int id) const
{
   return m_UltimateMassesBySpeciesName[id - 1];
}
double * SimulatorStateBase::getSpeciesUltimateMasses()
{
   return m_UltimateMassesBySpeciesName;
}

void SimulatorStateBase::setInitialToc ( const double toc ) {
   m_initialToc = toc;
   m_tocAtVre05 = toc;
   m_tocAtVre05Set = false;
} 

void SimulatorStateBase::setCurrentToc ( const double toc ) {
   m_currentToc = toc;
} 

}
