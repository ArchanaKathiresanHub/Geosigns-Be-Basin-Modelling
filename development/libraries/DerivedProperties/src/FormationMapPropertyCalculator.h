#ifndef DERIVED_PROPERTIES__FORMATION_MAP_PROPERTY_CALCULATOR_H
#define DERIVED_PROPERTIES__FORMATION_MAP_PROPERTY_CALCULATOR_H

#include <vector>
#include <string>

#include <boost/shared_ptr.hpp>

#include "AbstractProperty.h"
#include "AbstractSnapshot.h"
#include "AbstractFormation.h"

#include "FormationMapProperty.h"
#include "PropertyCalculator.h"

namespace DerivedProperties {

   // Forward declaration of PropertyManager
   class AbstractPropertyManager;

   /// \brief Calculates a derived property or set of properties.
   class FormationMapPropertyCalculator : public PropertyCalculator {

   public :

      virtual ~FormationMapPropertyCalculator () {}

      /// \brief Calculate the property values and add the property values to the list.
      virtual void calculate ( AbstractPropertyManager&            propManager,
                               const DataModel::AbstractSnapshot*  snapshot,
                               const DataModel::AbstractFormation* formation,
                                     FormationMapPropertyList&     derivedProperties ) const = 0;

   };


   typedef boost::shared_ptr<const FormationMapPropertyCalculator> FormationMapPropertyCalculatorPtr;


} // namespace DerivedProperties


#endif // DERIVED_PROPERTIES__FORMATION_MAP_PROPERTY_CALCULATOR_H
