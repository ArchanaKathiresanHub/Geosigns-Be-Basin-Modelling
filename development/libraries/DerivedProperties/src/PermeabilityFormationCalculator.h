#ifndef DERIVED_PROPERTIES__PERMEABITILY_CALCULATOR_H
#define DERIVED_PROPERTIES__PERMEABITILY_CALCULATOR_H

#include "FormationPropertyCalculator.h"
#include "GeoPhysicsProjectHandle.h"

namespace DerivedProperties {

   class PermeabilityFormationCalculator : public FormationPropertyCalculator {

   public :

      PermeabilityFormationCalculator ( const GeoPhysics::ProjectHandle* projectHandle );

      virtual ~PermeabilityFormationCalculator () {}
 
      virtual void calculate ( AbstractPropertyManager&            propManager,
                               const DataModel::AbstractSnapshot*  snapshot,
                               const DataModel::AbstractFormation* formation,
                                     FormationPropertyList&        derivedProperties ) const;

      /// \brief Get a list of the property names that will be calculated by the calculator.
      virtual const std::vector<std::string>& getPropertyNames () const;

   private :

      /// \brief Contains array of propert names, in this case PermeabilityVec2 and PermeabilityHVec2.
      ///
      /// These are the permeability in both vertical and horizontal directions.
      std::vector<std::string> m_propertyNames;

      /// \brief The project handle.
      const GeoPhysics::ProjectHandle* m_projectHandle;

   };


}

#endif 
