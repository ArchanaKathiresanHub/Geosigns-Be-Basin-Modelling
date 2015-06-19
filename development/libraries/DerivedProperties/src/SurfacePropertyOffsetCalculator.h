#ifndef DERIVED_PROPERTIES__SURFACE_PROPERTY_OFFSET_CALCULATOR_H
#define DERIVED_PROPERTIES__SURFACE_PROPERTY_OFFSET_CALCULATOR_H

#include <vector>
#include <string>

#include "AbstractProperty.h"
#include "AbstractSnapshot.h"
#include "AbstractSurface.h"

#include "AbstractPropertyManager.h"
#include "SurfaceProperty.h"
#include "SurfacePropertyCalculator.h"

namespace DerivedProperties {

   /// \brief Calculates a derived property or set of properties.
   ///
   /// Calculates the surface property from the formation property values.
   class SurfacePropertyOffsetCalculator : public SurfacePropertyCalculator {

   public :

      /// \brief Constructor.
      ///
      /// \param [in] property The proerty for which the calculator is to extract values.
      /// \param [in] dependentPropertyNames The list of properties that are required to be able to compute this property.
      /// \pre property points to a valid property object.
      SurfacePropertyOffsetCalculator ( const DataModel::AbstractProperty* property,
                                        const std::vector<std::string>&    dependentPropertyNames );

      /// \brief Calculate the property values and add the property values to the list.
      virtual void calculate ( AbstractPropertyManager&           propManager,
                               const DataModel::AbstractSnapshot* snapshot,
                               const DataModel::AbstractSurface*  surface,
                                     SurfacePropertyList&         derivedProperties ) const;

      /// \brief Determine if the property is computable for the specific combination of formation, surface and snapshot.
      virtual bool isComputable ( const AbstractPropertyManager&      propManager,
                                  const DataModel::AbstractSnapshot*  snapshot,
                                  const DataModel::AbstractSurface*   surface ) const;

   private :

      /// \brief The property calculated by this calculator.
      const DataModel::AbstractProperty* m_property;

   };


} // namespace DerivedProperties


#endif // DERIVED_PROPERTIES__SURFACE_PROPERTY_OFFSET_CALCULATOR_H
