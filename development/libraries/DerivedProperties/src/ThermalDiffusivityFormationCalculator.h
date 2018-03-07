//
// Copyright (C) 2016-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef DERIVED_PROPERTIES__THERMAL_DIFFUSIVITY_CALCULATOR_H
#define DERIVED_PROPERTIES__THERMAL_DIFFUSIVITY_CALCULATOR_H

#include "AbstractSnapshot.h"
#include "AbstractFormation.h"

#include "FormationPropertyCalculator.h"
#include "GeoPhysicsProjectHandle.h"

namespace DerivedProperties {

   /// \brief Calculates the thermal diffusivity for a formation.
   class ThermalDiffusivityFormationCalculator : public AbstractDerivedProperties::FormationPropertyCalculator {

   public :

      ThermalDiffusivityFormationCalculator ( const GeoPhysics::ProjectHandle* projectHandle );
 
      /// \brief Calculate the thermal diffusivity for the formation.
      /// 
      /// \param [in]  propManager The property manager.
      /// \param [in]  snapshot    The snapshot time for which the thermal diffusivity is requested.
      /// \param [in]  formation   The formation for which the thermal diffusivity is requested.
      /// \param [out] derivedProperties On exit will contain the thermal diffusivity property for the formation.
      /// \pre snapshot points to a valid snapshot age.
      /// \pre formation points to a valid formation.
      virtual void calculate (       AbstractDerivedProperties::AbstractPropertyManager& propManager,
                               const DataModel::AbstractSnapshot*                        snapshot,
                               const DataModel::AbstractFormation*                       formation,
                                     AbstractDerivedProperties::FormationPropertyList&   derivedProperties ) const;
      /// \brief Determine if the property is computable for the specific combination of formation and snapshot.
      virtual bool isComputable ( const AbstractDerivedProperties::AbstractPropertyManager& propManager,
                                  const DataModel::AbstractSnapshot*                        snapshot,
                                  const DataModel::AbstractFormation*                       formation ) const;
      
      /// \brief Calculate the thermal conductivity for the basement formation.
      /// 
      /// \param [in]  propManager The property manager.
      /// \param [in]  snapshot    The snapshot time for which the thermal conductivity is requested.
      /// \param [in]  formation   The formation for which the thermal conductivity is requested.
      /// \param [out] derivedProperties On exit will contain the thermal conductivity property for the formation.
      /// \pre snapshot points to a valid snapshot age.
      /// \pre formation points to a valid formation.
      virtual void calculateForBasement ( AbstractDerivedProperties::AbstractPropertyManager& propManager,
                                          const DataModel::AbstractSnapshot*                  snapshot,
                                          const DataModel::AbstractFormation*                 formation,
                                          AbstractDerivedProperties::FormationPropertyList&   derivedProperties ) const;
 
   private :

      const GeoPhysics::ProjectHandle* m_projectHandle;
 
   };


}

#endif // DERIVED_PROPERTIES__THERMAL_DIFFUSIVITY_CALCULATOR_H
