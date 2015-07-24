#ifndef DERIVED_PROPERTIES__REFLECTIVITY_SURFACE_CALCULATOR_H
#define DERIVED_PROPERTIES__REFLECTIVITY_SURFACE_CALCULATOR_H

#include "AbstractSnapshot.h"
#include "AbstractSurface.h"

#include "AbstractPropertyManager.h"
#include "SurfacePropertyCalculator.h"

#include "GeoPhysicsProjectHandle.h"


namespace DerivedProperties {

   /// \brief Calculator for the reflectivity of a surface.
   class  ReflectivitySurfaceCalculator : public SurfacePropertyCalculator {

   public :

      /// \brief Constructor.
      ReflectivitySurfaceCalculator ( const GeoPhysics::ProjectHandle* projectHandle );

      /// \brief Calculate the reflectivity for the surface.
      ///
      /// \param [in]  propManager The property manager.
      /// \param [in]  snapshot    The snapshot time for which the reflectivity is requested.
      /// \param [in]  surface     The surface for which the reflectivity is requested.
      /// \param [out] derivedProperties On exit will contain a single surface property, the reflectivity of the surface.
      /// \pre snapshot points to a valid snapshot age.
      /// \pre surface points to a valid surface.
      virtual void calculate ( AbstractPropertyManager&           propManager,
                               const DataModel::AbstractSnapshot* snapshot,
                               const DataModel::AbstractSurface*  surface,
                               SurfacePropertyList&               derivedProperties ) const;

      /// \brief Determine if the property is computable for the specific combination of surface and snapshot.
      ///
      /// \param [in]  propManager The property manager.
      /// \param [in]  snapshot    The snapshot time for which the reflectivity is requested.
      /// \param [in]  surface     The surface for which the reflectivity is requested.
      /// \pre snapshot points to a valid snapshot age or is null.
      /// \pre surface points to a valid surface or is null.
      virtual bool isComputable ( const AbstractPropertyManager&     propManager,
                                  const DataModel::AbstractSnapshot* snapshot,
                                  const DataModel::AbstractSurface*  surface ) const;

   private :

      /// \brief The project handle.
      const GeoPhysics::ProjectHandle* m_projectHandle;

    };


}

#endif // DERIVED_PROPERTIES__REFLECTIVITY_SURFACE_CALCULATOR_H
