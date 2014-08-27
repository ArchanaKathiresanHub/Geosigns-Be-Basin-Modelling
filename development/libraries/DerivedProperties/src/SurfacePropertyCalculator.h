#ifndef DERIVED_PROPERTIES__SURFACE_PROPERTY_CALCULATOR_H
#define DERIVED_PROPERTIES__SURFACE_PROPERTY_CALCULATOR_H

#include <boost/shared_ptr.hpp>

#include "AbstractProperty.h"
#include "AbstractSnapshot.h"
#include "AbstractSurface.h"

#include "SurfaceProperty.h"

namespace DerivedProperties {

   // Forward declaration of PropertyManager
   class DerivedPropertyManager;

   /// \brief Calculates a derived property.
   class SurfacePropertyCalculator {

   public :

      virtual ~SurfacePropertyCalculator () {}

      virtual void calculate ( DerivedPropertyManager&            propManager,
                               const DataModel::AbstractSnapshot* snapshot,
                               const DataModel::AbstractSurface*  surface,
                                     SurfacePropertyList&         derivedProperties ) const = 0;

   };


   typedef boost::shared_ptr<const SurfacePropertyCalculator> SurfacePropertyCalculatorPtr;


} // namespace DerivedProperties {


#endif // DERIVED_PROPERTIES__SURFACE_PROPERTY_CALCULATOR_H
