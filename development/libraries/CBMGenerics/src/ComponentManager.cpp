//
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "ComponentManager.h"

const char * CBMGenerics::ComponentManager::s_SpeciesNames[NUMBER_OF_SPECIES] = 
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

const char * CBMGenerics::ComponentManager::s_SpeciesNamesInput[NUMBER_OF_SPECIES] =
{
   "asphaltenes",
   "resins",
   "C15Aro",
   "C15Sat",
   "C6_14Aro",
   "C6_14Sat",
   "C5",
   "C4",
   "C3",
   "C2",
   "C1",
   "COx",
   "N2",
   "H2S",
   "LSC",
   "C15AT",
   "C6_14BT",
   "C6_14DBT",
   "C6_14BP",
   "C15AroS",
   "C15SatS",
   "C6_14SatS",
   "C6_14AroS"
};

const std::string CBMGenerics::ComponentManager::s_SpeciesNamesHistory[NUMBER_OF_SPECIES] =
{
   "ASPHALTENES",
   "RESINS",
   "C15_ARO",
   "C15_SAT",
   "C6_14ARO",
   "C6_14SAT",
   "C5",
   "C4",
   "C3",
   "C2",
   "C1",
   "COX",
   "N2",
   "H2S",
   "LSC",
   "C15_AT",
   "C6_14BT",
   "C6_14DBT",
   "C6_14BP",
   "C15_AROS",
   "C15_SATS",
   "C6_14SATS",
   "C6_14AROS"
};

const char * CBMGenerics::ComponentManager::s_phaseNames[NUMBER_OF_PHASES] =
{
   "Vapour",
   "Liquid"
};

const char * CBMGenerics::ComponentManager::s_speciesOutputProperty = "ExpelledCumulative";

CBMGenerics::ComponentManager::ComponentManager()
{
   for ( int i = 0; i < NUMBER_OF_SPECIES; ++i )
   {
       m_SpeciesIdByName[s_SpeciesNames[i]] = i;
   } 

   for ( int i = 0; i < NUMBER_OF_PHASES; ++i )
   {
       m_phaseIdByName[s_phaseNames[i]] = i;
   } 
}

CBMGenerics::ComponentManager & CBMGenerics::ComponentManager::getInstance()
{
   static ComponentManager theManager;
   return theManager;
}

int CBMGenerics::ComponentManager::getSpeciesIdByName( const std::string& name ) const
{
   std::map<std::string, int>::const_iterator it = m_SpeciesIdByName.find( name );
   return it != m_SpeciesIdByName.end() ? it->second : -1;
}

int CBMGenerics::ComponentManager::getPhaseIdByName( const std::string& name ) const
{
   std::map<std::string, int>::const_iterator it = m_phaseIdByName.find( name );
   return it != m_phaseIdByName.end() ? it->second : -1;
}

const std::string CBMGenerics::ComponentManager::getSpeciesName( int speciesIndex ) const
{
   static std::string def = "";
   return speciesIndex >= 0 && speciesIndex < NUMBER_OF_SPECIES ? s_SpeciesNames[speciesIndex] : def;
}

const std::string CBMGenerics::ComponentManager::getSpeciesInputName( int speciesIndex ) const {
   static std::string def = "";
   return speciesIndex >= 0 && speciesIndex < NUMBER_OF_SPECIES ? s_SpeciesNamesInput[speciesIndex] : def;
}

const std::string CBMGenerics::ComponentManager::getSpeciesNameHistory( int speciesIndex ) const
{
   static std::string def = "";
   return speciesIndex >= 0 && speciesIndex < NUMBER_OF_SPECIES ? s_SpeciesNamesHistory[speciesIndex] : def;
}

const std::string CBMGenerics::ComponentManager::getPhaseName( int phaseIndex ) const
{
   static std::string def = "";
   return phaseIndex >= 0 && phaseIndex < NUMBER_OF_PHASES ? s_phaseNames[phaseIndex] : def;
}

std::string CBMGenerics::ComponentManager::getSpeciesOutputPropertyName( const int speciesIndex, const bool addSRExtension ) const
{
   std::string speciesProp = getSpeciesName( speciesIndex );

   if ( speciesProp != "" )
   {
      speciesProp += addSRExtension ? (std::string( s_speciesOutputProperty ) + "SR") : s_speciesOutputProperty;
   }

   return speciesProp;
}

std::string CBMGenerics::ComponentManager::getSpeciesSourceRockExpelledByName( const int speciesIndex ) const
{
   return getSpeciesName( speciesIndex ) + s_speciesOutputProperty;
}

