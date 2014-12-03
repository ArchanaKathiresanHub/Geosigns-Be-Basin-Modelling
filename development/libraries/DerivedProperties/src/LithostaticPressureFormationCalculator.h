#ifndef DERIVED_PROPERTIES__LITHOSTATIC_PRESSURE_FORMATION_CALCULATOR_H
#define DERIVED_PROPERTIES__LITHOSTATIC_PRESSURE_FORMATION_CALCULATOR_H

#include "FormationPropertyCalculator.h"

namespace DerivedProperties {

   class LithostaticPressureFormationCalculator : public FormationPropertyCalculator {

   public :

      LithostaticPressureFormationCalculator ( const GeoPhysics::ProjectHandle* projectHandle );

      virtual ~LithostaticPressureFormationCalculator () {}
 
      virtual void calculate ( AbstractPropertyManager&            propManager,
                               const DataModel::AbstractSnapshot*  snapshot,
                               const DataModel::AbstractFormation* formation,
                                     FormationPropertyList&        derivedProperties ) const;

   private :

      const GeoPhysics::ProjectHandle* m_projectHandle;

   };


}

#endif 
