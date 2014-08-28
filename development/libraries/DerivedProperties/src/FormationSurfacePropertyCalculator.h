#ifndef DERIVED_PROPERTIES__FORMATION_SURFACE_PROPERTY_CALCULATOR_H
#define DERIVED_PROPERTIES__FORMATION_SURFACE_PROPERTY_CALCULATOR_H

#include <vector>
#include <string>

#include <boost/shared_ptr.hpp>

#include "AbstractProperty.h"
#include "AbstractSnapshot.h"
#include "AbstractSurface.h"

#include "FormationSurfaceProperty.h"

namespace DerivedProperties {

   // Forward declaration of PropertyManager
   class DerivedPropertyManager;

   /// \brief Calculates a derived property or set of properties.
   class FormationSurfacePropertyCalculator {

   public :

      virtual ~FormationSurfacePropertyCalculator () {}

      /// \brief Calculate the property values and add the property values to the list.
      virtual void calculate ( DerivedPropertyManager&             propManager,
                               const DataModel::AbstractSnapshot*  snapshot,
                               const DataModel::AbstractFormation* formation,
                               const DataModel::AbstractSurface*   surface,
                                     FormationSurfacePropertyList& derivedProperties ) const = 0;

      /// \brief Get a list of the property names that will be calculated by the calculator.
      virtual const std::vector<std::string>& getPropertyNames () const = 0;


   };


   typedef boost::shared_ptr<const FormationSurfacePropertyCalculator> FormationSurfacePropertyCalculatorPtr;


} // namespace DerivedProperties


#endif // DERIVED_PROPERTIES__FORMATION_SURFACE_PROPERTY_CALCULATOR_H
