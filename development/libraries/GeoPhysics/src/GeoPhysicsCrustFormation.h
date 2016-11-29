//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#ifndef GEOPHYSICS__CRUST_FORMATION_H
#define GEOPHYSICS__CRUST_FORMATION_H

// DataAccess library
#include "Interface/CrustFormation.h"

// Geophysics library
#include "GeoPhysicsFormation.h"

// CBMGenerics library
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
                                 database::Record*                     record );


      ~GeoPhysicsCrustFormation ();

      /// \return True since this is a crust formation
      bool isCrust () const noexcept;

      // Since this function is almost identical to the one in the GeoPhysicsMantleFormation,
      // would it be better to introduce a GeoPhysicsBasementFormation (derived from
      // the DAL::BasementFormation)?
      virtual bool setLithologiesFromStratTable ();

      /// \brief Sets the crust formation maximum and minimum thickness and the crust thickness paleo history
      void determineMinMaxThickness () final;

      /// \brief Sets and return the maximum number of elements for the crust layer
      unsigned int setMaximumNumberOfElements ( const bool readSizeFromVolumeData );

      /// \brief Sets the crust-thinning ratio and the crust maximum thickness
      /// \return The status of the function
      /// \details If the crust has a negative thickness, this function will return false
      bool determineCrustThinningRatio ();

      /// \return The crust paleo thickness at the given age
      double getCrustMaximumThicknessHistory ( const double age ) const;

      /// \return The crust thinning ratio
      double getCrustThinningRatio () const noexcept;

      /// \return The crust maximum thickness at the given (i,j) node
      double getCrustMaximumThickness ( const unsigned int i,
                                        const unsigned int j ) const;

      /// \brief Retrieves all the paleo crustal thicknesses map
      void retrieveAllThicknessMaps ();
      /// \brief Restores all the paleo crustal thicknesses map
      void restoreAllThicknessMaps ();

   protected :


      CBMGenerics::Polyfunction m_crustThickessHistory;         ///< The crust thickness history [m]
      double                    m_crustThinningRatio;           ///< The crust thinning ration   []

      DataAccess::Interface::GridMap*  m_crustMaximumThickness; ///< The crust maximum thickness [m]

   };

}

//------------------------------------------------------------//
//  Inline functions.
//------------------------------------------------------------//

inline double GeoPhysics::GeoPhysicsCrustFormation::getCrustThinningRatio () const noexcept {
   return m_crustThinningRatio;
}

inline bool GeoPhysics::GeoPhysicsCrustFormation::isCrust () const noexcept {
   return true;
}

#endif // GEOPHYSICS__CRUST_FORMATION_H
