//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef DERIVED_PROPERTIES__PRIMARY_FORMATION_PROPERTY_CALCULATOR_H
#define DERIVED_PROPERTIES__PRIMARY_FORMATION_PROPERTY_CALCULATOR_H

#include "AbstractProperty.h"
#include "AbstractSnapshot.h"
#include "AbstractSurface.h"

#include "GeoPhysicsProjectHandle.h"

#include "AbstractPropertyManager.h"
#include "FormationProperty.h"
#include "FormationPropertyCalculator.h"

#include <memory>
#include <string>
#include <vector>

namespace AbstractDerivedProperties {
   // Forward declaration of PropertyManager
   class AbstractPropertyManager;
}

namespace DerivedProperties {

   /// \brief Loads a primary property.
   ///
   /// A primary property is a property that has been computed by one of
   /// the simulators and saved to a file.
   class PrimaryFormationPropertyCalculator : public AbstractDerivedProperties::FormationPropertyCalculator {

   public :

      /// \brief Construct with the property whose values this calculator will load.
      PrimaryFormationPropertyCalculator ( const DataModel::AbstractProperty* property,
                                           const DataAccess::Interface::PropertyValueList& propertyValues );

      ~PrimaryFormationPropertyCalculator ();

      /// \brief Calculate the property values and add the property values to the list.
      virtual void calculate (       AbstractDerivedProperties::AbstractPropertyManager& propManager,
                               const DataModel::AbstractSnapshot*                        snapshot,
                               const DataModel::AbstractFormation*                       formation,
                                     AbstractDerivedProperties::FormationPropertyList&   derivedProperties ) const;

      /// \brief Determine if the property is computable for the specific combination of formatio nand snapshot.
      virtual bool isComputable ( const AbstractDerivedProperties::AbstractPropertyManager& propManager,
                                  const DataModel::AbstractSnapshot*                        snapshot,
                                  const DataModel::AbstractFormation*                       formation ) const;

   private :

      /// \brief The property.
      const DataModel::AbstractProperty*       m_property;

      /// \brief The list of all the property-values for the property.
      DataAccess::Interface::PropertyValueList m_formationPropertyValues;

   };

   typedef std::shared_ptr<PrimaryFormationPropertyCalculator> PrimaryFormationPropertyCalculatorPtr;


} // namespace DerivedProperties

#endif // DERIVED_PROPERTIES__PRIMARY_FORMATION_PROPERTY_CALCULATOR_H
