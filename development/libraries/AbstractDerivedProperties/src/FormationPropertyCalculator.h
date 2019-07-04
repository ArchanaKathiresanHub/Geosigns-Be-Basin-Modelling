//
// Copyright (C) 2017-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef ABSTRACTDERIVED_PROPERTIES__FORMATION_PROPERTY_CALCULATOR_H
#define ABSTRACTDERIVED_PROPERTIES__FORMATION_PROPERTY_CALCULATOR_H

#include "AbstractSnapshot.h"

#include "FormationProperty.h"

#include "PropertyCalculator.h"

#include <memory>
#include <vector>

namespace AbstractDerivedProperties {

   // Forward declaration of PropertyManager
   class AbstractPropertyManager;

   class Elt2dIndices
   {
   public:

      bool exists;
      int i[4];
      int j[4];

      bool nodeDefined[4];
   };

   typedef std::vector < Elt2dIndices > ElementList;

   /// \brief Calculates a derived property or set of properties.
   class FormationPropertyCalculator : public PropertyCalculator {

   public :

      virtual ~FormationPropertyCalculator() = default;

      /// \brief Calculate the property values and add the property values to the list.
      virtual void calculate ( AbstractPropertyManager&            propManager,
                               const DataModel::AbstractSnapshot*  snapshot,
                               const DataModel::AbstractFormation* formation,
                                     FormationPropertyList&        derivedProperties ) const = 0;

      /// \brief Determine if the property is computable for the specific combination of formation and snapshot.
      virtual bool isComputable ( const AbstractPropertyManager&      propManager,
                                  const DataModel::AbstractSnapshot*  snapshot,
                                  const DataModel::AbstractFormation* formation ) const;

      virtual void setUp2dEltMapping( AbstractPropertyManager& propManager, const FormationPropertyPtr aProperty, ElementList & mapElementList ) const;

    };


   typedef std::shared_ptr<const FormationPropertyCalculator> FormationPropertyCalculatorPtr;


} // namespace AbstractDerivedProperties


#endif // ABSTRACTDERIVED_PROPERTIES__FORMATION_PROPERTY_CALCULATOR_H
