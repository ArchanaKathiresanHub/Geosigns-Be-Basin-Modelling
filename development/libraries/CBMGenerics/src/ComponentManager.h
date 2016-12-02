//
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef COMPONENTMANAGER_H
#define COMPONENTMANAGER_H

// std library
#include <string>
#include <map>

namespace CBMGenerics
{

   /// \class ComponentManager Contains the names and enumerations of the hydrocarbone phases and species
   class ComponentManager
   {
   public:
      /// \brief The hydrocarbone species
      enum SpeciesNamesId
      {
         UNKNOWN         = -1,
         
         FIRST_COMPONENT = 0,
         
         ASPHALTENE      = 0,
         RESIN           = 1,
         C15_PLUS_ARO    = 2,
         C15_PLUS_SAT    = 3,
         C6_MINUS_14ARO  = 4,
         C6_MINUS_14SAT  = 5,
         C5              = 6,
         C4              = 7,
         C3              = 8,
         C2              = 9,
         C1              = 10,
         COX             = 11,
         N2              = 12,
         H2S             = 13,
         LSC             = 14,

         NUMBER_OF_SPECIES_TO_FLASH = 14, 

         C15_PLUS_AT       = 15,
         C6_MINUS_14BT     = 16,
         C6_MINUS_14DBT    = 17,
         C6_MINUS_14BP     = 18,
         C15_PLUS_ARO_S    = 19,
         C15_PLUS_SAT_S    = 20,
         C6_MINUS_14SAT_S  = 21,
         C6_MINUS_14ARO_S  = 22,
         
         LAST_COMPONENT    = 22,

         NUMBER_OF_SPECIES = 23
      };

      /// \brief The hydrocarbone phases
      enum PhaseId
      {
         GAS = 0, VAPOUR = 0,
         OIL = 1, LIQUID = 1,
         NUMBER_OF_PHASES = 2
      };

      /// \return The array containing all the species output name
      const char ** getSpeciesNameList()      const { return s_SpeciesNames;      };
      /// \return The array containing all the species input name
      const char ** getSpeciesNameInputList() const { return s_SpeciesNamesInput; };

      /// \return The output name of the hydrocarbone specie as a string
      const std::string getSpeciesName( int speciesIndex ) const;
      /// \return The input name of the hydrocarbone specie as a string
      const std::string getSpeciesInputName( int speciesIndex ) const;
      /// \return The history output name of the hydrocarbone specie as a string
      const std::string getSpeciesNameHistory( int speciesIndex ) const;
      /// \return The phase as a string (Liquid or Vapour)
      const std::string getPhaseName  ( int phaseIndex   ) const;

      /// \brief Return the name of the species.
      ///
      /// It may or may not have an extension SR.
      /// The SR extension indicates that the name represents a species that
      /// has been expelled from the kerogen into the source-rock pore-space.
      /// Without this extension the name represents a species that has been 
      /// expelled into the carrier-bed.
      std::string getSpeciesOutputPropertyName( const int speciesIndex, const bool addSRExtension = false ) const;

      /// \brief Return the name of the species expelled from the source-rock.
      std::string getSpeciesSourceRockExpelledByName( const int speciesIndex ) const;

      /// \brief Simple check to determine if the id is in the range [C1, C5]
      inline bool isGas( const SpeciesNamesId id ) const;

      /// \brief Simple check to determine if the id is in the range [C6, asphaltene].
      ///
      /// i.e. not isGas.
      inline bool isOil( const SpeciesNamesId id ) const;

      int getSpeciesIdByName( const std::string& name ) const;
      int getPhaseIdByName  ( const std::string& name ) const;
      
      inline bool isSulphurComponent    ( const int id ) const;
      inline bool isSbearingHCsComponent( const int id ) const;

      /// \returns The component manager static object
      static ComponentManager& getInstance();

      ~ComponentManager() {;}

   private:
      /// @todo All this names could be the same but it requires to update the database/UI
      static const char *       s_SpeciesNames       [NUMBER_OF_SPECIES]; ///< Output names of the hydrocarbone species
      static const char *       s_SpeciesNamesInput  [NUMBER_OF_SPECIES]; ///< Input names of the hydrocarbone species
      static const std::string  s_SpeciesNamesHistory[NUMBER_OF_SPECIES]; ///< Output names of the hydrocarbone species for history files (*.dat)
      std::map<std::string, int> m_SpeciesIdByName;                       ///< Mapping between the hydrocarbone species output (not history) names and their IDs

      static const char *        s_phaseNames[NUMBER_OF_PHASES]; ///< Names of the hydrocarbone phases
      std::map<std::string, int> m_phaseIdByName;                ///< Mapping between the hydrocarbone phases names and their IDs

      static const char *        s_speciesOutputProperty; ///< Suffix used for the hydrocarbone species output maps

      ComponentManager();                                            ///< Private constructor
      ComponentManager(const ComponentManager& in_Frm);              ///< Private copy constructor
      ComponentManager& operator = (const ComponentManager& in_Frm); ///< Private assignement opperator

   };

   inline bool ComponentManager::isGas( const SpeciesNamesId id ) const
   {
      return id == C1 or id == C2 or id == C3 or id == C4 or id == C5;
   }

   inline bool ComponentManager::isOil( const SpeciesNamesId id ) const 
   {
      return not isGas ( id ) and id != UNKNOWN and id != NUMBER_OF_SPECIES;
   }

   inline bool ComponentManager::isSulphurComponent( const int id ) const 
   {
      return id >= H2S and id < NUMBER_OF_SPECIES;
   }

   inline bool ComponentManager::isSbearingHCsComponent( const int id ) const 
   {
      return id != C6_MINUS_14BP and id > H2S and id < NUMBER_OF_SPECIES;
   }

}
#endif
