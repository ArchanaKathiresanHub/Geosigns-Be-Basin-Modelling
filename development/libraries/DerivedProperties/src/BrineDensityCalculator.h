//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef DERIVED_PROPERTIES__BRINE_DENSITY_FORMATION_CALCULATOR_H
#define DERIVED_PROPERTIES__BRINE_DENSITY_FORMATION_CALCULATOR_H

#include "AbstractFormation.h"
#include "AbstractSnapshot.h"

#include "GeoPhysicsProjectHandle.h"

#include "AbstractPropertyManager.h"
#include "FormationPropertyCalculator.h"
#include "FormationProperty.h"

namespace GeoPhysics
{
  class FluidType;
}

namespace DerivedProperties {

   /// \brief Calculator for the brine-density for a layer.
   class BrineDensityCalculator : public AbstractDerivedProperties::FormationPropertyCalculator {

   public :

      BrineDensityCalculator ( const GeoPhysics::ProjectHandle& projectHandle );

      /// \brief Calculate the brine-density for the formation.
      ///
      /// \param [in]  propManager The property manager.
      /// \param [in]  snapshot    The snapshot time for which the brine-density is requested.
      /// \param [in]  formation   The formation for which the brine-density is requested.
      /// \param [out] derivedProperties On exit will contain a single formation property, the brine-density of the layer.
      /// \pre snapshot points to a valid snapshot age.
      /// \pre formation points to a valid formation.
      virtual void calculate (       AbstractDerivedProperties::AbstractPropertyManager& propManager,
                               const DataModel::AbstractSnapshot*                          snapshot,
                               const DataModel::AbstractFormation*                         formation,
                                     AbstractDerivedProperties::FormationPropertyList&     derivedProperties ) const;

      virtual double calculateAtPosition(const GeoPhysics::GeoPhysicsFormation* formation,
                                         const GeoPhysics::CompoundLithology* lithology,
                                         const std::map<std::string, double>& dependentProperties) const override;
   private :

      const GeoPhysics::ProjectHandle& m_projectHandle;
      bool m_hydrostaticMode;

      double calculateBrineDensity(const GeoPhysics::FluidType* fluid, const double temperature, const double porePressure) const;
      double calculateBrineDensityHydroStatic(const GeoPhysics::FluidType* fluid) const;
   };


}

#endif // DERIVED_PROPERTIES__BRINE_DENSITY_FORMATION_CALCULATOR_H
