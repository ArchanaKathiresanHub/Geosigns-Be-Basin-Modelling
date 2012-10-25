#include "stdafx.h"

#include "ComponentManager.h"

namespace CBMGenerics
{ 

ComponentManager::ComponentManager():m_speciesOutputProperty("ExpelledCumulative")
{
   int i;

   const std::string tempSpeciesNames[] =
   {
      "asphaltenes", "resins", "C15+Aro", "C15+Sat", "C6-14Aro", "C6-14Sat", 
      "C5", "C4", "C3", "C2","C1","COx", "N2", "H2S",
      "LSC", "C15+AT", "C6-14BT", "C6-14DBT", "C6-14BP", "C15+AroS", "C15+SatS", "C6-14SatS", "C6-14AroS",
      ""
   };
   for(i = 0; tempSpeciesNames[i] != ""; ++i)   
   {
       m_SpeciesNames[i] = tempSpeciesNames[i]; 
       m_SpeciesIdByName[tempSpeciesNames[i]] = i;
   } 

   const std::string phaseNames[] = 
   {
      "Vapour", "Liquid", ""
   };

   for(i = 0; phaseNames[i] != ""; ++i)   
   {
       m_phaseNames[i] = phaseNames[i]; 
       m_phaseIdByName[phaseNames[i]] = i;
   } 

}

ComponentManager::~ComponentManager()
{
    m_SpeciesIdByName.clear();
}

ComponentManager & ComponentManager::getInstance()
{
   static ComponentManager theManager;
   return theManager;
}

int ComponentManager::GetSpeciedIdByName(const std::string &name)
{
   return GetSpeciesIdByName(name);
}

int ComponentManager::GetSpeciesIdByName(const std::string &name)
{
   int ret = -1;
   
   std::map<std::string, int>::iterator it = m_SpeciesIdByName.find(name);
   if( it != m_SpeciesIdByName.end())
   {
       ret = it->second;
   }
   return ret;
}

int ComponentManager::GetPhaseIdByName(const std::string &name)
{
   int ret = -1;
   std::map<std::string, int>::iterator it = m_phaseIdByName.find(name);
   if( it != m_phaseIdByName.end())
   {
       ret = it->second;
   }
   return ret;
}

const std::string& ComponentManager::GetSpeciesName(int speciesIndex) 
{
   static std::string def = "";
   return speciesIndex >= 0 && speciesIndex < NumberOfOutputSpecies ? m_SpeciesNames[speciesIndex] : def;
}

const std::string& ComponentManager::GetPhaseName(int phaseIndex) 
{
   static std::string def = "";
   return phaseIndex >= 0 && phaseIndex < NumberOfPhases ? m_phaseNames[phaseIndex] : def;
}

std::string ComponentManager::GetSpeciesOutputPropertyName ( const int  speciesIndex,
                                                             const bool addSRExtension ) {

   std::string speciesProp = GetSpeciesName(speciesIndex);

   if ( speciesProp != "" ) {

      if ( addSRExtension ) {
         speciesProp += "ExpelledCumulativeSR";
      } else {
         speciesProp += "ExpelledCumulative";
      }

   }

   return speciesProp;
}

std::string ComponentManager::getSpeciesSourceRockExpelledByName ( const int speciesIndex ) {

   std::string speciesProp = GetSpeciesName(speciesIndex);
   speciesProp += "ExpelledCumulative";

   return speciesProp;
}

}

