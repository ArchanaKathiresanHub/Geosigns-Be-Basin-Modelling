#ifndef DERIVED_PROPERTIES__PERMEABITILY_FORMATION_SURFACE_CALCULATOR_H
#define DERIVED_PROPERTIES__PERMEABITILY_FORMATION_SURFACE_CALCULATOR_H

#include "FormationSurfacePropertyCalculator.h"
#include "GeoPhysicsProjectHandle.h"

namespace DerivedProperties {

   class PermeabilityFormationSurfaceCalculator : public FormationSurfacePropertyCalculator {

   public :

      PermeabilityFormationSurfaceCalculator ( const GeoPhysics::ProjectHandle* projectHandle );

      virtual ~PermeabilityFormationSurfaceCalculator () {}
 
      virtual void calculate ( AbstractPropertyManager&            propManager,
                               const DataModel::AbstractSnapshot*  snapshot,
                               const DataModel::AbstractFormation* formation,
                               const DataModel::AbstractSurface*   surface,
                                     FormationSurfacePropertyList& derivedProperties ) const;

   private :

      const GeoPhysics::ProjectHandle* m_projectHandle;

   };


}

#endif 
