#ifndef DERIVED_PROPERTIES__VES_FORMATION_CALCULATOR_H
#define DERIVED_PROPERTIES__VES_FORMATION_CALCULATOR_H

#include "FormationPropertyCalculator.h"

namespace DerivedProperties {

   class VesFormationCalculator : public FormationPropertyCalculator {

   public :

      VesFormationCalculator ();

      virtual ~VesFormationCalculator () {}
 
      virtual void calculate ( AbstractPropertyManager&            propManager,
                               const DataModel::AbstractSnapshot*  snapshot,
                               const DataModel::AbstractFormation* formation,
                                     FormationPropertyList&        derivedProperties ) const;

    };


}

#endif 
