#include "stdafx.h"
#include "ComponentManager.h"

const char * CBMGenerics::ComponentManager::s_SpeciesNames[NumberOfOutputSpecies] = 
{
   "asphaltenes", 
   "resins", 
   "C15+Aro", 
   "C15+Sat", 
   "C6-14Aro", 
   "C6-14Sat", 
   "C5", 
   "C4", 
   "C3", 
   "C2",
   "C1",
   "COx", 
   "N2", 
   "H2S",
   "LSC",
   "C15+AT",
   "C6-14BT",
   "C6-14DBT",
   "C6-14BP",
   "C15+AroS",
   "C15+SatS",
   "C6-14SatS",
   "C6-14AroS"
};

double CBMGenerics::ComponentManager::MolecularWeight[NumberOfSpecies] =
{
   795.2186697,      //Asphaltenes
   618.07398,        //Resins
   474.1257111,      //C15+ Aro
   281.2408344,      //C15+ Sat
   158.3971864,      //C6-14 Aro
   103.0699011,      //C6-14 Sat
   72.15064,         //C5
   58.1237,          //C4
   44.09676,         //C3
   30.06982,         //C2
   16.04288,         //C1
   44.0098,          //COx
   28.01352,         //N2
   34.08,            //H2S
   281.2408344,      //LSC
   281.2408344,      //C15+ AT
   158.3971864,      //C6-14 BT
   158.3971864,      //C6-14 DBT
   158.3971864,      //C6-14 BP
   281.2408344,      //C15+ AroS
   281.2408344,      //C15+ SatS
   158.3971864,      //C6-14 SatS
   158.3971864      //C6-14 AroS
};


const char * CBMGenerics::ComponentManager::s_phaseNames[NumberOfPhases] = { "Vapour", "Liquid" };
const char * CBMGenerics::ComponentManager::s_speciesOutputProperty = "ExpelledCumulative";

CBMGenerics::ComponentManager::ComponentManager()
{
   for ( int i = 0; i < NumberOfOutputSpecies; ++i )
   {
       m_SpeciesIdByName[s_SpeciesNames[i]] = i;
   } 

   for ( int i = 0; i < NumberOfPhases; ++i )
   {
       m_phaseIdByName[s_phaseNames[i]] = i;
   } 
}

CBMGenerics::ComponentManager & CBMGenerics::ComponentManager::getInstance()
{
   static ComponentManager theManager;
   return theManager;
}

int CBMGenerics::ComponentManager::GetSpeciedIdByName( const std::string& name ) const
{
   return GetSpeciesIdByName(name);
}

int CBMGenerics::ComponentManager::GetSpeciesIdByName( const std::string& name ) const
{
   std::map<std::string, int>::const_iterator it = m_SpeciesIdByName.find( name );
   return it != m_SpeciesIdByName.end() ? it->second : -1;
}

int CBMGenerics::ComponentManager::GetPhaseIdByName( const std::string& name ) const
{
   std::map<std::string, int>::const_iterator it = m_phaseIdByName.find( name );
   return it != m_phaseIdByName.end() ? it->second : -1;
}

const std::string CBMGenerics::ComponentManager::GetSpeciesName( int speciesIndex ) const
{
   static std::string def = "";
   return speciesIndex >= 0 && speciesIndex < NumberOfOutputSpecies ? s_SpeciesNames[speciesIndex] : def;
}

const std::string CBMGenerics::ComponentManager::GetPhaseName( int phaseIndex ) const
{
   static std::string def = "";
   return phaseIndex >= 0 && phaseIndex < NumberOfPhases ? s_phaseNames[phaseIndex] : def;
}

std::string CBMGenerics::ComponentManager::GetSpeciesOutputPropertyName( const int speciesIndex, const bool addSRExtension ) const
{
   std::string speciesProp = GetSpeciesName( speciesIndex );

   if ( speciesProp != "" )
   {
      speciesProp += addSRExtension ? (std::string( s_speciesOutputProperty ) + "SR") : s_speciesOutputProperty;
   }

   return speciesProp;
}

std::string CBMGenerics::ComponentManager::getSpeciesSourceRockExpelledByName( const int speciesIndex ) const
{
   return GetSpeciesName( speciesIndex ) + s_speciesOutputProperty;
}

