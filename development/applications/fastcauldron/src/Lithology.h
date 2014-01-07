#ifndef _FASTCAULDRON__LITHOLOGY_H_
#define _FASTCAULDRON__LITHOLOGY_H_

#include <string>

#include "CompoundLithology.h"
#include "PVTCalculator.h"
#include "Saturation.h"
#include "globaldefs.h"

class Lithology : public GeoPhysics::CompoundLithology {

public :

   Lithology ( GeoPhysics::ProjectHandle* projectHandle );


   /// \brief Compute the relative-permeability.
   double relativePermeability ( const Saturation::Phase phase,
                                 const Saturation&       saturation ) const;

#if 0
   /// \brief Compute the capillary-pressure for the phase.
   ///
   /// Permeability should be in m^2.
   double capillaryPressure ( const Saturation::Phase phase,
                              const Saturation        saturation,
                              const double            permeability ) const;
#endif

   /// \brief Compute the capillary pressure for the phase.
   ///
   /// Units for permeability are m^2.
   double capillaryPressure ( const Saturation::Phase phase,
                              const Saturation        saturation,
                              const double            temperature,
                              const double            permeability,
                              const double            brineDensity,
                              const double            hcPhaseDensity,
                              const double            criticalTemperature ) const;

   /// \brief Compute the capillary entry pressure for the phase.
   ///
   /// Units for permeability are m^2.
   double capillaryEntryPressure ( const double            temperature,
                                   const double            permeability,
                                   const double            brineDensity,
                                   const double            hcPhaseDensity,
                                   const double            criticalTemperature ) const;

   void setLithologyID ( const int id );

   int  getLithologyID () const;

   std::string getName () const;
   
protected :

   double calculateTemisRelPerm ( const Saturation::Phase phase,
                                  const double            saturation ) const;


   PVTPhaseValues m_contactAngle;
   PVTPhaseValues m_cosContactAngle;
   int            m_lithologyId;

   double         m_cosHcWaterContactAngle;
   double         m_cosAirHgContactAngle;
   double         m_airMercuryInterfacialTension;

};

inline void Lithology::setLithologyID ( const int id ) {
   m_lithologyId = id;
}

inline int Lithology::getLithologyID () const {
   return m_lithologyId;
}


#endif // _FASTCAULDRON__LITHOLOGY_H_
