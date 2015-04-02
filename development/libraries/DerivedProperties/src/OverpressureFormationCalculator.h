#ifndef DERIVED_PROPERTIES__OVERPRESSURE_FORMATION_CALCULATOR_H
#define DERIVED_PROPERTIES__OVERPRESSURE_FORMATION_CALCULATOR_H

#include "FormationPropertyCalculator.h"

namespace DerivedProperties {

   class OverpressureFormationCalculator : public FormationPropertyCalculator {

   public :

      OverpressureFormationCalculator ();

      virtual ~OverpressureFormationCalculator () {}
 
      virtual void calculate ( AbstractPropertyManager&            propManager,
                               const DataModel::AbstractSnapshot*  snapshot,
                               const DataModel::AbstractFormation* formation,
                                     FormationPropertyList&        derivedProperties ) const;

    };


}

#endif // DERIVED_PROPERTIES__OVERPRESSURE_FORMATION_CALCULATOR_H
