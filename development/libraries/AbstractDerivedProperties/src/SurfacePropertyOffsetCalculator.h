//
// Copyright (C) 2013-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef ABSTRACTDERIVED_PROPERTIES__SURFACE_PROPERTY_OFFSET_CALCULATOR_H
#define ABSTRACTDERIVED_PROPERTIES__SURFACE_PROPERTY_OFFSET_CALCULATOR_H

#include <vector>
#include <string>

#include "AbstractProperty.h"
#include "AbstractSnapshot.h"
#include "AbstractSurface.h"
#include "AbstractFormation.h"

#include "AbstractPropertyManager.h"
#include "SurfaceProperty.h"
#include "SurfacePropertyCalculator.h"

namespace AbstractDerivedProperties {

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
      /// \pre The propManager references to a valid property manager object.
      /// \pre The snapshot points to a valid snapshot object or is null.
      /// \pre The surface points to a valid surface object or is null.
      virtual bool isComputable ( const AbstractPropertyManager&      propManager,
                                  const DataModel::AbstractSnapshot*  snapshot,
                                  const DataModel::AbstractSurface*   surface ) const;

   private :

      /// \brief Get the formation connected to the surface.
      ///
      /// Always get the formation below unless it does not exist or is the crust.
      /// \param [in] surface The surface for which the connecting formation is sought.
      /// \pre surface points to a valid surface object.
      const DataModel::AbstractFormation* getAdjacentFormation ( const DataModel::AbstractSurface* surface, const bool useBottom ) const;

      /// \brief The property calculated by this calculator.
      const DataModel::AbstractProperty* m_property;

   };


} // namespace AbstractDerivedProperties


#endif // ABSTRACTDERIVED_PROPERTIES__SURFACE_PROPERTY_OFFSET_CALCULATOR_H
