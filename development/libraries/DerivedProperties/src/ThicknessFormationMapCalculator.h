#ifndef DERIVED_PROPERTIES__THICKNESS_FORMATION_MAP_CALCULATOR_H
#define DERIVED_PROPERTIES__THICKNESS_FORMATION_MAP_CALCULATOR_H

#include <vector>
#include <string>

#include <boost/shared_ptr.hpp>

#include "AbstractProperty.h"
#include "AbstractSnapshot.h"
#include "AbstractFormation.h"
#include "AbstractPropertyManager.h"

#include "FormationMapProperty.h"
#include "FormationMapPropertyCalculator.h"

namespace DerivedProperties {

   /// \brief Calculates a derived property or set of properties.
   class ThicknessFormationMapCalculator : public FormationMapPropertyCalculator {

   public :

      /// Constructor.
      ThicknessFormationMapCalculator ();

      /// \brief Calculate the property values and add the property values to the list.
      virtual void calculate ( AbstractPropertyManager&            propManager,
                               const DataModel::AbstractSnapshot*  snapshot,
                               const DataModel::AbstractFormation* formation,
                                     FormationMapPropertyList&     derivedProperties ) const;

   };

} // namespace DerivedProperties


#endif // DERIVED_PROPERTIES__THICKNESS_FORMATION_MAP_CALCULATOR_H
