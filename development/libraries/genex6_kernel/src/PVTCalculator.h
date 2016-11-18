//
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#ifndef GENEX6_KERNEL__PVT_CALCULATOR_H
#define GENEX6_KERNEL__PVT_CALCULATOR_H

// std library
#include <string>

// Eospack library
#include "EosPack.h"

// CBMGenerics library
#include "ComponentManager.h"
typedef CBMGenerics::ComponentManager::PhaseId PhaseId;
typedef CBMGenerics::ComponentManager::SpeciesNamesId ComponentId;

namespace Genex6 {


   // This file needs to be split into several smaller 
   // files for each of the classes contained.
   // There is probably some duplication of code with fastmig and 
   // the Darcy flow simulator.

   /// \brief A composition of all species used in pvt.
   class PVTComponents {

   public :

      PVTComponents ();

      /// \brief Accessor.
      double  operator ()( const ComponentId id ) const;

      /// \brief Accessor.
      double& operator ()( const ComponentId id );

      /// \brief Set all concentrations to zero.
      void zero ();

      /// \brief Summ all concentrations.
      double sum () const;

      /// \brief Scale all concentrations by some scalar.
      PVTComponents& operator *= ( const double scale );

      /// \brief Increment the concentrations.
      PVTComponents& operator += ( const PVTComponents& components );

      /// \brief Return a string representation of the concentrations.
      std::string image () const;

      void setComponents( const Genex6::PVTComponents& components1, const double scale1, 
                          const Genex6::PVTComponents& components2, const double scale2 );

      // Made public temporarily.
   // private :

      friend class PVTCalc;

      double m_components [ComponentId::NUMBER_OF_SPECIES];

   };


   /// \brief A composition of all species used in pvt seperated by phase.
   class PVTComponentMasses {

   public :

      PVTComponentMasses ();

      /// \brief Accessor.
      double  operator ()( const PhaseId     phase,
                           const ComponentId id ) const;

      /// \brief Accessor.
      double& operator ()( const PhaseId     phase,
                           const ComponentId id );

      /// \brief Set all consentrations to zero.
      void zero ();

      /// \brief Summ all concentrations of a particular phase.
      double sum ( const PhaseId phase ) const;

      /// \brief Increment the concentrations.
      PVTComponentMasses& operator+=( const PVTComponentMasses& components );

      /// \brief Get the concentrations for a particular phase.
      void getPhaseComponents ( const PhaseId        phase,
                                      PVTComponents& components ) const;

      /// \brief Get the concentrations for a particular phase.
      PVTComponents getPhaseComponents ( const PhaseId phase ) const;

      /// \brief Return the string representation of the concentrations.
      std::string image () const;

   private :

      friend class PVTCalc;

      double m_masses [ PhaseId::NUMBER_OF_PHASES][ ComponentId::NUMBER_OF_SPECIES ];

   };

   /// \brief Phases.
   class PVTPhaseValues {

   public :

      PVTPhaseValues ();

      /// \brief Accessor.
      double  operator ()( const PhaseId phase ) const;

      /// \brief Accessor.
      double& operator ()( const PhaseId phase );

      /// \brief Set phases to zero.
      void zero ();

      /// \brief Return the string representation of the phases.
      std::string image () const;

      void setValues( const Genex6::PVTPhaseValues& values1, const double scale1, const Genex6::PVTPhaseValues& values2, const double scale2 );
   private :

      friend class PVTCalc;

      double m_values [PhaseId::NUMBER_OF_PHASES];

   };



   class PVTCalc {

   public :

      static PVTCalc& getInstance ();

      bool compute ( const double               temperature,
                     const double               pressure,
                           PVTComponents&       components,
                           PVTComponentMasses&  masses,
                           PVTPhaseValues&      densities,
                           PVTPhaseValues&      viscosities,
                     const bool                 gormIsPrescribed = false,
                     const double               gorm = 0.0 );

      double computeGorm ( const PVTComponents& vapour,
                           const PVTComponents& liquid ) const;

   private :

      PVTCalc ();

   };

}


//------------------------------------------------------------//


inline double Genex6::PVTComponents::operator ()( const ComponentId id ) const {
   return m_components [ id ];
}

inline double& Genex6::PVTComponents::operator ()( const ComponentId id ) {
   return m_components [ id ];
}

inline double Genex6::PVTComponentMasses::operator ()( const PhaseId     phase,
                                                       const ComponentId id ) const {
   return m_masses [ phase ][ id ];
}

inline double& Genex6::PVTComponentMasses::operator ()( const PhaseId     phase,
                                                        const ComponentId id ) {
   return m_masses [ phase ][ id ];
}

inline double Genex6::PVTPhaseValues::operator ()( const PhaseId phase ) const {
   return m_values [ phase ];
}

inline double& Genex6::PVTPhaseValues::operator ()( const PhaseId phase ) {
   return m_values [ phase ];
}


#endif // GENEX6_KERNEL__PVT_CALCULATOR_H
