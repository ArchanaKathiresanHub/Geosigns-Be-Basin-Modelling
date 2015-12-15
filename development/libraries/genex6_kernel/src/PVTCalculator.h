#ifndef _GENEX6_KERNEL__PVT_CALCULATOR_H_
#define _GENEX6_KERNEL__PVT_CALCULATOR_H_

#include <string>
#include "EosPack.h"

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
      double  operator ()( const pvtFlash::ComponentId id ) const;

      /// \brief Accessor.
      double& operator ()( const pvtFlash::ComponentId id );

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

      double m_components [ pvtFlash::NUM_COMPONENTS ];

   };


   /// \brief A composition of all species used in pvt seperated by phase.
   class PVTComponentMasses {

   public :

      PVTComponentMasses ();

      /// \brief Accessor.
      double  operator ()( const pvtFlash::PVTPhase    phase,
                           const pvtFlash::ComponentId id ) const;

      /// \brief Accessor.
      double& operator ()( const pvtFlash::PVTPhase    phase,
                           const pvtFlash::ComponentId id );

      /// \brief Set all consentrations to zero.
      void zero ();

      /// \brief Summ all concentrations of a particular phase.
      double sum ( const pvtFlash::PVTPhase phase ) const;

      /// \brief Increment the concentrations.
      PVTComponentMasses& operator+=( const PVTComponentMasses& components );

      /// \brief Get the concentrations for a particular phase.
      void getPhaseComponents ( const pvtFlash::PVTPhase phase,
                                      PVTComponents&     components ) const;

      /// \brief Get the concentrations for a particular phase.
      PVTComponents getPhaseComponents ( const pvtFlash::PVTPhase phase ) const;

      /// \brief Return the string representation of the concentrations.
      std::string image () const;

   private :

      friend class PVTCalc;

      double m_masses [ pvtFlash::N_PHASES ][ CBMGenerics::ComponentManager::NumberOfOutputSpecies ];

   };

   /// \brief Phases.
   class PVTPhaseValues {

   public :

      PVTPhaseValues ();

      /// \brief Accessor.
      double  operator ()( const pvtFlash::PVTPhase phase ) const;

      /// \brief Accessor.
      double& operator ()( const pvtFlash::PVTPhase phase );

      /// \brief Set phases to zero.
      void zero ();

      /// \brief Return the string representation of the phases.
      std::string image () const;

      void setValues( const Genex6::PVTPhaseValues& values1, const double scale1, const Genex6::PVTPhaseValues& values2, const double scale2 );
   private :

      friend class PVTCalc;

      double m_values [ pvtFlash::N_PHASES ];

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


inline double Genex6::PVTComponents::operator ()( const pvtFlash::ComponentId id ) const {
   return m_components [ id ];
}

inline double& Genex6::PVTComponents::operator ()( const pvtFlash::ComponentId id ) {
   return m_components [ id ];
}

inline double Genex6::PVTComponentMasses::operator ()( const pvtFlash::PVTPhase    phase,
                                                       const pvtFlash::ComponentId id ) const {
   return m_masses [ phase ][ id ];
}

inline double& Genex6::PVTComponentMasses::operator ()( const pvtFlash::PVTPhase    phase,
                                                        const pvtFlash::ComponentId id ) {
   return m_masses [ phase ][ id ];
}

inline double Genex6::PVTPhaseValues::operator ()( const pvtFlash::PVTPhase phase ) const {
   return m_values [ phase ];
}

inline double& Genex6::PVTPhaseValues::operator ()( const pvtFlash::PVTPhase phase ) {
   return m_values [ phase ];
}


#endif // _GENEX6_KERNEL__PVT_CALCULATOR_H_
