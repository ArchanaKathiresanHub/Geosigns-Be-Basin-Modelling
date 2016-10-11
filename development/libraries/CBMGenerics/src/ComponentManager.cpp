//
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

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

std::string CBMGenerics::ComponentManager::GetSpeciesName( int speciesIndex )
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

