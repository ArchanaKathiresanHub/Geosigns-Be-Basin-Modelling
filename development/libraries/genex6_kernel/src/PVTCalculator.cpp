//
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "PVTCalculator.h"

#include <sstream>
#include <iomanip>


Genex6::PVTComponents::PVTComponents () {
   zero ();
}

void Genex6::PVTComponents::zero () {

   int i;

   for ( i = 0; i < ComponentId::NUMBER_OF_SPECIES; ++i ) {
      m_components [ i ] = 0.0;
   }

}

double Genex6::PVTComponents::sum () const {

   double result = 0.0;
   int i;

   for ( i = 0; i < ComponentId::NUMBER_OF_SPECIES; ++i ) {
      result += m_components [ i ];
   }

   return result;
}

Genex6::PVTComponents& Genex6::PVTComponents::operator += ( const PVTComponents& components ) {

   int i;

   for ( i = 0; i < ComponentId::NUMBER_OF_SPECIES; ++i ) {
      m_components [ i ] += components.m_components [ i ];
   }

   return *this;
}

Genex6::PVTComponents& Genex6::PVTComponents::operator *= ( const double scale ) {

   int i;

   for ( i = 0; i < ComponentId::NUMBER_OF_SPECIES; ++i ) {
      m_components [ i ] *= scale;
   }

   return *this;
}

std::string Genex6::PVTComponents::image () const {

   std::stringstream buffer;
   int i;

   buffer.flags ( std::ios::scientific );
   buffer.precision ( 6 );

   buffer << " {";

   for ( i = 0; i < ComponentId::NUMBER_OF_SPECIES; ++i ) {
      buffer << "( " << std::setw ( 11 ) << CBMGenerics::ComponentManager::getInstance().getSpeciesName( i ) << " = " << std::setw ( 12 ) << m_components [ i ] << " )";

      if ( i < ComponentId::NUMBER_OF_SPECIES - 1 ) {
         buffer << ", ";
      }

   }

   buffer << " }";

   return buffer.str ();
}

void Genex6::PVTComponents::setComponents( const Genex6::PVTComponents& components1, const double scale1, 
                                           const Genex6::PVTComponents& components2, const double scale2 ) {

    int i;

   for ( i = 0; i < ComponentId::NUMBER_OF_SPECIES; ++i ) {
      m_components [ i ] = components1.m_components [ i ] * scale1 + components2.m_components [ i ] * scale2;
   }
}


//------------------------------------------------------------//


Genex6::PVTComponentMasses::PVTComponentMasses () {
   zero ();
}

void Genex6::PVTComponentMasses::zero () {

   int i;
   int j;

   for ( i = 0; i < PhaseId::NUMBER_OF_PHASES; ++i ) {

      for ( j = 0; j < ComponentId::NUMBER_OF_SPECIES; ++j ) {
         m_masses [ i ][ j ] = 0.0;
      }

   }

}

double Genex6::PVTComponentMasses::sum ( const PhaseId phase ) const {

   double result = 0.0;
   int j;

   for ( j = 0; j < ComponentId::NUMBER_OF_SPECIES; ++j ) {
      result += m_masses [ phase ][ j ];
   }

   return result;
}

Genex6::PVTComponentMasses& Genex6::PVTComponentMasses::operator+=( const PVTComponentMasses& components ) {

   int i;
   int j;

   for ( i = 0; i < PhaseId::NUMBER_OF_PHASES; ++i ) {

      for ( j = 0; j < ComponentId::NUMBER_OF_SPECIES; ++j ) {
         m_masses [ i ][ j ] += components ( PhaseId ( i ), ComponentId ( j ));
      }

   }

   return *this;
}

void Genex6::PVTComponentMasses::getPhaseComponents ( const PhaseId phase,
                                                            PVTComponents&     components ) const {

   int j;

   for ( j = 0; j < ComponentId::NUMBER_OF_SPECIES; ++j ) {
      components ( ComponentId ( j )) = m_masses [ phase ][ j ];
   }

}

Genex6::PVTComponents Genex6::PVTComponentMasses::getPhaseComponents ( const PhaseId phase ) const {

   PVTComponents components;

   getPhaseComponents ( phase, components );

   return components;
}

std::string Genex6::PVTComponentMasses::image () const {

   std::stringstream buffer;
   int i;

   buffer.flags ( std::ios::scientific );
   buffer.precision ( 6 );

   buffer  << " Vapour => {";

   for ( i = 0; i < ComponentId::NUMBER_OF_SPECIES; ++i ) {
      buffer << "( " << std::setw ( 11 ) << CBMGenerics::ComponentManager::getInstance().getSpeciesName( i ) << " = " << std::setw ( 12 ) << m_masses [ 0 ][ i ] << " )";

      if ( i < ComponentId::NUMBER_OF_SPECIES - 1 ) {
         buffer << ", ";
      }

   }

   buffer << " }" << std::endl;

   buffer  << " Liquid => {";

   for ( i = 0; i < ComponentId::NUMBER_OF_SPECIES; ++i ) {
      buffer << "( " << std::setw ( 11 ) << CBMGenerics::ComponentManager::getInstance().getSpeciesName( i ) << " = " << std::setw ( 12 ) << m_masses [ 1 ][ i ] << " )";

      if ( i < ComponentId::NUMBER_OF_SPECIES - 1 ) {
         buffer << ", ";
      }

   }

   buffer << " }" << std::endl;

   return buffer.str ();
}

//------------------------------------------------------------//

Genex6::PVTPhaseValues::PVTPhaseValues () {
   zero ();
}

void Genex6::PVTPhaseValues::zero () {

   int i;

   for ( i = 0; i < PhaseId::NUMBER_OF_PHASES; ++i ) {
      m_values [ i ] = 0.0;
   }

}
void Genex6::PVTPhaseValues::setValues( const Genex6::PVTPhaseValues& values1, const double scale1, 
                                        const Genex6::PVTPhaseValues& values2, const double scale2 ) {

    int i;

   for ( i = 0; i < PhaseId::NUMBER_OF_PHASES; ++i ) {
      m_values [ i ] = values1.m_values [ i ] * scale1 + values2.m_values [ i ] * scale2;
   }
}

std::string Genex6::PVTPhaseValues::image () const {

   std::stringstream buffer;

   buffer.flags ( std::ios::scientific );
   buffer.precision ( 6 );

   buffer << " { Vapour => " << m_values [ 0 ] << ", Liquid => " << m_values [ 1 ] << " }";

   return buffer.str ();
}

//------------------------------------------------------------//

Genex6::PVTCalc::PVTCalc () {
}

Genex6::PVTCalc& Genex6::PVTCalc::getInstance () {

   static PVTCalc theInstance;

   return theInstance;
}

bool Genex6::PVTCalc::compute ( const double               temperature,
                                const double               pressure,
                                      PVTComponents&       components,
                                      PVTComponentMasses&  masses,
                                      PVTPhaseValues&      densities,
                                      PVTPhaseValues&      viscosities,
                                const bool                 gormIsPrescribed,
                                const double               gorm ) {

   return pvtFlash::EosPack::getInstance ().computeWithLumping ( temperature,
                                                                 pressure,
                                                                 components.m_components,
                                                                 masses.m_masses,
                                                                 densities.m_values,
                                                                 viscosities.m_values,
                                                                 gormIsPrescribed,
                                                                 gorm );
}

double Genex6::PVTCalc::computeGorm ( const PVTComponents& vapour,
                                      const PVTComponents& liquid ) const {

   int i;
   double vapourMass = 0.0;
   double liquidMass = 0.0;
   double gorm;

   // Mass of normally vapour components.
   for ( i = ComponentId::C5; i <= ComponentId::C1; ++i ) {
      vapourMass += vapour ( ComponentId ( i )) + liquid ( ComponentId ( i ));
   }

   // Add H2S mass to normally vapour components
   vapourMass += vapour (ComponentId(ComponentId::H2S)) + liquid(ComponentId(ComponentId::H2S));

   // Mass of normally liquid components.
   for ( i = ComponentId::ASPHALTENE; i <= ComponentId::C6_MINUS_14SAT; ++i ) {
      liquidMass += vapour ( ComponentId ( i )) + liquid ( ComponentId ( i ));
   }

   // Add sulphur components to normally liquid components.
   for ( i = ComponentId::LSC; i <= ComponentId::C6_MINUS_14ARO_S; ++i ) {
      liquidMass += vapour ( ComponentId ( i )) + liquid ( ComponentId ( i ));
   }

   if ( liquidMass == 0.0 ) {
      gorm = 1.0e80;
   } else {
      gorm = vapourMass / liquidMass;
   }

   return gorm;

}


//------------------------------------------------------------//
