#ifndef _GEOPHYSICS__MANTLE_FORMATION_H_
#define _GEOPHYSICS__MANTLE_FORMATION_H_

#include "Interface/MantleFormation.h"

#include "GeoPhysicsFormation.h"


namespace DataAccess
{
   namespace Interface
   {
      class ProjectHandle;
   }
}



namespace GeoPhysics {

   class GeoPhysicsMantleFormation : virtual public GeoPhysics::Formation,
                                     virtual public DataAccess::Interface::MantleFormation {

   public :

      GeoPhysicsMantleFormation ( DataAccess::Interface::ProjectHandle* projectHandle,
                                  database::Record*                          record );



      bool isMantle () const;

      // Since this function is almost identical to the one in the CrustFormation,
      // would it be better to introduce a GeoPhysicsBasementFormation (derived from
      // the DAL::BasementFormation)?
      virtual bool setLithologiesFromStratTable ();

      void determineMinMaxThickness ();

      unsigned int setMaximumNumberOfElements ( const bool readSizeFromVolumeData );

      void retrieveAllThicknessMaps ();

      void restoreAllThicknessMaps ();

      double m_mantleElementHeight0;
    };
}

inline bool GeoPhysics::GeoPhysicsMantleFormation::isMantle () const {
   return true;
}


#endif // _GEOPHYSICS__MANTLE_FORMATION_H_
