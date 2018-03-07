//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef DERIVED_PROPERTIES__VES_FORMATION_CALCULATOR_H
#define DERIVED_PROPERTIES__VES_FORMATION_CALCULATOR_H

#include "FormationPropertyCalculator.h"

namespace DerivedProperties {

   /// \brief Calculator for VES (vertical effective stress).
   ///
   /// This is a derived property and is calculated by \f$ \sigma = p_l - p_p\f$
   class VesFormationCalculator : public AbstractDerivedProperties::FormationPropertyCalculator {

   public :

      VesFormationCalculator ();

      /// \brief Calculate the ves derived property.
      /// 
      /// \param [in]  propManager The property manager.
      /// \param [in]  snapshot    The snapshot time for which the ves property is requested.
      /// \param [in]  formation   The formation for which the ves property is requested.
      /// \param [out] derivedProperties On exit will contain a single formation property, the ves property values.
      /// \pre snapshot points to a valid snapshot age.
      /// \pre formation points to a valid formation.
      virtual void calculate (       AbstractDerivedProperties::AbstractPropertyManager& propManager,
                               const DataModel::AbstractSnapshot*                        snapshot,
                               const DataModel::AbstractFormation*                       formation,
                                     AbstractDerivedProperties::FormationPropertyList&   derivedProperties ) const;

    };


}

#endif 
