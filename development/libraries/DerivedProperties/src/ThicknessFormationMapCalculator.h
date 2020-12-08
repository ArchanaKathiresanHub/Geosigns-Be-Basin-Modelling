//
// Copyright (C) 2016-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef DERIVED_PROPERTIES__THICKNESS_FORMATION_MAP_CALCULATOR_H
#define DERIVED_PROPERTIES__THICKNESS_FORMATION_MAP_CALCULATOR_H

#include <vector>
#include <string>

#include "AbstractProperty.h"
#include "AbstractSnapshot.h"
#include "AbstractFormation.h"
#include "AbstractPropertyManager.h"

#include "FormationMapProperty.h"
#include "FormationMapPropertyCalculator.h"

namespace DerivedProperties {

   /// \brief Calculates a derived property or set of properties.
   class ThicknessFormationMapCalculator : public AbstractDerivedProperties::FormationMapPropertyCalculator {

   public :

      /// Constructor.
      ThicknessFormationMapCalculator ();

      /// \brief Calculate the thickness derived property.
      ///
      /// \param [in]  propManager The property manager.
      /// \param [in]  snapshot    The snapshot time for which the thickness property is requested.
      /// \param [in]  formation   The formation for which the thickness property is requested.
      /// \param [out] derivedProperties On exit will contain a single formation-map property, the thickness of the layer.
      /// \pre snapshot points to a valid snapshot age.
      /// \pre formation points to a valid formation.
      virtual void calculate (       AbstractDerivedProperties::AbstractPropertyManager&  propManager,
                               const DataModel::AbstractSnapshot*                         snapshot,
                               const DataModel::AbstractFormation*                        formation,
                                     AbstractDerivedProperties::FormationMapPropertyList& derivedProperties ) const;

      /// \brief Determine if the property is computable for the specific combination of surface and snapshot.
      virtual bool isComputable ( const AbstractDerivedProperties::AbstractPropertyManager& propManager,
                                  const DataModel::AbstractSnapshot*                        snapshot,
                                  const DataModel::AbstractFormation*                       formation ) const;

   private:
      double calculateThickness(const double bottomDepth, const double topDepth) const;
   };

} // namespace DerivedProperties


#endif // DERIVED_PROPERTIES__THICKNESS_FORMATION_MAP_CALCULATOR_H
