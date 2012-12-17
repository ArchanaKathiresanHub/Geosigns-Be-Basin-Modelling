#ifndef COMPONENTMANAGER_H
#define COMPONENTMANAGER_H

#if defined(_WIN32) || defined (_WIN64)	
#define or ||
#define not !
#define and &&
#endif

#include "DllExport.h"

#include <string>
#include <map>

namespace CBMGenerics
{
   const double undefined = 99999;

   class CBMGENERICS_DLL_EXPORT ComponentManager
   {
   public:
      enum SpeciesNamesId
      {
         UNKNOWN = -1,
         asphaltene = 0, 
         resin = 1, 
         C15PlusAro = 2, 
         C15PlusSat = 3, 
         C6Minus14Aro = 4, 
         C6Minus14Sat = 5, 
         C5 = 6, 
         C4 = 7, 
         C3 = 8, 
         C2 = 9, 
         C1 = 10, 
         COx = 11, 
         N2  = 12, 
         H2S = 13,
         LSC = 14, 
         
         NumberOfSpeciesToFlash = 14, 
         
         C15PlusAT = 15, 
         C6Minus14BT = 16, 
         C6Minus14DBT = 17, 
         C6Minus14BP = 18, 
         C15PlusAroS = 19, 
         C15PlusSatS = 20, 
         C6Minus14SatS = 21, 
         C6Minus14AroS = 22, 

         NumberOfOutputSpecies = 23, 
         NumberOfSpecies = 23
      };

      enum PhaseId
      {
         Gas = 0, Vapour = 0, 
         Oil = 1, Liquid = 1, 
         NumberOfPhases = 2
      };

      static double MolecularWeight[NumberOfSpecies];

      const std::string GetSpeciesName( int speciesIndex ) const;
      const std::string GetPhaseName( int phaseIndex ) const;

      /// \brief Return the name of the species.
      ///
      /// It may or may not have an extension SR.
      /// The SR extension indicates that the name represents a species that
      /// has been expelled from the kerogen into the source-rock pore-space.
      /// Without this extension the name represents a species that has been 
      /// expelled into the carrier-bed.
      std::string GetSpeciesOutputPropertyName( const int speciesIndex, const bool addSRExtension = false ) const;

      /// \brief Return the name of the species expelled from the source-rock.
      std::string getSpeciesSourceRockExpelledByName( const int speciesIndex ) const;

      /// \brief Simple check to determine if the id is in the range [C1, C5]
      inline bool isGas( const SpeciesNamesId id ) const;

      /// \brief Simple check to determine if the id is in the range [C6, asphaltene].
      ///
      /// i.e. not isGas.
      inline bool isOil( const SpeciesNamesId id ) const;

      int GetSpeciesIdByName( const std::string& name ) const;
      int GetSpeciedIdByName( const std::string& name ) const ;

      int GetPhaseIdByName( const std::string& name ) const;
      
      inline bool isSulphurComponent( const int id ) const;
      inline bool isSbearingHCsComponent( const int id ) const;

      static ComponentManager& getInstance();

      ~ComponentManager() {;}

   private:
      static const char *        s_SpeciesNames[NumberOfOutputSpecies];
      std::map<std::string, int> m_SpeciesIdByName;

      int                        m_numberOfPhases;
      static const char *        s_phaseNames[NumberOfPhases];
      std::map<std::string, int> m_phaseIdByName;

      static const char *        s_speciesOutputProperty;
      
      ComponentManager();
      ComponentManager(const ComponentManager& in_Frm);
      ComponentManager& operator = (const ComponentManager& in_Frm);
      
   };

   inline bool ComponentManager::isGas( const SpeciesNamesId id ) const
   {
      return id == C1 or id == C2 or id == C3 or id == C4 or id == C5;
   }

   inline bool ComponentManager::isOil( const SpeciesNamesId id ) const 
   {
      return not isGas ( id ) and id != UNKNOWN and id != NumberOfSpecies;
   }

   inline bool ComponentManager::isSulphurComponent( const int id ) const 
   {
      return id >= H2S and id < NumberOfSpecies;
   }

   inline bool ComponentManager::isSbearingHCsComponent( const int id ) const 
   {
      return id != C6Minus14BP and id > H2S and id < NumberOfSpecies;
   }
}
#endif
