#include "ImmobileSpeciesValues.h"
#include <sstream>

#include "utils.h"

std::string ImmobileSpeciesValues::s_namesLc [ NUM_IMMOBILES ] = { "precoke", "coke1", "hetero1", "coke2", "cokes" };
std::string ImmobileSpeciesValues::s_names [ NUM_IMMOBILES + 1 ] = { "precoke", "coke1", "Hetero1", "coke2", "CokeS", "Unknown" };

// Default values for density of immobile species.
double ImmobileSpeciesValues::s_densities [ NUM_IMMOBILES ] = { 1264.05, 1247.43, 2323.59, 2245.05, 2159.056 };

ImmobileSpeciesValues::ImmobileSpeciesValues () {

   unsigned int i;

   for ( i = 0; i < NUM_IMMOBILES; ++i ) {
      m_retained [ i ] = 0.0;
   }

}

ImmobileSpeciesValues::SpeciesId ImmobileSpeciesValues::getId ( const std::string& name ) {

   unsigned int i;

   std::string nameLc = toLower ( name );

   for ( i = 0; i < NUM_IMMOBILES; ++i ) {

      if ( nameLc == s_namesLc [ i ]) {
         return SpeciesId ( i );
      }

   }

   // Name was not found.
   return UNKNOWN;

}

const std::string& ImmobileSpeciesValues::getName ( const SpeciesId id ) { 

   if ( id != UNKNOWN ) {
      return s_names [ id ];
   } else {
      return s_names [ UNKNOWN ];
   }

}


void ImmobileSpeciesValues::setRetained ( const SpeciesId id,
                                                  double    retained ) {

   if ( id != UNKNOWN ) {
      m_retained [ id ] = retained;
   }

}

std::string ImmobileSpeciesValues::image () const {

   std::stringstream buffer;
   unsigned int i;

   buffer << std::endl;

   for ( i = 0; i < NUM_IMMOBILES; ++i ) {
      buffer << "( " << getName ( SpeciesId ( i )) << " = " << m_retained [ i ] << " )";
   }

   buffer << std::endl;

   return buffer.str ();
}

double ImmobileSpeciesValues::getRetainedVolume () const {

   double retainedSum = 0.0;
   unsigned int i;

   for ( i = 0; i < NUM_IMMOBILES; ++i ) {
      retainedSum += m_retained [ i ] / s_densities [ i ];
   }

   return retainedSum;
}


void ImmobileSpeciesValues::setDensity ( const SpeciesId id,
                                           const double    density ) {
   s_densities [ id ] = density;
}
