//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef DERIVED_PROPERTIES__PERMEABITILY_CALCULATOR_H
#define DERIVED_PROPERTIES__PERMEABITILY_CALCULATOR_H

#include "FormationPropertyCalculator.h"
#include "GeoPhysicsProjectHandle.h"

namespace DerivedProperties {

   class PermeabilityFormationCalculator : public AbstractDerivedProperties::FormationPropertyCalculator {

   public :

      PermeabilityFormationCalculator ( const GeoPhysics::ProjectHandle* projectHandle );
 
      /// \brief Calculate the permeability for the formation.
      /// 
      /// \param [in]  propManager The property manager.
      /// \param [in]  snapshot    The snapshot time for which the permeability is requested.
      /// \param [in]  formation   The formation for which the permeability is requested.
      /// \param [out] derivedProperties On exit will contain two formation properties, the horizontal and vertical permeability of the layer.
      /// \pre snapshot points to a valid snapshot age.
      /// \pre formation points to a valid formation.
      virtual void calculate (       AbstractDerivedProperties::AbstractPropertyManager& propManager,
                               const DataModel::AbstractSnapshot*                        snapshot,
                               const DataModel::AbstractFormation*                       formation,
                                     AbstractDerivedProperties::FormationPropertyList&   derivedProperties ) const;

   private :

      const GeoPhysics::ProjectHandle* m_projectHandle;
      bool m_chemicalCompactionRequired;
   };

}

#endif 
