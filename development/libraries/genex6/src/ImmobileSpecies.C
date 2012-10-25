#include "ImmobileSpecies.h"
#include <sstream>
#include "Utilities.h"

std::string Genex6::ImmobileSpecies::s_namesLc [ NUM_IMMOBILES ] = { "precoke", "coke1", "hetero1", "coke2", "cokes" };
std::string Genex6::ImmobileSpecies::s_names [ NUM_IMMOBILES + 1 ] = { "precoke", "coke1", "Hetero1", "coke2", "CokeS", "Unknown" };

//double Genex6::ImmobileSpecies::s_densities [ NUM_IMMOBILES ] = { 1.0, 1.0, 1.0, 1.0, 1.0 };

Genex6::ImmobileSpecies::ImmobileSpecies () {

   unsigned int i;

   for ( i = 0; i < NUM_IMMOBILES; ++i ) {
      m_retained [ i ] = 0.0;
      m_densities [ i ] = 1.0;
   }
}

Genex6::ImmobileSpecies::SpeciesId Genex6::ImmobileSpecies::getId ( const std::string& name ) {

   unsigned int i;

   std::string nameLc = Genex6::toLower ( name );

   for ( i = 0; i < NUM_IMMOBILES; ++i ) {

      if ( nameLc == s_namesLc [ i ]) {
         return SpeciesId ( i );
      }

   }

   // Name was not found.
   return UNKNOWN;

}

const std::string& Genex6::ImmobileSpecies::getName ( const SpeciesId id ) { 

   if ( id != UNKNOWN ) {
      return s_names [ id ];
   } else {
      return s_names [ UNKNOWN ];
   }

}


void Genex6::ImmobileSpecies::setRetained ( const SpeciesId id,
                                                  double    retained ) {

   if ( id != UNKNOWN ) {
      m_retained [ id ] = retained;
   }

}

std::string Genex6::ImmobileSpecies::image () const {

   std::stringstream buffer;
   unsigned int i;

   buffer << std::endl;

   for ( i = 0; i < NUM_IMMOBILES; ++i ) {
      buffer << "( " << getName ( SpeciesId ( i )) << " = " << m_retained [ i ] << " )";
   }

   buffer << std::endl;

   return buffer.str ();
}

double Genex6::ImmobileSpecies::getRetainedVolume ( const double thickness ) const {

   double retainedSum = 0.0;
   unsigned int i;

   for ( i = 0; i < NUM_IMMOBILES; ++i ) {

      if ( m_densities [ i ] != 1.0 ) {
         retainedSum += m_retained [ i ] / m_densities [ i ];
      }

   }

   return retainedSum / thickness;
}



void Genex6::ImmobileSpecies::setDensity ( const SpeciesId id,
                                           const double    density ) {
   m_densities [ id ] = density;
}

Genex6::ImmobileSpecies & Genex6::ImmobileSpecies::operator=(const Genex6::ImmobileSpecies &inSpecies)
{
   unsigned int i;

   for ( i = 0; i < NUM_IMMOBILES; ++i ) {
      m_retained [ i ] = inSpecies.m_retained [ i ];
      m_densities [ i ] = inSpecies.m_densities [ i ];
   }

   return (*this);
}
