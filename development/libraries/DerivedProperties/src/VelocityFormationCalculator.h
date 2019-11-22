//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef DERIVED_PROPERTIES__VELOCITY_CALCULATOR_H
#define DERIVED_PROPERTIES__VELOCITY_CALCULATOR_H

#include "FormationPropertyCalculator.h"

namespace DerivedProperties {

   /// \brief Calculator for the velocity of a layer.
   class VelocityFormationCalculator : public AbstractDerivedProperties::FormationPropertyCalculator {

   public :

      VelocityFormationCalculator ();

      /// \brief Calculate the seismic velocity for the formation.
      ///
      /// \param [in]  propManager The property manager.
      /// \param [in]  snapshot    The snapshot time for which the seismic velocity is requested.
      /// \param [in]  formation   The formation for which the seismic velocity is requested.
      /// \param [out] derivedProperties On exit will contain a single formation property, the seismic velocity in the layer.
      /// \pre snapshot points to a valid snapshot age.
      /// \pre formation points to a valid formation.
      virtual void calculate (       AbstractDerivedProperties::AbstractPropertyManager& propManager,
                               const DataModel::AbstractSnapshot*                        snapshot,
                               const DataModel::AbstractFormation*                       formation,
                                     AbstractDerivedProperties::FormationPropertyList&   derivedProperties ) const;

      void calculateForBasement ( AbstractDerivedProperties::AbstractPropertyManager& propManager,
                                  const DataModel::AbstractSnapshot*                  snapshot,
                                  const DataModel::AbstractFormation*                 formation,
                                  AbstractDerivedProperties::FormationPropertyList&   derivedProperties ) const;

      virtual bool isComputable ( const AbstractDerivedProperties::AbstractPropertyManager& propManager,
                                  const DataModel::AbstractSnapshot*                        snapshot,
                                  const DataModel::AbstractFormation*                       formation ) const;

   };


}

#endif // DERIVED_PROPERTIES__VELOCITY_CALCULATOR_H
