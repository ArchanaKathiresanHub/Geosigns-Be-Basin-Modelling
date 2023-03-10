//
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#ifndef FASTCAULDRON__SATURATION_H
#define FASTCAULDRON__SATURATION_H

// std library
#include <string>

// Eospack library
#include "EosPack.h"

#include "PVTCalculator.h"

// CBMGenerics library
#include "ComponentManager.h"
typedef CBMGenerics::ComponentManager::PhaseId PhaseId;

class Saturation {

public :

   enum Phase { WATER = 0, OIL = 1, LIQUID = 1, GAS = 2, VAPOUR = 2, IMMOBILE = 3 };

   static const int NumberOfPhases = 4;

   static const int BlockSize = NumberOfPhases;

   typedef Phase IndexType;

   /// \brief Return the string representation of the phase enumeration.
   static const std::string& PhaseImage ( const Phase p );

   /// \brief Converts pvt-flash pahse enumeration into a Saturation phase enumeration.
   static Phase convert ( const PhaseId phase );

   Saturation ();

   /// Return the saturation-value for the phase.
   double  operator ()( const Phase p ) const;

   /// Return the saturation-value for the phase.
   double& operator ()( const Phase p );

   /// \brief Assignment operator.
   Saturation& operator=( const Saturation& sats );

   /// \brief Add a saturation object to another.
   Saturation& operator+= ( const Saturation& sats );

   /// \brief Multiply saturation values by a scalar divisor.
   Saturation& operator*= ( const double scalar );

   /// \brief Divide saturation values by a scalar divisor.
   Saturation& operator/= ( const double divisor );

   /// Set Water to 100%, Oil and Gas to 0%.
   void initialise ();

   /// Set all saturations given the saturations of liquid and vapour.
   ///
   /// Simple calculation: S_w = 1 - S_o - S_g. 
   /// The immobile saturatio is set to zero.
   void set ( const PVTPhaseValues& phaseSaturations );

   /// Set all saturations given the saturations of liquid, vapour and immobile.
   ///
   /// Simple calculation: S_w = 1 - S_o - S_g - S_i.
   void set ( const PVTPhaseValues& phaseSaturations,
              const double          immobileSaturation );

   /// Set all saturations given the saturations of liquid and vapour.
   ///
   /// Simple calculation: S_w = 1 - S_o - S_g.
   /// The immobile saturatio is set to zero.
   void set ( const double liquidSaturation,
              const double vapourSaturation );

   /// Set all saturations given the saturations of liquid, vapour and immobile.
   ///
   /// Simple calculation: S_w = 1 - S_o - S_g - S_i.
   void set ( const double liquidSaturation,
              const double vapourSaturation,
              const double immobileSaturation );

   /// \brief Determine if all values are finite.
   ///
   /// Returns false if any value is a nan or an inf.
   bool isFinite () const;

   /// Return the string representation of the saturation values.
   std::string image () const;

private :

   double m_values [ BlockSize ];

};

inline double Saturation::operator()( Phase p ) const {
   return m_values [ int ( p )];
}

inline double& Saturation::operator()( Phase p ) {
   return m_values [ int ( p )];
}


#endif // FASTCAULDRON__SATURATION_H
