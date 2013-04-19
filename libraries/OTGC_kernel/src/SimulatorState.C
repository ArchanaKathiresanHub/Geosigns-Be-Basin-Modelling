#include "SimulatorState.h"
#include "SpeciesState.h"
#include <map>

namespace OTGC
{
SimulatorState::SimulatorState()
{
   m_referenceTime = 0.0;

	m_maxprecokeTransformationRatio = 0.0;
	m_maxcoke2TransformationRatio = 0.0;

   m_isInitialized = false;

}
SimulatorState::SimulatorState(const double &inReferenceTime, 
                               const vector<std::string> &SpeciesInChemModel, 
                               const std::map<string, double> &initSpeciesConcs 
                               )
{
   m_referenceTime = inReferenceTime;

	m_maxprecokeTransformationRatio = 0.0;
	m_maxcoke2TransformationRatio = 0.0;

   m_isInitialized = false;
 
   std::map<string, double>::const_iterator itConcsEnd = initSpeciesConcs.end();
   for(vector<std::string>::const_iterator it = SpeciesInChemModel.begin(), itEnd = SpeciesInChemModel.end(); it!=itEnd; ++it)
   {
      std::map<string, double>::const_iterator itSpeciesConc = initSpeciesConcs.find(*it);
      double conc = 0.0;
      if(itSpeciesConc != itConcsEnd)
      {
         conc = itSpeciesConc->second;
      }
      SpeciesState *stateToAdd = new SpeciesState(conc);
      AddSpeciesStateByName(*it, stateToAdd);
   }
}
double SimulatorState::getSpeciesUltimateMassByName(const std::string & name)
{
   std::map<std::string,double>::const_iterator it;
   double ret = (it = m_UltimateMassesBySpeciesName.find(name) ) == m_UltimateMassesBySpeciesName.end() ? 0.0 : it->second;
   return ret;
}
void SimulatorState::setSpeciesUltimateMassesByName(const std::map<std::string, double> &inUltimateMassesByName)
{
   m_UltimateMassesBySpeciesName = inUltimateMassesByName;
}
void SimulatorState::clearSpeciesState()
{
   std::map<std::string,SpeciesState*>::iterator itEnd = m_SpeciesStateBySpeciesName.end();
   for(std::map<std::string,SpeciesState*>::iterator it = m_SpeciesStateBySpeciesName.begin();it != itEnd; ++it)
   {
      delete (it->second);
   }
   m_SpeciesStateBySpeciesName.clear();
}
SimulatorState::~SimulatorState()
{
   clearSpeciesState();
}
void SimulatorState::AddSpeciesStateByName(const std::string &in_SpeciesName, SpeciesState *theSpeciesState)
{
   std::map<std::string,SpeciesState*>::iterator itB = m_SpeciesStateBySpeciesName.find(in_SpeciesName);
   if(itB == m_SpeciesStateBySpeciesName.end())
   {
      m_SpeciesStateBySpeciesName.insert(std::make_pair(in_SpeciesName,theSpeciesState));
   }
   else
   {
     //throw a warning
   }
}
double SimulatorState::GetSpeciesConcentrationByName(const std::string &in_SpeciesName) const
{
   std::map<std::string, SpeciesState*>::const_iterator it;

   return (it = m_SpeciesStateBySpeciesName.find(in_SpeciesName)) == m_SpeciesStateBySpeciesName.end() ? 0.0 : it->second->GetConcentration();
}
SpeciesState *SimulatorState::GetSpeciesStateByName(const std::string &in_SpeciesName)
{
    SpeciesState *ret=0;
    std::map<std::string, SpeciesState*>::iterator it = m_SpeciesStateBySpeciesName.find(in_SpeciesName);
    if(it!=m_SpeciesStateBySpeciesName.end())
    {
      ret=it->second;
    }
    return ret;
}
void SimulatorState::GetSpeciesStateConcentrations(std::map<string, double> &currentSpeciesConcs) const
{
   currentSpeciesConcs.clear();
   typedef std::map<std::string, SpeciesState*>::const_iterator cIt;
   for(cIt it = m_SpeciesStateBySpeciesName.begin(), itEnd = m_SpeciesStateBySpeciesName.end(); it != itEnd; ++it)
   {
      const std::pair<const std::string, OTGC::SpeciesState*> & thePair = (*it);
      currentSpeciesConcs.insert(make_pair(thePair.first,thePair.second->GetConcentration()));
   }
}

}
