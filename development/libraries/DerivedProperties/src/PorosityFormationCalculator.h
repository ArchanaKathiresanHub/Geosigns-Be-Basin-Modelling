#ifndef DERIVED_PROPERTIES__POROSITY_CALCULATOR_H
#define DERIVED_PROPERTIES__POROSITY_CALCULATOR_H

#include "FormationPropertyCalculator.h"
#include "GeoPhysicsProjectHandle.h"

namespace DerivedProperties {

   class PorosityFormationCalculator : public FormationPropertyCalculator {

   public :

      PorosityFormationCalculator ( const GeoPhysics::ProjectHandle* projectHandle );

      virtual ~PorosityFormationCalculator () {}
 
      virtual void calculate ( AbstractPropertyManager&            propManager,
                               const DataModel::AbstractSnapshot*  snapshot,
                               const DataModel::AbstractFormation* formation,
                                     FormationPropertyList&        derivedProperties ) const;

   private :

      const GeoPhysics::ProjectHandle* m_projectHandle;

   };


}

#endif 
