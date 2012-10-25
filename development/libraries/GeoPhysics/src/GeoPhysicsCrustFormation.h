#ifndef _GEOPHYSICS__CRUST_FORMATION_H_
#define _GEOPHYSICS__CRUST_FORMATION_H_

#include "Interface/CrustFormation.h"

#include "GeoPhysicsFormation.h"

#include "Polyfunction.h"

namespace DataAccess
{
   namespace Interface
   {
      class ProjectHandle;
      class GridMap;
   }
}

namespace GeoPhysics {

   class GeoPhysicsCrustFormation : virtual public GeoPhysics::Formation,
                                    virtual public DataAccess::Interface::CrustFormation {

   public :

      GeoPhysicsCrustFormation ( DataAccess::Interface::ProjectHandle* projectHandle,
                                 database::Record*                          record );


      ~GeoPhysicsCrustFormation ();

      bool isCrust () const;


      // Since this function is almost identical to the one in the GeoPhysicsMantleFormation,
      // would it be better to introduce a GeoPhysicsBasementFormation (derived from
      // the DAL::BasementFormation)?
     virtual bool setLithologiesFromStratTable ();

      void determineMinMaxThickness ();

      unsigned int setMaximumNumberOfElements ( const bool readSizeFromVolumeData );

      /// Determine the crust-thinning ratio.
      ///
      /// If the crust has a negative thickness, this function will return false.
      bool determineCrustThinningRatio ();

      double getCrustMaximumThicknessHistory ( const double age ) const;

      double getCrustThinningRatio () const;

      double getCrustMaximumThickness ( const unsigned int i,
                                        const unsigned int j ) const;


      void retrieveAllThicknessMaps ();

      void restoreAllThicknessMaps ();

   protected :


      CBMGenerics::Polyfunction m_crustThickessHistory;
      double                    m_crustThinningRatio;

      DataAccess::Interface::GridMap*  m_crustMaximumThickness;

   };

}

//------------------------------------------------------------//
//  Inline functions.
//------------------------------------------------------------//

inline double GeoPhysics::GeoPhysicsCrustFormation::getCrustThinningRatio () const {
   return m_crustThinningRatio;
}

inline bool GeoPhysics::GeoPhysicsCrustFormation::isCrust () const {
   return true;
}

#endif // _GEOPHYSICS__CRUST_FORMATION_H_
