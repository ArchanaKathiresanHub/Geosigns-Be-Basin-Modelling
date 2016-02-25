#include "Saturation.h"

#include <iomanip>
#include <cmath>

#ifdef _MSC_VER
#include <float.h>  // for _isnan() on VC++
#define isnan(x) _isnan(x)  // VC++ uses _isnan() instead of isnan()
#define isinf(x) !_finite(x) 
#endif /** _MSC_VER */

const std::string& Saturation::PhaseImage ( const Phase p ) {

   static const std::string images [ NumberOfPhases + 1 ] = { "WATER", "LIQUID", "VAPOUR", "IMMOBILE", "UNKNOWN"};

   if ( WATER <= p and p <= IMMOBILE ) {
      return images [ p ];
   } else {
      return images [ NumberOfPhases ];
   }

}

Saturation::Phase Saturation::convert ( const pvtFlash::PVTPhase phase ) {
   return ( phase == pvtFlash::LIQUID_PHASE ? LIQUID : VAPOUR );
}


Saturation::Saturation () {
   initialise ();
}


void Saturation::initialise () {
   m_values [ WATER    ] = 1.0;
   m_values [ LIQUID   ] = 0.0;
   m_values [ VAPOUR   ] = 0.0;
   m_values [ IMMOBILE ] = 0.0;
}

Saturation& Saturation::operator=( const Saturation& sats ) {
   m_values [ WATER    ] = sats.m_values [ WATER ];
   m_values [ LIQUID   ] = sats.m_values [ LIQUID ];
   m_values [ VAPOUR  ]  = sats.m_values [ VAPOUR ];
   m_values [ IMMOBILE ] = sats.m_values [ IMMOBILE ];

   return *this;
}

Saturation& Saturation::operator+= ( const Saturation& sats ) {

   m_values [ WATER    ] += sats.m_values [ WATER ];
   m_values [ LIQUID   ] += sats.m_values [ LIQUID ];
   m_values [ VAPOUR  ]  += sats.m_values [ VAPOUR ];
   m_values [ IMMOBILE ] += sats.m_values [ IMMOBILE ];

   return *this;
}

Saturation& Saturation::operator*= ( const double scalar ) {

   m_values [ WATER    ] *= scalar;
   m_values [ LIQUID   ] *= scalar;
   m_values [ VAPOUR   ] *= scalar;
   m_values [ IMMOBILE ] *= scalar;

   return *this;
}

Saturation& Saturation::operator/= ( const double divisor ) {

   if ( divisor != 0.0 ) {
      m_values [ WATER    ] /= divisor;
      m_values [ LIQUID   ] /= divisor;
      m_values [ VAPOUR  ]  /= divisor;
      m_values [ IMMOBILE ] /= divisor;
   } else {
      // Error!
   }

   return *this;
}

void Saturation::set ( const PVTPhaseValues& phaseSaturations ) {
   set ( phaseSaturations ( pvtFlash::LIQUID_PHASE ), phaseSaturations ( pvtFlash::VAPOUR_PHASE ), 0.0 );
}

void Saturation::set ( const PVTPhaseValues& phaseSaturations,
                       const double          immobileSaturation ) {
   set ( phaseSaturations ( pvtFlash::LIQUID_PHASE ), phaseSaturations ( pvtFlash::VAPOUR_PHASE ), immobileSaturation );
}

void Saturation::set ( const double liquidSaturation,
                       const double vapourSaturation ) {

   // m_values [ LIQUID   ] = liquidSaturation;
   // m_values [ VAPOUR   ] = vapourSaturation;
   // m_values [ WATER    ] = 1.0 - m_values [ LIQUID ] - m_values [ VAPOUR ];
   // m_values [ IMMOBILE ] = 0.0;

   set ( liquidSaturation, vapourSaturation, 0.0 );
}

void Saturation::set ( const double liquidSaturation,
                       const double vapourSaturation,
                       const double immobileSaturation ) {

   m_values [ LIQUID   ] = liquidSaturation;
   m_values [ VAPOUR   ] = vapourSaturation;
   m_values [ IMMOBILE ] = immobileSaturation;
   m_values [ WATER    ] = 1.0 - m_values [ LIQUID ] - m_values [ VAPOUR ] - m_values [ IMMOBILE ];
}

bool Saturation::isFinite () const {
   return not ( isinf ( m_values [ WATER ])    or isnan ( m_values [ WATER ]) or
                isinf ( m_values [ LIQUID ])   or isnan ( m_values [ LIQUID ]) or
                isinf ( m_values [ VAPOUR ])   or isnan ( m_values [ VAPOUR ]) or
                isinf ( m_values [ IMMOBILE ]) or isnan ( m_values [ IMMOBILE ]));
}


std::string Saturation::image () const {

   std::stringstream buffer;

   buffer.flags ( std::ios::scientific );
   buffer.precision ( 6 );

   buffer << " { ";
   buffer << std::setw ( 5 ) << PhaseImage (    WATER ) << " => " << std::setw ( 13 ) << m_values [ WATER ] << ", ";
   buffer << std::setw ( 5 ) << PhaseImage (   LIQUID ) << " => " << std::setw ( 13 ) << m_values [ LIQUID ] << ", ";
   buffer << std::setw ( 5 ) << PhaseImage (   VAPOUR ) << " => " << std::setw ( 13 ) << m_values [ VAPOUR ] << ", ";
   buffer << std::setw ( 5 ) << PhaseImage ( IMMOBILE ) << " => " << std::setw ( 13 ) << m_values [ IMMOBILE ];

   buffer << " }";

   return buffer.str ();
}

