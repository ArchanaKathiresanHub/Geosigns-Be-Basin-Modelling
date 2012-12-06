#ifndef _FASTCAULDRON__IMMOBILE_SPECIES_VALUES_H_
#define _FASTCAULDRON__IMMOBILE_SPECIES_VALUES_H_

#include "SpeciesManager.h"

//using namespace Genex6;

#include <string>

/// \brief Holds concentrations of immobile species.
//
// This class could be generalised some what.
class ImmobileSpeciesValues {

public :

   /// \brief The name of the immobile species.
   enum SpeciesId { PRECOKE, COKE1, HETERO1, COKE2, COKES, UNKNOWN = 5, NUM_IMMOBILES = 5 };


   static const int NumberOfImmobileSpecies = NUM_IMMOBILES;

   static const int BlockSize = NumberOfImmobileSpecies;

   typedef SpeciesId IndexType;


   /// \brief Constructor.
   ImmobileSpeciesValues ();

   /// \brief Return the value for the species-id.
   double operator ()( const SpeciesId id ) const;

   /// \brief Return a reference to the value for the species-id.
   double& operator ()( const SpeciesId id );


   /// \brief Get the retained concentration of the species.
   double getRetained ( const SpeciesId id ) const;

   /// \brief Set the retained concentration of the species.
   void setRetained ( const SpeciesId id,
                            double    retained );

   /// Returns the total immobile retained volume.
   ///
   /// Each retained-immobile-species is divided by its corresponding density.
   double getRetainedVolume () const;

   /// \brief Return the sum of values for all the immobile species.
   double sum () const;

   /// \brief Return a string representation of the object.
   std::string image () const;

   /// \brief Get the density of the immobile species.
   static double getDensity ( const SpeciesId id );

   /// \brief Set the density of the immobile species.
   static void setDensity ( const SpeciesId id, 
                            const double    density );

   /// \brief Get the specied-id from the string representation.
   ///
   /// If the name cannot be found then UNKNOWN will be returned.
   /// The comparison is not case-sensitive.
   static SpeciesId getId ( const std::string& name );

   /// \brief Return the string for the species-id.
   static const std::string& getName ( const SpeciesId id );

   /// \brief Return the correspondent SpeciesManager id.
   static int getSpeciesManagerId(  const SpeciesId id );

   /// \brief Set the mapping to SpeciesManager
   static void setMappingToSpeciesManager ( const Genex6::SpeciesManager & species );
private :

   /// \brief Array of immobile-species names in lower case.
   static std::string s_namesLc [ NUM_IMMOBILES ];

   /// \brief Array of immobile-species names.
   static std::string s_names   [ NUM_IMMOBILES + 1 ];

   /// \brief Array if species densities.
   static double s_densities [ NUM_IMMOBILES ];

   /// \brief Array for mapping immobile species to Species Manager id.
   static double s_mappingToSpeciesManager [ NUM_IMMOBILES ];

   double m_retained [ NUM_IMMOBILES ];

};


inline double ImmobileSpeciesValues::operator ()( const SpeciesId id ) const {
   return m_retained [ id ];
}

inline double& ImmobileSpeciesValues::operator ()( const SpeciesId id ) {
   return m_retained [ id ];
}

inline double ImmobileSpeciesValues::sum () const {
   return m_retained [ PRECOKE ] + m_retained [ COKE1 ] + m_retained [ HETERO1 ] + m_retained [ COKE2 ] + m_retained [ COKES ];
}

inline double ImmobileSpeciesValues::getRetained ( const SpeciesId id ) const {

   if ( id != UNKNOWN ) {
      return m_retained [ id ];
   } else {
      return 0.0;
   }

}

inline double ImmobileSpeciesValues::getDensity ( const SpeciesId id ) {

   if ( id != UNKNOWN ) {
      return s_densities [ id ];
   } else {
      return 0.0;
   }

}

#endif // _FASTCAULDRON__IMMOBILE_SPECIES_VALUES_H_
