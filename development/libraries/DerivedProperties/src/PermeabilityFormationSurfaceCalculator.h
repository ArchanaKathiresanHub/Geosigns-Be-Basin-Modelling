#ifndef DERIVED_PROPERTIES__PERMEABITILY_FORMATION_SURFACE_CALCULATOR_H
#define DERIVED_PROPERTIES__PERMEABITILY_FORMATION_SURFACE_CALCULATOR_H

#include "FormationSurfacePropertyCalculator.h"

namespace DerivedProperties {

   class PermeabilityFormationSurfaceCalculator : public FormationSurfacePropertyCalculator {

   public :

      PermeabilityFormationSurfaceCalculator ();

      virtual ~PermeabilityFormationSurfaceCalculator () {}
 
      virtual void calculate ( AbstractPropertyManager&            propManager,
                               const DataModel::AbstractSnapshot*  snapshot,
                               const DataModel::AbstractFormation* formation,
                               const DataModel::AbstractSurface*   surface,
                                     FormationSurfacePropertyList& derivedProperties ) const;

      /// \brief Get a list of the property names that will be calculated by the calculator.
      virtual const std::vector<std::string>& getPropertyNames () const;

   private :

      std::vector<std::string> m_propertyNames;
    };


}

#endif 
