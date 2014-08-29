#ifndef DERIVED_PROPERTIES__FORMATION_PROPERTY_CALCULATOR_H
#define DERIVED_PROPERTIES__FORMATION_PROPERTY_CALCULATOR_H

#include <vector>
#include <string>

#include <boost/shared_ptr.hpp>

#include "AbstractProperty.h"
#include "AbstractSnapshot.h"
#include "AbstractSurface.h"

#include "FormationProperty.h"

namespace DerivedProperties {

   // Forward declaration of PropertyManager
   class AbstractPropertyManager;

   /// \brief Calculates a derived property or set of properties.
   class FormationPropertyCalculator {

   public :

      virtual ~FormationPropertyCalculator () {}

      /// \brief Calculate the property values and add the property values to the list.
      virtual void calculate ( AbstractPropertyManager&            propManager,
                               const DataModel::AbstractSnapshot*  snapshot,
                               const DataModel::AbstractFormation* formation,
                                     FormationPropertyList&        derivedProperties ) const = 0;

      /// \brief Get a list of the property names that will be calculated by the calculator.
      virtual const std::vector<std::string>& getPropertyNames () const = 0;


   };


   typedef boost::shared_ptr<const FormationPropertyCalculator> FormationPropertyCalculatorPtr;


} // namespace DerivedProperties


#endif // DERIVED_PROPERTIES__FORMATION_PROPERTY_CALCULATOR_H
