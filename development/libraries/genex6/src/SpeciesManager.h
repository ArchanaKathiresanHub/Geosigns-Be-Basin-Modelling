#ifndef _GENEX6__SPECIES_MANAGER_H_
#define _GENEX6__SPECIES_MANAGER_H_

#include <iostream>
#include <string>

#include "EosPack.h"
#include "ComponentManager.h"

#include "NumericFunctions.h"

namespace Genex6
{

   /// \brief Manages the species-identifiers.
   ///
   /// It is possible to have more than one set of identifiers in use at any one time.
   /// E.g. with using the OTGC within the shale-gas simulator.
   class SpeciesManager {

   public:

      /// \brief The maximum number of elements.
      static const int numberOfElements = 5;

      /// \brief The maximum number of species.
      static const int numberOfSpecies = 36;
   

      /// \brief Constructor.
      SpeciesManager ();


      /// \name Species identifiers.
      //@{

      /// \brief Get the id of Kerogen species.
      int getKerogenId () const;

      /// \brief Get the id of Pre-asphalt species.
      int getPreasphaltId () const;

      /// \brief Get the id of Asphaltene species.
      int getAsphaltenesId () const;

      /// \brief Get the id of Resin species.
      int getResinsId () const;

      /// \brief Get the id of Pre-coke species.
      int getPrecokeId () const;

      /// \brief Get the id of Coke-1 species.
      int getCoke1Id () const;

      /// \brief Get the id of Coke-2 species.
      int getCoke2Id () const;

      /// \brief Get the id of species aromatic C15 and greater.
      int getC15plusAroId () const;

      /// \brief Get the id of species saturate C15 and greater.
      int getC15plusSatId () const;

      /// \brief Get the id of aromatic C6 to C14 species.
      int getC6to14AroId () const;

      /// \brief Get the id of saturate C6 to C14 species.
      int getC6to14SatId () const;

      /// \brief Get the id of Hetero species.
      int getHetero1Id () const;

      /// \brief Get the id of C5 species.
      int getC5Id () const;

      /// \brief Get the id of C4 species.
      int getC4Id () const;

      /// \brief Get the id of C3 species.
      int getC3Id () const;

      /// \brief Get the id of C2 species.
      int getC2Id () const;

      /// \brief Get the id of C1 species.
      int getC1Id () const;

      /// \brief Get the id of species aromatic C15 with Sulphur and greater.
      int getC15plusAroSId () const;

      /// \brief Get the id of species saturate C15 with Sulphur and greater.
      int getC15plusSatSId () const;

      /// \brief Get the id of LSC species.
      int getLSCId () const;

      /// \brief Get the id of C15 and AT species.
      int getC15plusATId () const; 

      /// \brief Get the id of C6 to C14 with BT species.
      int getC6to14BTId () const;

      /// \brief Get the id of C6 to C14 with DBT species.
      int getC6to14DBTId () const;

      /// \brief Get the id of C6 to C14 with BP species.
      int getC6to14BPId () const;

      /// \brief Get the id of aromatic C6 to C14 with Sulphur species.
      int getC6to14AroSId () const;

      /// \brief Get the id of saturate C6 to C14 with Sulphur species.
      int getC6to14SatSId () const;

      /// \brief Get the id of Coke with Sulphur species.
      int getCokeSId () const;

      /// \brief Get the id of SO4 species.
      int getSO4Id () const;

      /// \brief Get the id of N2 species.
      int getN2Id () const;

      /// \brief Get the id of COx species.
      int getCOxId () const;
      //@}


      /// \name Element identifiers.
      //@{

      /// \brief Get the id of elemental Carbon.
      int getCarbonId () const;

      /// \brief Get the id of elemental Hydrogen.
      int getHydrogenId () const;

      /// \brief Get the id of elemental Oxygen.
      int getOxygenId () const;

      /// \brief Get the id of elemental Nitrogen.
      int getNitrogenId () const;

      /// \brief Get the id of elemental Sulphur.
      int getSulphurId () const;
      //@}


      /// \brief Get the id based on the element-name.
      int GetElementIdByName ( const std::string& elementName ) const;

      /// \brief Get the id based on the species-name.
      int getSpeciesIdByName ( const std::string& elementName ) const;

      /// \brief Set the species-id from the name.
      void setSpeciesNum ( const std::string& speciesName, int id );

      /// \brief Set the element-id from the name.
      void setElementNum ( const std::string& elementName, int id );


      /// \name Mapping between identifier sets.
      //@{

      /// \brief Maps the species-manager-id to the pvt-flash::component-id.
      ///
      /// id must be in range 1 .. speciesManager.numberOfSpecies.
      /// If there is no corresponding pvt-species-id then pvtFlash::UNKNOWN will be returned.
      pvtFlash::ComponentId mapIdToPvtComponents ( const int id ) const;

      /// \brief Maps the pvt-flash::component-id to the species-manager-id.
      int mapPvtComponentsToId ( const pvtFlash::ComponentId component ) const;

      /// \brief Maps the species-manager-id to the component-manager::species-names-id.
      ///
      /// id must be in range 1 .. speciesManager.numberOfSpecies.
      /// If there is no corresponding component-manager-species-id then ComponentManager::UNKNOWN will be returned.
      CBMGenerics::ComponentManager::SpeciesNamesId mapIdToComponentManagerSpecies ( const int id ) const;

      /// \brief Maps the component-manager::species-names-id to the species-manager-id.
      int mapComponentManagerSpeciesToId ( const CBMGenerics::ComponentManager::SpeciesNamesId species ) const;

      //@}

      /// \brief The id of the first species.
      ///
      /// any species-id will be in the closed interval [ first-species .. last-species ]
      int firstSpecies () const;

      /// \brief The id of the last species.
      ///
      /// This will equal the number of species.
      /// any species-id will be in the closed interval [ first-species .. last-species ]
      int lastSpecies () const;

      /// \brief Return the number of species.
      int getNumberOfSpecies () const;

      /// \brief Return the number of elements.
      int getNumberOfElements () const;

      /// \brief Return the name of the species with the id.
      const std::string& getSpeciesName ( const int id ) const;

      /// \brief Print state of species-manager to screen.
      void print ( std::ostream& stream = std::cout ) const;

      /// \brief Return the Sulphur allocated
      bool isSulphur() const;

   private :


      // Id's for atoms.
      int IatomC;
      int IatomH;
      int IatomO;
      int IatomN;
      int IatomS;
   
      // Id's for species.
      int Lkerogen;
      int Lpreasphalt;
      int Lasphaltenes;
      int Lresins;
      int Lprecoke;
      int Lcoke1;
      int Lcoke2;
      int LC15plusAro;
      int LC15plusSat;
      int LC6to14Sat;
      int LC6to14Aro;
      int Lhetero1;
      int LC5;
      int LC4;
      int LC3;
      int LC2;
      int LC1;
      int LC15plusAroS;
      int LC15plusSatS;
      int LLSC;
      int LC15plusAT; 
      int LC6to14BT;
      int LC6to14DBT;
      int LC6to14BP;
      int LC6to14AroS;
      int LC6to14SatS;
      int LcokeS;
      int LSO4;
      int LN2;
      int LCOx;


      /// \brief Mapping from the species-id to pvt-species-id.
      ///
      /// If there is no corresponding pvt-species-id then UNKNOWN will be used.
      pvtFlash::ComponentId s_mappingToPvtComponents [ numberOfSpecies + 1 ];

      int s_mappingFromPvtComponents [ pvtFlash::NUM_COMPONENTS ];

      /// \brief Mapping from the species-id to component-manager-species-id.
      ///
      /// If there is no corresponding component-manager-species-id then UNKNOWN will be used.
      CBMGenerics::ComponentManager::SpeciesNamesId s_mappingToComponentManagerSpecies [ numberOfSpecies + 1 ];

      /// \brief Mapping from the component-manager-species-id to species-id.
      int s_mappingFromComponentManagerSpecies [ CBMGenerics::ComponentManager::NumberOfSpecies ];

      std::string s_idNames [ numberOfSpecies + 2 ];

   };

}

// Inline functions.

inline int Genex6::SpeciesManager::firstSpecies () const {
   return 1;
}

inline int Genex6::SpeciesManager::lastSpecies () const {
   return numberOfSpecies;
}

inline int Genex6::SpeciesManager::getNumberOfSpecies () const {
   return numberOfSpecies;
}

inline int Genex6::SpeciesManager::getNumberOfElements () const {
   return numberOfElements;
}


inline pvtFlash::ComponentId Genex6::SpeciesManager::mapIdToPvtComponents ( const int id ) const {

   if ( NumericFunctions::inRange ( id, 1, getNumberOfSpecies ())) {
      return s_mappingToPvtComponents [ id - 0 ];
   } else {
      return pvtFlash::UNKNOWN;
   }

}

inline int Genex6::SpeciesManager::mapPvtComponentsToId ( const pvtFlash::ComponentId component ) const {

   if ( component != pvtFlash::UNKNOWN ) {
      return s_mappingFromPvtComponents [ static_cast<int>(component)];
   } else {
      return -1;
   }

}

inline CBMGenerics::ComponentManager::SpeciesNamesId Genex6::SpeciesManager::mapIdToComponentManagerSpecies ( const int id ) const {

   if ( NumericFunctions::inRange ( id, 1, getNumberOfSpecies ())) {
      return s_mappingToComponentManagerSpecies [ id - 0 ];
   } else {
      return CBMGenerics::ComponentManager::UNKNOWN;
   }

}

inline int Genex6::SpeciesManager::mapComponentManagerSpeciesToId ( const CBMGenerics::ComponentManager::SpeciesNamesId species ) const {

   if ( species != CBMGenerics::ComponentManager::UNKNOWN ) {
      return s_mappingFromComponentManagerSpecies [ static_cast<int>(species) ];
   } else {
      return -1;
   }

}

//------------------------------------------------------------//

inline int Genex6::SpeciesManager::getCarbonId () const {
   return IatomC;
}

inline int Genex6::SpeciesManager::getHydrogenId () const {
   return IatomH;
}

inline int Genex6::SpeciesManager::getOxygenId () const {
   return IatomO;
}

inline int Genex6::SpeciesManager::getNitrogenId () const {
   return IatomN;
}

inline int Genex6::SpeciesManager::getSulphurId () const {
   return IatomS;
}

//------------------------------------------------------------//

inline int Genex6::SpeciesManager::getKerogenId () const {
   return Lkerogen;
}

inline int Genex6::SpeciesManager::getPreasphaltId () const {
   return Lpreasphalt;
}

inline int Genex6::SpeciesManager::getAsphaltenesId () const {
   return Lasphaltenes;
}

inline int Genex6::SpeciesManager::getResinsId () const {
   return Lresins;
}

inline int Genex6::SpeciesManager::getPrecokeId () const {
   return Lprecoke;
}

inline int Genex6::SpeciesManager::getCoke1Id () const {
   return Lcoke1;
}

inline int Genex6::SpeciesManager::getCoke2Id () const {
   return Lcoke2;
}

inline int Genex6::SpeciesManager::getC15plusAroId () const {
   return LC15plusAro;
}

inline int Genex6::SpeciesManager::getC15plusSatId () const {
   return LC15plusSat;
}

inline int Genex6::SpeciesManager::getC6to14SatId () const {
   return LC6to14Sat;
}

inline int Genex6::SpeciesManager::getC6to14AroId () const {
   return LC6to14Aro;
}

inline int Genex6::SpeciesManager::getHetero1Id () const {
   return Lhetero1;
}

inline int Genex6::SpeciesManager::getC5Id () const {
   return LC5;
}

inline int Genex6::SpeciesManager::getC4Id () const {
   return LC4;
}

inline int Genex6::SpeciesManager::getC3Id () const {
   return LC3;
}

inline int Genex6::SpeciesManager::getC2Id () const {
   return LC2;
}

inline int Genex6::SpeciesManager::getC1Id () const {
   return LC1;
}

inline int Genex6::SpeciesManager::getN2Id () const {
   return LN2;
}

inline int Genex6::SpeciesManager::getCOxId () const {
   return LCOx;
}

inline int Genex6::SpeciesManager::getC15plusAroSId () const {
   return LC15plusAroS;
}

inline int Genex6::SpeciesManager::getC15plusSatSId () const {
   return LC15plusSatS;
}

inline int Genex6::SpeciesManager::getLSCId () const {
   return LLSC;
}

inline int Genex6::SpeciesManager::getC15plusATId () const {
   return LC15plusAT; 
}

inline int Genex6::SpeciesManager::getC6to14BTId () const {
   return LC6to14BT;
}

inline int Genex6::SpeciesManager::getC6to14DBTId () const {
   return LC6to14DBT;
}

inline int Genex6::SpeciesManager::getC6to14BPId () const {
   return LC6to14BP;
}

inline int Genex6::SpeciesManager::getC6to14AroSId () const {
   return LC6to14AroS;
}

inline int Genex6::SpeciesManager::getC6to14SatSId () const {
   return LC6to14SatS;
}

inline int Genex6::SpeciesManager::getCokeSId () const {
   return LcokeS;
}

inline int Genex6::SpeciesManager::getSO4Id () const {
   return LSO4;
}

inline const std::string& Genex6::SpeciesManager::getSpeciesName ( const int id ) const {

   if ( NumericFunctions::inRange ( id, 1, getNumberOfSpecies ())) {
      return s_idNames [ id ];
   } else {
      return s_idNames [ getNumberOfSpecies () + 1 ];
   }

}


//------------------------------------------------------------//

#endif // _GENEX6__SPECIES_MANAGER_H_
