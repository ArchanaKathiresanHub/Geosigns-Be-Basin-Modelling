//
// Copyright (C) 2013-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef ABSTRACTDERIVED_PROPERTIES__SURFACE_PROPERTY_CALCULATOR_H
#define ABSTRACTDERIVED_PROPERTIES__SURFACE_PROPERTY_CALCULATOR_H

#include "AbstractProperty.h"
#include "AbstractSnapshot.h"
#include "AbstractSurface.h"

#include "SurfaceProperty.h"
#include "PropertyCalculator.h"

#include <memory>
#include <string>
#include <vector>

namespace AbstractDerivedProperties {

   // Forward declaration of PropertyManager
   class AbstractPropertyManager;

   /// \brief Calculates a derived property or set of properties.
   class SurfacePropertyCalculator : public PropertyCalculator {

   public :

      virtual ~SurfacePropertyCalculator () = default;

      /// \brief Calculate the property values and add the property values to the list.
      virtual void calculate ( AbstractPropertyManager&           propManager,
                               const DataModel::AbstractSnapshot* snapshot,
                               const DataModel::AbstractSurface*  surface,
                                     SurfacePropertyList&         derivedProperties ) const = 0;

      /// \brief Determine if the property is computable for the specific combination of surface and snapshot.
      virtual bool isComputable ( const AbstractPropertyManager&     propManager,
                                  const DataModel::AbstractSnapshot* snapshot,
                                  const DataModel::AbstractSurface*  surface ) const;

   };


   typedef std::shared_ptr<const SurfacePropertyCalculator> SurfacePropertyCalculatorPtr;


} // namespace AbstractDerivedProperties


#endif // ABSTRACTDERIVED_PROPERTIES__SURFACE_PROPERTY_CALCULATOR_H
