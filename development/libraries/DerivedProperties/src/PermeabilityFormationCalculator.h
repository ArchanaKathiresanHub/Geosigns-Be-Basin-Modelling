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

   private :

      const GeoPhysics::ProjectHandle* m_projectHandle;

   };


}

#endif 
