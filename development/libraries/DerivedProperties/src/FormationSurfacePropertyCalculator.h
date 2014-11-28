#ifndef DERIVED_PROPERTIES__FORMATION_SURFACE_PROPERTY_CALCULATOR_H
#define DERIVED_PROPERTIES__FORMATION_SURFACE_PROPERTY_CALCULATOR_H

#include <vector>
#include <string>

#include <boost/shared_ptr.hpp>

#include "AbstractProperty.h"
#include "AbstractSnapshot.h"
#include "AbstractSurface.h"

#include "FormationSurfaceProperty.h"
#include "PropertyCalculator.h"

namespace DerivedProperties {

   // Forward declaration of PropertyManager
   class AbstractPropertyManager;

   /// \brief Calculates a derived property or set of properties.
   class FormationSurfacePropertyCalculator : public PropertyCalculator {

   public :

      virtual ~FormationSurfacePropertyCalculator () {}

      /// \brief Calculate the property values and add the property values to the list.
      virtual void calculate ( AbstractPropertyManager&            propManager,
                               const DataModel::AbstractSnapshot*  snapshot,
                               const DataModel::AbstractFormation* formation,
                               const DataModel::AbstractSurface*   surface,
                                     FormationSurfacePropertyList& derivedProperties ) const = 0;


   };


   typedef boost::shared_ptr<const FormationSurfacePropertyCalculator> FormationSurfacePropertyCalculatorPtr;


} // namespace DerivedProperties


#endif // DERIVED_PROPERTIES__FORMATION_SURFACE_PROPERTY_CALCULATOR_H
