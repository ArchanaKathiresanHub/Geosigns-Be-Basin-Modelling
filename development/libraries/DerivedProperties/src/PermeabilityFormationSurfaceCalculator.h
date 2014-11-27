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

      /// \brief Get a list of the property names that will be calculated by the calculator.
      virtual const std::vector<std::string>& getPropertyNames () const;

   private :

      /// \brief Contains array of propert names, in this case PermeabilityVec2 and PermeabilityHVec2.
      ///
      /// These are the permeability in both vertical and horizontal directions.
      std::vector<std::string> m_propertyNames;

    };


}

#endif 
