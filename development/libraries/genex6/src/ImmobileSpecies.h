#ifndef _GENEX6__IMMOBILE_SPECIES_H_
#define _GENEX6__IMMOBILE_SPECIES_H_

#include <string>

namespace Genex6 {

   /// \brief Holds concentrations of immobile species.
   //
   // This class could be generalised some what.
   class ImmobileSpecies {

   public :

      /// \brief The name of the immobile species.
      enum SpeciesId { PRECOKE, COKE1, HETERO1, COKE2, COKES, UNKNOWN = 5, NUM_IMMOBILES = 5 };


      /// \brief Constructor.
      ImmobileSpecies ();

      /// \brief Get the retained concentration of the species.
      double getRetained ( const SpeciesId id ) const;

      /// \brief Set the retained concentration of the species.
      void setRetained ( const SpeciesId id,
                               double    retained );

      /// Returns the total immobile retained volume.
      ///
      /// Each retained-immobile-species is divided by its corresponding density, then
      /// the whole sum is divided by the thickness.
      double getRetainedVolume ( const double thickness ) const;

      /// \brief Return a string representation of the object.
      std::string image () const;

      /// \brief Get the density of the immobile species.
      double getDensity ( const SpeciesId id );

      /// \brief Set the density of the immobile species.
      void setDensity ( const SpeciesId id,
                               const double    density );

      /// \brief Get the specied-id frmo the strnig representation.
      ///
      /// If the name cannot be found then UNKNOWN will be returned.
      /// The comparison is not case-sensitive.
      SpeciesId getId ( const std::string& name );

      /// \brief Return the string for the species-id.
      static const std::string& getName ( const SpeciesId id );

      ImmobileSpecies & operator=(const ImmobileSpecies &aSpecies);

   private :

      /// \brief Array of immobile-species names in lower case.
      static std::string s_namesLc [ NUM_IMMOBILES ];

      /// \brief Array of immobile-species names.
      static std::string s_names   [ NUM_IMMOBILES + 1 ];

      /// \brief Array if species densities.
      double m_densities [ NUM_IMMOBILES ];

      double m_retained [ NUM_IMMOBILES ];

   };

}


inline double Genex6::ImmobileSpecies::getRetained ( const SpeciesId id ) const {

   if ( id != UNKNOWN ) {
      return m_retained [ id ];
   } else {
      return 0.0;
   }

}

inline double Genex6::ImmobileSpecies::getDensity ( const SpeciesId id ) {

   if ( id != UNKNOWN ) {
      return m_densities [ id ];
   } else {
      return 0.0;
   }

}

#endif // _GENEX6__IMMOBILE_SPECIES_H_
