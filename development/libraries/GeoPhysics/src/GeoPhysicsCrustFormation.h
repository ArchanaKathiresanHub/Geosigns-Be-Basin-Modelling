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

   public:

      GeoPhysicsCrustFormation ( DataAccess::Interface::ProjectHandle* projectHandle,
                                 database::Record*                     record );


      ~GeoPhysicsCrustFormation();


      // redefine set of methods which are differently were defined in base classes
      // Inherited from BasementFormation
      const string                                        & getName()                    const { return DataAccess::Interface::BasementFormation::getName(); }
      const DataAccess::Interface::LithoType              * getLithoType1()              const { return DataAccess::Interface::BasementFormation::getLithoType1(); }
      const DataAccess::Interface::GridMap                * getLithoType1PercentageMap() const { return DataAccess::Interface::BasementFormation::getLithoType1PercentageMap(); }
      const DataAccess::Interface::LithoType              * getLithoType2()              const { return DataAccess::Interface::BasementFormation::getLithoType2(); }
      const DataAccess::Interface::GridMap                * getLithoType2PercentageMap() const { return DataAccess::Interface::BasementFormation::getLithoType2PercentageMap(); }
      const DataAccess::Interface::LithoType              * getLithoType3()              const { return DataAccess::Interface::BasementFormation::getLithoType3(); }
      const DataAccess::Interface::GridMap                * getLithoType3PercentageMap() const { return DataAccess::Interface::BasementFormation::getLithoType3PercentageMap(); }
      DataAccess::Interface::ReservoirList                * getReservoirs()              const { return DataAccess::Interface::BasementFormation::getReservoirs(); }
      DataAccess::Interface::MobileLayerList              * getMobileLayers()            const { return DataAccess::Interface::BasementFormation::getMobileLayers(); }
      const DataAccess::Interface::AllochthonousLithology * getAllochthonousLithology()  const { return DataAccess::Interface::BasementFormation::getAllochthonousLithology(); }
      DataAccess::Interface::FaultCollectionList          * getFaultCollections()        const { return DataAccess::Interface::BasementFormation::getFaultCollections(); }
      bool                                                  isMobileLayer()              const { return DataAccess::Interface::BasementFormation::isMobileLayer(); }
      bool                                                  hasAllochthonousLithology()  const { return DataAccess::Interface::BasementFormation::hasAllochthonousLithology(); }
      bool                                                  isSourceRock()               const { return DataAccess::Interface::BasementFormation::isSourceRock(); }
      bool                                                  hasConstrainedOverpressure() const { return DataAccess::Interface::BasementFormation::hasConstrainedOverpressure(); }
      bool                                                  hasChemicalCompaction()      const { return DataAccess::Interface::BasementFormation::hasChemicalCompaction(); }
      CBMGenerics::capillarySealStrength::MixModel          getMixModel()                const { return DataAccess::Interface::BasementFormation::getMixModel(); }
      float                                                 getLayeringIndex()           const { return DataAccess::Interface::BasementFormation::getLayeringIndex(); }
      const DataAccess::Interface::FluidType              * getFluidType()               const { return DataAccess::Interface::BasementFormation::getFluidType(); }
      const std::string                                   & getMixModelStr()             const { return DataAccess::Interface::BasementFormation::getMixModelStr(); }
      DataAccess::Interface::GridMap                      * loadThicknessMap()           const { return DataAccess::Interface::BasementFormation::loadThicknessMap(); }
      DataAccess::Interface::GridMap                      * computeThicknessMap()        const { return DataAccess::Interface::BasementFormation::computeThicknessMap(); }
      DataAccess::Interface::GridMap                      * computeFaultGridMap( const DataAccess::Interface::Grid * localGrid, const DataAccess::Interface::Snapshot * snapshot ) const {
         return DataAccess::Interface::BasementFormation::computeFaultGridMap( localGrid, snapshot ); }
      unsigned int                                          getElementRefinement() const { return DataAccess::Interface::BasementFormation::getElementRefinement(); }

      /// \return True since this is a crust formation
      bool isCrust () const noexcept;

      // Since this function is almost identical to the one in the GeoPhysicsMantleFormation,
      // would it be better to introduce a GeoPhysicsBasementFormation (derived from
      // the DAL::BasementFormation)?
      virtual bool setLithologiesFromStratTable ();

      /// \brief Sets the crust formation maximum and minimum thickness and the crust thickness paleo history
      void determineMinMaxThickness ();

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

      /// \return The crust minimum thickness at the given (i,j) node
      double getCrustMinimumThickness ( const unsigned int i,
                                        const unsigned int j ) const;

      /// \brief Retrieves all the paleo crustal thicknesses map
      void retrieveAllThicknessMaps ();
      /// \brief Restores all the paleo crustal thicknesses map
      void restoreAllThicknessMaps ();

   protected :


      CBMGenerics::Polyfunction m_crustThickessHistory;         ///< The crust thickness history [m]
      double                    m_crustThinningRatio;           ///< The crust thinning ration   []

      DataAccess::Interface::GridMap*  m_crustMaximumThickness; ///< The crust maximum thickness [m]
      DataAccess::Interface::GridMap*  m_crustMinimumThickness; ///< The crust minimum thickness [m]

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
