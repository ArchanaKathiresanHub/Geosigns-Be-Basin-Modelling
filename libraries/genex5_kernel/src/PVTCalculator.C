#include "PVTCalculator.h"

#include <sstream>
#include <iomanip>


Genex5::PVTComponents::PVTComponents () {
   zero ();
}

void Genex5::PVTComponents::zero () {

   int i;

   for ( i = 0; i < pvtFlash::NUM_COMPONENTS; ++i ) {
      m_components [ i ] = 0.0;
   }

}

double Genex5::PVTComponents::sum () const {

   double result = 0.0;
   int i;

   for ( i = 0; i < pvtFlash::NUM_COMPONENTS; ++i ) {
      result += m_components [ i ];
   }

   return result;
}

Genex5::PVTComponents& Genex5::PVTComponents::operator += ( const PVTComponents& components ) {

   int i;

   for ( i = 0; i < pvtFlash::NUM_COMPONENTS; ++i ) {
      m_components [ i ] += components.m_components [ i ];
   }

   return *this;
}

Genex5::PVTComponents& Genex5::PVTComponents::operator *= ( const double scale ) {

   int i;

   for ( i = 0; i < pvtFlash::NUM_COMPONENTS; ++i ) {
      m_components [ i ] *= scale;
   }

   return *this;
}

std::string Genex5::PVTComponents::image () const {

   std::stringstream buffer;
   int i;

   buffer.flags ( std::ios::scientific );
   buffer.precision ( 6 );

   buffer << " {";

   for ( i = 0; i < pvtFlash::NUM_COMPONENTS; ++i ) {
      buffer << "( " << std::setw ( 11 ) << pvtFlash::ComponentIdNames [ i ] << " = " << std::setw ( 12 ) << m_components [ i ] << " )";

      if ( i < pvtFlash::NUM_COMPONENTS - 1 ) {
         buffer << ", ";
      }

   }

   buffer << " }";

   return buffer.str ();
}


//------------------------------------------------------------//


Genex5::PVTComponentMasses::PVTComponentMasses () {
   zero ();
}

void Genex5::PVTComponentMasses::zero () {

   int i;
   int j;

   for ( i = 0; i < pvtFlash::N_PHASES; ++i ) {

      for ( j = 0; j < pvtFlash::NUM_COMPONENTS; ++j ) {
         m_masses [ i ][ j ] = 0.0;
      }

   }

}

double Genex5::PVTComponentMasses::sum ( const pvtFlash::PVTPhase phase ) const {

   double result = 0.0;
   int j;

   for ( j = 0; j < pvtFlash::NUM_COMPONENTS; ++j ) {
      result += m_masses [ phase ][ j ];
   }

   return result;
}

Genex5::PVTComponentMasses& Genex5::PVTComponentMasses::operator+=( const PVTComponentMasses& components ) {

   int i;
   int j;

   for ( i = 0; i < pvtFlash::N_PHASES; ++i ) {

      for ( j = 0; j < pvtFlash::NUM_COMPONENTS; ++j ) {
         m_masses [ i ][ j ] += components ( pvtFlash::PVTPhase ( i ), pvtFlash::ComponentId ( j ));
      }

   }

   return *this;
}

void Genex5::PVTComponentMasses::getPhaseComponents ( const pvtFlash::PVTPhase phase,
                                                            PVTComponents&     components ) const {

   int j;

   for ( j = 0; j < pvtFlash::NUM_COMPONENTS; ++j ) {
      components ( pvtFlash::ComponentId ( j )) = m_masses [ phase ][ j ];
   }

}

Genex5::PVTComponents Genex5::PVTComponentMasses::getPhaseComponents ( const pvtFlash::PVTPhase phase ) const {

   PVTComponents components;

   getPhaseComponents ( phase, components );

   return components;
}

std::string Genex5::PVTComponentMasses::image () const {

   std::stringstream buffer;
   int i;

   buffer.flags ( std::ios::scientific );
   buffer.precision ( 6 );

   buffer  << " Vapour => {";

   for ( i = 0; i < pvtFlash::NUM_COMPONENTS; ++i ) {
      buffer << "( " << std::setw ( 11 ) << pvtFlash::ComponentIdNames [ i ] << " = " << std::setw ( 12 ) << m_masses [ 0 ][ i ] << " )";

      if ( i < pvtFlash::NUM_COMPONENTS - 1 ) {
         buffer << ", ";
      }

   }

   buffer << " }" << std::endl;

   buffer  << " Liquid => {";

   for ( i = 0; i < pvtFlash::NUM_COMPONENTS; ++i ) {
      buffer << "( " << std::setw ( 11 ) << pvtFlash::ComponentIdNames [ i ] << " = " << std::setw ( 12 ) << m_masses [ 1 ][ i ] << " )";

      if ( i < pvtFlash::NUM_COMPONENTS - 1 ) {
         buffer << ", ";
      }

   }

   buffer << " }" << std::endl;

   return buffer.str ();
}

//------------------------------------------------------------//

Genex5::PVTPhaseValues::PVTPhaseValues () {
   zero ();
}

void Genex5::PVTPhaseValues::zero () {

   int i;

   for ( i = 0; i < pvtFlash::N_PHASES; ++i ) {
      m_values [ i ] = 0.0;
   }

}

std::string Genex5::PVTPhaseValues::image () const {

   std::stringstream buffer;
   int i;

   buffer.flags ( std::ios::scientific );
   buffer.precision ( 6 );

   buffer << " { Vapour => " << m_values [ 0 ] << ", Liquid => " << m_values [ 1 ] << " }";

   return buffer.str ();
}




//------------------------------------------------------------//

Genex5::PVTCalc::PVTCalc () {
}

Genex5::PVTCalc& Genex5::PVTCalc::getInstance () {

   static PVTCalc theInstance;

   return theInstance;
}

bool Genex5::PVTCalc::compute ( const double               temperature,
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

double Genex5::PVTCalc::computeGorm ( const PVTComponents& vapour,
                                      const PVTComponents& liquid ) const {

   int i;
   double vapourMass = 0.0;
   double liquidMass = 0.0;
   double gorm;

   for ( i = pvtFlash::C5; i <= pvtFlash::C1; ++i ) {
      vapourMass += vapour ( pvtFlash::ComponentId ( i )) + liquid ( pvtFlash::ComponentId ( i ));
   }

   for ( i = pvtFlash::ASPHALTENES; i <= pvtFlash::C6_14SAT; ++i ) {
      liquidMass += vapour ( pvtFlash::ComponentId ( i )) + liquid ( pvtFlash::ComponentId ( i ));
   }

   if ( liquidMass == 0.0 ) {
      gorm = 1.0e80;
   } else {
      gorm = vapourMass / liquidMass;
   }

   return gorm;

}


//------------------------------------------------------------//
