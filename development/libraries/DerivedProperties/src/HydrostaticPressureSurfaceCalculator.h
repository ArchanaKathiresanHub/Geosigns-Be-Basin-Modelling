//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef DERIVED_PROPERTIES__HYDROSTATIC_PRESSURE_SURFACE_CALCULATOR_H
#define DERIVED_PROPERTIES__HYDROSTATIC_PRESSURE_SURFACE_CALCULATOR_H

#include "Surface.h"

#include "SurfacePropertyCalculator.h"
#include "GeoPhysicsProjectHandle.h"
#include "GeoPhysicsFluidType.h"

#include "AbstractPropertyManager.h"
#include "DerivedSurfaceProperty.h"

namespace DerivedProperties {


   /// \brief Calculates the hydrostatic pressure for a surface.
   class HydrostaticPressureSurfaceCalculator : public AbstractDerivedProperties::SurfacePropertyCalculator {

   public :

      /// \brief Constructor.
      ///
      /// \param [in] projectHandle The geophysics project handle.
      HydrostaticPressureSurfaceCalculator (const GeoPhysics::ProjectHandle& projectHandle );

      /// \brief Calculate the hydrostatic pressure for the surface.
      ///
      /// \param [in]  propertyManager   Manager for all derived properties.
      /// \param [in]  snapshot          The snapshot at which the property is to be calculated.
      /// \param [in]  surface           The surface for which the property is to be calculated.
      /// \param [out] derivedProperties On exit will contain the newly computed hydrostatic pressure.
      /// \pre snapshot is not null and is a valid snapshot age.
      /// \pre surface is not null and is a  valid surface.
      virtual void calculate (       AbstractDerivedProperties::AbstractPropertyManager& propertyManager,
                               const DataModel::AbstractSnapshot*                        snapshot,
                               const DataModel::AbstractSurface*                         surface,
                                     AbstractDerivedProperties::SurfacePropertyList&     derivedProperties ) const;

   private :

      /// \brief Compute the hydrostatic pressure at the top of the domain.
      void computeHydrostaticPressureAtSeaBottom ( const AbstractDerivedProperties::AbstractPropertyManager& propertyManager,
                                                   const double                                              snapshotAge,
                                                   const DataAccess::Interface::Surface*                     surface,
                                                         DerivedSurfacePropertyPtr&                          hydrostaticPressure ) const;

      /// \brief Copy the hydrostatic pressure from the formation directly above the surface.
      void copyHydrostaticPressureFromLayerAbove ( AbstractDerivedProperties::AbstractPropertyManager& propertyManager,
                                                   const DataModel::AbstractProperty*                  hydrostaticPressureProperty,
                                                   const DataModel::AbstractSnapshot*                  snapshot,
                                                   const DataAccess::Interface::Surface*               surface,
                                                         DerivedSurfacePropertyPtr&                    hydrostaticPressure ) const;

      const GeoPhysics::ProjectHandle& m_projectHandle;

   };


}

#endif // DERIVED_PROPERTIES__HYDROSTATIC_PRESSURE_SURFACE_CALCULATOR_H
