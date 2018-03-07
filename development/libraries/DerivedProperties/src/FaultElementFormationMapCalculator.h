//
// Copyright (C) 2016-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef DERIVED_PROPERTIES__FAULT_ELEMENT_FORMATION_MAP_CALCULATOR_H
#define DERIVED_PROPERTIES__FAULT_ELEMENT_FORMATION_MAP_CALCULATOR_H

#include <vector>
#include <string>

#include <boost/shared_ptr.hpp>

#include "AbstractProperty.h"
#include "AbstractSnapshot.h"
#include "AbstractFormation.h"
#include "AbstractPropertyManager.h"

#include "FormationMapProperty.h"
#include "FormationMapPropertyCalculator.h"

namespace DerivedProperties {

   /// \brief Calculates a map containing values indicating whether or not a location has faulted.
   class FaultElementFormationMapCalculator : public AbstractDerivedProperties::FormationMapPropertyCalculator {

   public :

      /// Constructor.
      FaultElementFormationMapCalculator ();

      /// \brief Calculate the property values and add the property values to the list.
      ///
      /// \param [in]  propManager The property manager.
      /// \param [in]  snapshot    The snapshot time for which the fault-element property is requested.
      /// \param [in]  formation   The formation for which the fault-element property is requested.
      /// \param [out] derivedProperties On exit will contain a single formation map property, the fault-element property values.
      /// \pre snapshot points to a valid snapshot age.
      /// \pre formation points to a valid formation.
      virtual void calculate (       AbstractDerivedProperties::AbstractPropertyManager&  propManager,
                               const DataModel::AbstractSnapshot*                         snapshot,
                               const DataModel::AbstractFormation*                        formation,
                                     AbstractDerivedProperties::FormationMapPropertyList& derivedProperties ) const;

   };

} // namespace DerivedProperties


#endif // DERIVED_PROPERTIES__FAULT_ELEMENT_FORMATION_MAP_CALCULATOR_H
