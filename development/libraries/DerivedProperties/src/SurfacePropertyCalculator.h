#ifndef DERIVED_PROPERTIES__SURFACE_PROPERTY_CALCULATOR_H
#define DERIVED_PROPERTIES__SURFACE_PROPERTY_CALCULATOR_H

#include <vector>
#include <string>

#include <boost/shared_ptr.hpp>

#include "AbstractProperty.h"
#include "AbstractSnapshot.h"
#include "AbstractSurface.h"

#include "SurfaceProperty.h"

namespace DerivedProperties {

   // Forward declaration of PropertyManager
   class AbstractPropertyManager;

   /// \brief Calculates a derived property or set of properties.
   class SurfacePropertyCalculator {

   public :

      virtual ~SurfacePropertyCalculator () {}

      /// \brief Calculate the property values and add the property values to the list.
      virtual void calculate ( AbstractPropertyManager&           propManager,
                               const DataModel::AbstractSnapshot* snapshot,
                               const DataModel::AbstractSurface*  surface,
                                     SurfacePropertyList&         derivedProperties ) const = 0;

      /// \brief Get a list of the property names that will be calculated by the calculator.
      virtual const std::vector<std::string>& getPropertyNames () const = 0;


   };


   typedef boost::shared_ptr<const SurfacePropertyCalculator> SurfacePropertyCalculatorPtr;


} // namespace DerivedProperties


#endif // DERIVED_PROPERTIES__SURFACE_PROPERTY_CALCULATOR_H
