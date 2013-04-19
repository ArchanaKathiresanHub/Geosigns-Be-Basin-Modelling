#ifndef _GENEX5_KERNEL_IMMOBILE_SPECIES_H_
#define _GENEX5_KERNEL_IMMOBILE_SPECIES_H_

#include <string>

namespace Genex5 {

   class ImmobileSpecies {

   public :

      enum SpeciesId { PRECOKE, COKE1, HETERO1, COKE2, UNKNOWN = 4, NUM_IMMOBILES = 4 };

      ImmobileSpecies ();

      double getRetained ( const SpeciesId id ) const;

      void setRetained ( const SpeciesId id,
                               double    retained );

      /// Returns the total immobile retained volume.
      ///
      /// Each retained-immobile-species is divided by its corresponding density, then
      /// the whole sum is divided by the thickness.
      double getRetainedVolume ( const double thickness ) const;


      std::string image () const;

      static double getDensity ( const SpeciesId id );

      static void setDensity ( const SpeciesId id,
                               const double    density );

      static SpeciesId getId ( const std::string& name );

      static const std::string& getName ( const SpeciesId id );


   private :

      static std::string s_names [ NUM_IMMOBILES + 1 ];

      static double s_densities [ NUM_IMMOBILES ];

      double m_retained [ NUM_IMMOBILES ];

   };

}


inline double Genex5::ImmobileSpecies::getRetained ( const SpeciesId id ) const {

   if ( id != UNKNOWN ) {
      return m_retained [ id ];
   } else {
      return 0.0;
   }

}

inline double Genex5::ImmobileSpecies::getDensity ( const SpeciesId id ) {

   if ( id != UNKNOWN ) {
      return s_densities [ id ];
   } else {
      return 0.0;
   }

}

#endif // _GENEX5_KERNEL_IMMOBILE_SPECIES_H_
