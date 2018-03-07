//
// Copyright (C) 2013-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef ABSTRACTDERIVED_PROPERTIES__FORMATION_MAP_PROPERTY_CALCULATOR_H
#define ABSTRACTDERIVED_PROPERTIES__FORMATION_MAP_PROPERTY_CALCULATOR_H

#include <vector>
#include <string>

#include <boost/shared_ptr.hpp>

#include "AbstractProperty.h"
#include "AbstractSnapshot.h"
#include "AbstractFormation.h"

#include "FormationMapProperty.h"
#include "PropertyCalculator.h"

namespace AbstractDerivedProperties {

   // Forward declaration of PropertyManager
   class AbstractPropertyManager;

   /// \brief Calculates a derived property or set of properties.
   class FormationMapPropertyCalculator : public PropertyCalculator {

   public :

      virtual ~FormationMapPropertyCalculator () = default;

      /// \brief Calculate the property values and add the property values to the list.
      virtual void calculate ( AbstractPropertyManager&            propManager,
                               const DataModel::AbstractSnapshot*  snapshot,
                               const DataModel::AbstractFormation* formation,
                                     FormationMapPropertyList&     derivedProperties ) const = 0;

      /// \brief Determine if the property is computable for the specific combination of formation and snapshot.
      virtual bool isComputable ( const AbstractPropertyManager&      propManager,
                                  const DataModel::AbstractSnapshot*  snapshot,
                                  const DataModel::AbstractFormation* formation ) const;

   };


   typedef boost::shared_ptr<const FormationMapPropertyCalculator> FormationMapPropertyCalculatorPtr;


} // namespace AbstractDerivedProperties


#endif // ABSTRACTDERIVED_PROPERTIES__FORMATION_MAP_PROPERTY_CALCULATOR_H
