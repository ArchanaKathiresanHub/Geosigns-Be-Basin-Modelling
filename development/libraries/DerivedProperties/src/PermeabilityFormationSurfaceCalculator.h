//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef DERIVED_PROPERTIES__PERMEABITILY_FORMATION_SURFACE_CALCULATOR_H
#define DERIVED_PROPERTIES__PERMEABITILY_FORMATION_SURFACE_CALCULATOR_H

#include "FormationSurfacePropertyCalculator.h"
#include "GeoPhysicsProjectHandle.h"

namespace DerivedProperties {

   class PermeabilityFormationSurfaceCalculator : public AbstractDerivedProperties::FormationSurfacePropertyCalculator {

   public :

      PermeabilityFormationSurfaceCalculator ( const GeoPhysics::ProjectHandle* projectHandle );

      virtual ~PermeabilityFormationSurfaceCalculator () = default;
 
      virtual void calculate (       AbstractDerivedProperties::AbstractPropertyManager&      propManager,
                               const DataModel::AbstractSnapshot*                             snapshot,
                               const DataModel::AbstractFormation*                            formation,
                               const DataModel::AbstractSurface*                              surface,
                                     AbstractDerivedProperties::FormationSurfacePropertyList& derivedProperties ) const;

   private :

      const GeoPhysics::ProjectHandle* m_projectHandle;

   };


}

#endif 
