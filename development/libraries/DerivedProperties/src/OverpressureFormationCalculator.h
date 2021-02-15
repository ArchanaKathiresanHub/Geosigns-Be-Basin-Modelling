//
// Copyright (C) 2016-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef DERIVED_PROPERTIES__OVERPRESSURE_FORMATION_CALCULATOR_H
#define DERIVED_PROPERTIES__OVERPRESSURE_FORMATION_CALCULATOR_H

#include "FormationPropertyCalculator.h"

namespace DerivedProperties {

   /// \brief Calcualtor for the overpressure.
   class OverpressureFormationCalculator : public AbstractDerivedProperties::FormationPropertyCalculator {

   public :

      OverpressureFormationCalculator ();
 
      /// \brief Calculate the overpressure for the formation.
      /// 
      /// \param [in]  propManager The property manager.
      /// \param [in]  snapshot    The snapshot time for which the overpressure is requested.
      /// \param [in]  formation   The formation for which the overpressure is requested.
      /// \param [out] derivedProperties On exit will contain a single formation property, the overpressure of the layer.
      /// \pre snapshot points to a valid snapshot age.
      /// \pre formation points to a valid formation.
      virtual void calculate (       AbstractDerivedProperties::AbstractPropertyManager& propManager,
                               const DataModel::AbstractSnapshot*                        snapshot,
                               const DataModel::AbstractFormation*                       formation,
                                     AbstractDerivedProperties::FormationPropertyList&   derivedProperties ) const;

   private:
      double calculateOverpressure(const double porePressure, const double hydrostaticPressure) const;
   };


}

#endif // DERIVED_PROPERTIES__OVERPRESSURE_FORMATION_CALCULATOR_H
