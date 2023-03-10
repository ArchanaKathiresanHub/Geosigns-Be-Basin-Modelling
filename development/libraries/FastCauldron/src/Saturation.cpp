//
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "Saturation.h"

// std library
#include <iomanip>
#include <cmath>

const std::string& Saturation::PhaseImage ( const Phase p ) {

   static const std::string images [ NumberOfPhases + 1 ] = { "WATER", "LIQUID", "VAPOUR", "IMMOBILE", "UNKNOWN"};

   if ( WATER <= p and p <= IMMOBILE ) {
      return images [ p ];
   } else {
      return images [ NumberOfPhases ];
   }

}

Saturation::Phase Saturation::convert ( const PhaseId phase ) {
   return ( phase == PhaseId::LIQUID ? LIQUID : VAPOUR );
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
   set ( phaseSaturations ( PhaseId::LIQUID ), phaseSaturations ( PhaseId::VAPOUR ), 0.0 );
}

void Saturation::set ( const PVTPhaseValues& phaseSaturations,
                       const double          immobileSaturation ) {
   set ( phaseSaturations ( PhaseId::LIQUID ), phaseSaturations ( PhaseId::VAPOUR ), immobileSaturation );
}

void Saturation::set ( const double liquidSaturation,
                       const double vapourSaturation ) {

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
   return not ( std::isinf ( m_values [ WATER ])    or std::isnan ( m_values [ WATER ]) or
                std::isinf ( m_values [ LIQUID ])   or std::isnan ( m_values [ LIQUID ]) or
                std::isinf ( m_values [ VAPOUR ])   or std::isnan ( m_values [ VAPOUR ]) or
                std::isinf ( m_values [ IMMOBILE ]) or std::isnan ( m_values [ IMMOBILE ]));
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

