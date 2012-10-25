#include "ImmobileSpecies.h"
#include <sstream>

std::string Genex5::ImmobileSpecies::s_names [ NUM_IMMOBILES + 1 ] = { "precoke", "coke1", "Hetero1", "coke2", "Unknown" };

double Genex5::ImmobileSpecies::s_densities [ NUM_IMMOBILES ] = { 1.0, 1.0, 1.0, 1.0 };

Genex5::ImmobileSpecies::ImmobileSpecies () {

   unsigned int i;

   for ( i = 0; i < NUM_IMMOBILES; ++i ) {
      m_retained [ i ] = 0.0;
   }

}

Genex5::ImmobileSpecies::SpeciesId Genex5::ImmobileSpecies::getId ( const std::string& name ) {

   unsigned int i;

   for ( i = 0; i < NUM_IMMOBILES; ++i ) {

      if ( name == s_names [ i ]) {
         return SpeciesId ( i );
      }

   }

   // Name was not found.
   return UNKNOWN;

}

const std::string& Genex5::ImmobileSpecies::getName ( const SpeciesId id ) { 

   if ( id != UNKNOWN ) {
      return s_names [ id ];
   } else {
      return s_names [ UNKNOWN ];
   }

}


void Genex5::ImmobileSpecies::setRetained ( const SpeciesId id,
                                                  double    retained ) {

   if ( id != UNKNOWN ) {
      m_retained [ id ] = retained;
   }

}

std::string Genex5::ImmobileSpecies::image () const {

   std::stringstream buffer;
   unsigned int i;

   buffer << std::endl;

   for ( i = 0; i < NUM_IMMOBILES; ++i ) {
      buffer << "( " << getName ( SpeciesId ( i )) << " = " << m_retained [ i ] << " )";
   }

   buffer << std::endl;

   return buffer.str ();
}

double Genex5::ImmobileSpecies::getRetainedVolume ( const double thickness ) const {

   double retainedSum = 0.0;
   unsigned int i;

   for ( i = 0; i < NUM_IMMOBILES; ++i ) {
      retainedSum += m_retained [ i ] / s_densities [ i ];
   }

   return retainedSum / thickness;
}



void Genex5::ImmobileSpecies::setDensity ( const SpeciesId id,
                                           const double    density ) {
   s_densities [ id ] = density;
}
