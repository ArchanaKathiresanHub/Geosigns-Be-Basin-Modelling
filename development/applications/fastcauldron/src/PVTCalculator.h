//
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#ifndef FASTCAULDRON__PVT_CALCULATOR__H
#define FASTCAULDRON__PVT_CALCULATOR__H

// std library
#include <string>

// Eospack library
#include "EosPack.h"

// CBMGenerics library
#include "ComponentManager.h"
typedef CBMGenerics::ComponentManager::PhaseId PhaseId;
typedef CBMGenerics::ComponentManager::SpeciesNamesId ComponentId;

/// The number of components modelled in the pvt flash calculator.
static const int NumberOfPVTComponents = ComponentId::NUMBER_OF_SPECIES;

/// Container for all PVT components.
///
/// DO NOT add any virtual functions to this class.
class PVTComponents {

public :

   static const int BlockSize = NumberOfPVTComponents;

   typedef ComponentId IndexType;


   /// Return the value for the component.
   double  operator ()( const ComponentId id ) const;

   /// Return the value for the component.
   double& operator ()( const ComponentId id );

   /// Set all values to zero.
   void zero ();

   /// Fill the component-values with the desired scalar value.
   void fill ( const double withTheValue );

   /// Fill the component-values with the desired scalar value.
   PVTComponents& operator=( const double value );

   /// Fill the component-values with the desired scalar value.
   PVTComponents& operator=( const PVTComponents& values );

   /// Return the sum of all values.
   double sum () const;

   /// Return the sum of the ratio of all values.
   double sumRatios ( const PVTComponents& divisors ) const;

   /// An inner-product of ...
   friend double dot ( const PVTComponents& components1,
                       const PVTComponents& components2 );

   /// Here phase represents the phase at STP.
   ///
   /// Gas/Vapour = C1, C2, C3, C4 and C5;
   /// Oil/Liquid = All others, e.g. C15_SAT, ...
   double sum ( const PhaseId phase ) const;

   /// Scale all component-values by scalar.
   PVTComponents& operator*= ( const double scale );

   /// Add component-values to current values.
   PVTComponents& operator+= ( const PVTComponents& components );

   /// Subtract component-values from current values.
   PVTComponents& operator-= ( const PVTComponents& components );

   /// Multiply values component-wise.
   PVTComponents& operator*= ( const PVTComponents& components );

   /// Divide values component-wise.
   PVTComponents& operator/= ( const PVTComponents& components );

   /// Add scalar value to current values.
   PVTComponents& operator += ( const double value );

   /// Return a string representation of the component-values.
   std::string image ( const bool reverseOrder = true ) const;

   bool isFinite() const;

   /// \brief Determine if all values are non-negative.
   bool isNonNegative () const;

// private :

   friend class PVTCalc;

   double m_components [ NumberOfPVTComponents ];

};

/// \brief Add two sets of components together.
PVTComponents operator+( const PVTComponents& left, const PVTComponents& right );

/// \brief Subtract one sets of components from another.
PVTComponents operator-( const PVTComponents& left, const PVTComponents& right );

/// \brief Multiply (pre) a set of components by a scalar.
PVTComponents operator*( const double left, const PVTComponents& right );

/// \brief Multiply (post) a set of components by a scalar.
PVTComponents operator*( const PVTComponents& left, const double right );

/// \brief Divide a set of components by a scalar.
PVTComponents operator/( const PVTComponents& left, const double right );

/// \brief Multiplty values component-wise.
PVTComponents operator*( const PVTComponents& left, const PVTComponents& right );

/// \brief Divide values component-wise.
PVTComponents operator/( const PVTComponents& left, const PVTComponents& right );

/// \brief return the maximum of two compositions.
PVTComponents maximum ( const PVTComponents& left,
                        const PVTComponents& right );

/// \brief return the maximum of a composition and a scalar value.
PVTComponents maximum ( const PVTComponents& left, const double right );

double minimum ( const PVTComponents& cmps );

/// Container for PVT phases.
class PVTPhaseValues {

public :

   PVTPhaseValues ();

   /// Return the value for the phase.
   double  operator ()( const PhaseId phase ) const;

   /// Return the value for the phase.
   double& operator ()( const PhaseId phase );

   /// \brief Assignment operator.
   PVTPhaseValues& operator=( const PVTPhaseValues& values );

   /// Summ all phase values.
   double sum () const;

   /// Set all phase-values to zero.
   void zero ();

   /// Return the string representation of the phase values.
   std::string image () const;

   PVTPhaseValues& operator*=( const double scalar );

   PVTPhaseValues& operator/=( const PVTPhaseValues& divisor );

// private :

   friend class PVTCalc;

   double m_values [PhaseId::NUMBER_OF_PHASES];

};

PVTPhaseValues maximum ( const PVTPhaseValues& values, 
                         const double          scalar );

PVTPhaseValues maximum ( const PVTPhaseValues& values1, 
                         const PVTPhaseValues& values2 );



/// Container for all PVT components for each phase.
class PVTPhaseComponents {

public :

   /// Return the value for the component in particular phase.
   double  operator ()( const PhaseId     phase,
                        const ComponentId id ) const;

   /// Return the value for the component in particular phase.
   double& operator ()( const PhaseId     phase,
                        const ComponentId id );

   /// Set all values to zero.
   void zero ();

   /// Sum all component values for a particular phase.
   double sum ( const PhaseId phase ) const;

   /// \brief Sum the components for each phase.
   void sum ( PVTPhaseValues& phases ) const;

   /// \brief Sum the components for each phase scaled by a scalar.
   void sumProduct ( const PVTComponents&  scalars,
                           PVTPhaseValues& phases ) const;

   /// \brief Sum the phase components.
   void sum ( PVTComponents& components ) const;

   /// Assign phase-component-values to current values.
   PVTPhaseComponents& operator=( const PVTPhaseComponents& components );

   /// Addd phase-component-values to current values.
   PVTPhaseComponents& operator+=( const PVTPhaseComponents& components );

   /// Set the component-values for a particular phase.
   void setPhaseComponents ( const PhaseId        phase,
                             const PVTComponents& components );

   /// Get the component-values for a particular phase.
   void getPhaseComponents ( const PhaseId        phase,
                                   PVTComponents& components ) const;

   /// Get the component-values for a particular phase.
   PVTComponents getPhaseComponents ( const PhaseId phase ) const;

   /// Return the string representation of phase-component-values.
   std::string image ( const bool reverseOrder = true ) const;


   /// Compute the component values for the particular phase concentrations.
   friend PVTComponents operator* ( const PVTPhaseComponents& cmps, 
                                    const PVTPhaseValues&     phases );


   /// Multiply component-values by scalar.
   PVTPhaseComponents& operator*= ( const double value );

   /// Multiply values component-wise.
   PVTPhaseComponents& operator*= ( const PVTComponents& components );

   /// Divide values component-wise.
   PVTPhaseComponents& operator/= ( const PVTComponents& components );

   /// Divide values phase-wise.
   PVTPhaseComponents& operator/= ( const PVTPhaseValues& phases );

// private :

   friend class PVTCalc;

   double m_masses [PhaseId::NUMBER_OF_PHASES][ NumberOfPVTComponents ];

};

double minimum ( const PVTPhaseComponents& cmps );


class PVTCalc {

public :

   static PVTCalc& getInstance ();

   /// Call the pvt flash calculation.
   bool compute ( const double               temperature,
                  const double               pressure,
                        PVTComponents&       components,
                        PVTPhaseComponents&  masses,
                        PVTPhaseValues&      densities,
                        PVTPhaseValues&      viscosities,
                        PVTComponents&       kValues,
                  const bool                 gormIsPrescribed = false,
                  const double               gorm = 0.0 );

   /// Compute the gorm.
   double computeGorm ( const PVTComponents& vapour,
                        const PVTComponents& liquid ) const;

   double computeGorm ( const PVTPhaseComponents& phaseComposition ) const;

   /// Compute the gorm.
   double computeGorm ( const PVTComponents& weights ) const;

   const PVTComponents& getMolarMass () const;

   PVTComponents computeMolarMass ( const PVTComponents& weights ) const;

   PVTPhaseValues computeCriticalTemperature ( const PVTPhaseComponents& composition,
                                               const bool                gormIsPrescribed = false,
                                               const double              prescribedGorm = 0.0 ) const;

   double computeCriticalTemperature ( const PVTComponents& composition,
                                       const bool           gormIsPrescribed = false,
                                       const double         prescribedGorm = 0.0 ) const;

private :

   PVTCalc ();

   static PVTCalc* m_theInstance;


   PVTComponents m_molarMass;

};


//------------------------------------------------------------//


inline double PVTComponents::operator ()( const ComponentId id ) const {
   return m_components [ id ];
}

inline double& PVTComponents::operator ()( const ComponentId id ) {
   return m_components [ id ];
}

inline double PVTPhaseComponents::operator ()( const PhaseId     phase,
                                               const ComponentId id ) const {
   return m_masses [ phase ][ id ];
}

inline double& PVTPhaseComponents::operator ()( const PhaseId     phase,
                                                const ComponentId id ) {
   return m_masses [ phase ][ id ];
}

inline double PVTPhaseValues::operator ()( const PhaseId phase ) const {
   return m_values [ phase ];
}

inline double& PVTPhaseValues::operator ()( const PhaseId phase ) {
   return m_values [ phase ];
}

inline PVTPhaseValues& PVTPhaseValues::operator/=( const PVTPhaseValues& divisor ) {
   m_values [ 0 ] /= divisor ( PhaseId::VAPOUR );
   m_values [ 1 ] /= divisor ( PhaseId::LIQUID );

   return *this;
}


inline double PVTPhaseValues::sum () const {
   return m_values [ 0 ] + m_values [ 1 ];
}


inline PVTCalc& PVTCalc::getInstance () {

   if ( m_theInstance == 0 ) {
      m_theInstance = new PVTCalc;
   }

   return *m_theInstance;
}

inline const PVTComponents& PVTCalc::getMolarMass () const {
   return m_molarMass;
}


#endif // FASTCAULDRON__PVT_CALCULATOR__H
