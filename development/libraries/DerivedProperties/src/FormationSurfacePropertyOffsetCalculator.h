#ifndef DERIVED_PROPERTIES__FORMATION_SURFACE_PROPERTY_OFFSET_CALCULATOR_H
#define DERIVED_PROPERTIES__FORMATION_SURFACE_PROPERTY_OFFSET_CALCULATOR_H

#include <vector>
#include <string>

#include "AbstractProperty.h"
#include "AbstractSnapshot.h"
#include "AbstractSurface.h"

#include "AbstractPropertyManager.h"
#include "FormationSurfaceProperty.h"
#include "FormationSurfacePropertyCalculator.h"

namespace DerivedProperties {

   /// \brief Calculates a derived property or set of properties.
   ///
   /// Calcualtes the surface property from the formation property values.
   class FormationSurfacePropertyOffsetCalculator : public FormationSurfacePropertyCalculator {

   public :

      /// \brief Constructor.
      ///
      /// \param [in] property The proerty for which the calculator is to extract values.
      /// \pre property points to a valid property object.
      FormationSurfacePropertyOffsetCalculator ( const DataModel::AbstractProperty* property );

      /// \brief Calculate the property values and add the property values to the list.
      virtual void calculate ( AbstractPropertyManager&            propManager,
                               const DataModel::AbstractSnapshot*  snapshot,
                               const DataModel::AbstractFormation* formation,
                               const DataModel::AbstractSurface*   surface,
                                     FormationSurfacePropertyList& derivedProperties ) const;

   private :

      const DataModel::AbstractProperty* m_property;

   };


} // namespace DerivedProperties


#endif // DERIVED_PROPERTIES__FORMATION_SURFACE_PROPERTY_OFFSET_CALCULATOR_H
