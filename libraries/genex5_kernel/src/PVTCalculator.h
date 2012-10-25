#ifndef _GENEX5_PVT_CALCULATOR_H_
#define _GENEX5_PVT_CALCULATOR_H_

#include <string>
#include "EosPack.h"

namespace Genex5 {


   class PVTComponents {

   public :

      PVTComponents ();

      double  operator ()( const pvtFlash::ComponentId id ) const;

      double& operator ()( const pvtFlash::ComponentId id );

      void zero ();

      double sum () const;

      PVTComponents& operator *= ( const double scale );

      PVTComponents& operator += ( const PVTComponents& components );

      std::string image () const;


   private :

      friend class PVTCalc;

      double m_components [ pvtFlash::NUM_COMPONENTS ];

   };


   class PVTComponentMasses {

   public :

      PVTComponentMasses ();

      double  operator ()( const pvtFlash::PVTPhase    phase,
                           const pvtFlash::ComponentId id ) const;

      double& operator ()( const pvtFlash::PVTPhase    phase,
                           const pvtFlash::ComponentId id );

      void zero ();

      double sum ( const pvtFlash::PVTPhase phase ) const;

      PVTComponentMasses& operator+=( const PVTComponentMasses& components );

      void getPhaseComponents ( const pvtFlash::PVTPhase phase,
                                      PVTComponents&     components ) const;

      PVTComponents getPhaseComponents ( const pvtFlash::PVTPhase phase ) const;

      std::string image () const;

   private :

      friend class PVTCalc;

      double m_masses [ pvtFlash::N_PHASES ][ CBMGenerics::ComponentManager::NumberOfOutputSpecies ];

   };

   class PVTPhaseValues {

   public :

      PVTPhaseValues ();

      double  operator ()( const pvtFlash::PVTPhase phase ) const;

      double& operator ()( const pvtFlash::PVTPhase phase );

      void zero ();

      std::string image () const;

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


inline double Genex5::PVTComponents::operator ()( const pvtFlash::ComponentId id ) const {
   return m_components [ id ];
}

inline double& Genex5::PVTComponents::operator ()( const pvtFlash::ComponentId id ) {
   return m_components [ id ];
}

inline double Genex5::PVTComponentMasses::operator ()( const pvtFlash::PVTPhase    phase,
                                                       const pvtFlash::ComponentId id ) const {
   return m_masses [ phase ][ id ];
}

inline double& Genex5::PVTComponentMasses::operator ()( const pvtFlash::PVTPhase    phase,
                                                        const pvtFlash::ComponentId id ) {
   return m_masses [ phase ][ id ];
}

inline double Genex5::PVTPhaseValues::operator ()( const pvtFlash::PVTPhase phase ) const {
   return m_values [ phase ];
}

inline double& Genex5::PVTPhaseValues::operator ()( const pvtFlash::PVTPhase phase ) {
   return m_values [ phase ];
}


#endif // _GENEX5_PVT_CALCULATOR_H_
