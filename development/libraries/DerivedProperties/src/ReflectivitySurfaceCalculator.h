#ifndef DERIVED_PROPERTIES__REFLECTIVITY_SURFACE_CALCULATOR_H
#define DERIVED_PROPERTIES__REFLECTIVITY_SURFACE_CALCULATOR_H

#include "AbstractSnapshot.h"
#include "AbstractSurface.h"

#include "AbstractPropertyManager.h"
#include "SurfacePropertyCalculator.h"

#include "GeoPhysicsProjectHandle.h"


namespace DerivedProperties {

   /// 
   class  ReflectivitySurfaceCalculator : public SurfacePropertyCalculator {

   public :

      ReflectivitySurfaceCalculator ( const GeoPhysics::ProjectHandle* projectHandle );

      virtual void calculate ( AbstractPropertyManager&            propManager,
                               const DataModel::AbstractSnapshot*  snapshot,
                               const DataModel::AbstractSurface*   surface,
                                     SurfacePropertyList&          derivedProperties ) const;

      /// \brief Determine if the property is computable for the specific combination of surface and snapshot.
      virtual bool isComputable ( const AbstractPropertyManager&     propManager,
                                  const DataModel::AbstractSnapshot* snapshot,
                                  const DataModel::AbstractSurface*  surface ) const;

   private :

      const GeoPhysics::ProjectHandle* m_projectHandle;

    };


}

#endif // DERIVED_PROPERTIES__REFLECTIVITY_SURFACE_CALCULATOR_H
