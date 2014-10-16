#ifndef DERIVED_PROPERTIES__HYDROSTATIC_PRESSURE_SURFACE_CALCULATOR_H
#define DERIVED_PROPERTIES__HYDROSTATIC_PRESSURE_SURFACE_CALCULATOR_H

#include "Interface/Surface.h"

#include "SurfacePropertyCalculator.h"
#include "GeoPhysicsProjectHandle.h"
#include "GeoPhysicsFluidType.h"

#include "AbstractPropertyManager.h"
#include "DerivedSurfaceProperty.h"

namespace DerivedProperties {


   /// \brief Calculates the hydrostatic pressure for a surface.
   class HydrostaticPressureSurfaceCalculator : public SurfacePropertyCalculator {

   public :

      /// \brief Constructor.
      ///
      /// \param [in] projectHandle The geophysics project handle.
      HydrostaticPressureSurfaceCalculator ( const GeoPhysics::ProjectHandle* projectHandle );

      /// \brief Calculate the hydrostatic pressure for the surface.
      ///
      /// \param [in]  propertyManager   Manager for all derived properties.
      /// \param [in]  snapshot          The snapshot at which the property is to be calculated.
      /// \param [in]  surface           The surface for which the property is to be calculated.
      /// \param [out] derivedProperties On exit will contain the newly computed hydrostatic pressure.
      /// \pre snapshot is not null and is a valid snapshot age.
      /// \pre surface is not null and is a  valid surface.
      virtual void calculate ( AbstractPropertyManager&            propertyManager,
                               const DataModel::AbstractSnapshot*  snapshot,
                               const DataModel::AbstractSurface*   surface,
                                     SurfacePropertyList&          derivedProperties ) const;

      /// \brief Get a list of the property names that will be calculated by the calculator.
      virtual const std::vector<std::string>& getPropertyNames () const;

   private :

      /// \brief Compute the hydrostatic pressure at the top of the domain.
      void computeHydrostaticPressureAtSeaBottom ( const AbstractPropertyManager&        propertyManager,
                                                   const double                          snapshotAge,
                                                   const DataAccess::Interface::Surface* surface,
                                                         DerivedSurfacePropertyPtr&      hydrostaticPressure ) const;

      /// \brief Copy the hydrostatic pressure from the formation directly above the surface.
      void copyHydrostaticPressureFromLayerAbove ( AbstractPropertyManager&              propertyManager,
                                                   const DataModel::AbstractProperty*    hydrostaticPressureProperty,
                                                   const DataModel::AbstractSnapshot*    snapshot,
                                                   const DataAccess::Interface::Surface* surface,
                                                         DerivedSurfacePropertyPtr&      hydrostaticPressure ) const;

      /// \brief Will contain only a single string, that of HydroStaticPressure.
      std::vector<std::string> m_propertyNames;
      const GeoPhysics::ProjectHandle* m_projectHandle;

    };


}

#endif // DERIVED_PROPERTIES__HYDROSTATIC_PRESSURE_SURFACE_CALCULATOR_H
