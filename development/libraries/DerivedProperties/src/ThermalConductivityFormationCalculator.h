//
// Copyright (C) 2016-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef DERIVED_PROPERTIES__THERMAL_CONDUCTIVITY_CALCULATOR_H
#define DERIVED_PROPERTIES__THERMAL_CONDUCTIVITY_CALCULATOR_H

#include "AbstractSnapshot.h"
#include "AbstractFormation.h"

#include "FormationPropertyCalculator.h"
#include "GeoPhysicsProjectHandle.h"

namespace GeoPhysics
{
  class CompoundLithology;
  class FluidType;
}

namespace DerivedProperties {

   /// \brief Calculates the thermal conductivity for a formation.
   class ThermalConductivityFormationCalculator : public AbstractDerivedProperties::FormationPropertyCalculator {

   public :

      ThermalConductivityFormationCalculator ( const GeoPhysics::ProjectHandle& projectHandle );

      /// \brief Calculate the thermal conductivity for the formation.
      ///
      /// \param [in]  propManager The property manager.
      /// \param [in]  snapshot    The snapshot time for which the thermal conductivity is requested.
      /// \param [in]  formation   The formation for which the thermal conductivity is requested.
      /// \param [out] derivedProperties On exit will contain the thermal conductivity property for the formation.
      /// \pre snapshot points to a valid snapshot age.
      /// \pre formation points to a valid formation.
      virtual void calculate (       AbstractDerivedProperties::AbstractPropertyManager&            propManager,
                               const DataModel::AbstractSnapshot*  snapshot,
                               const DataModel::AbstractFormation* formation,
                                     AbstractDerivedProperties::FormationPropertyList&        derivedProperties ) const;

      /// \brief Calculate the thermal conductivity for the basement formation.
      ///
      /// \param [in]  propManager The property manager.
      /// \param [in]  snapshot    The snapshot time for which the thermal conductivity is requested.
      /// \param [in]  formation   The formation for which the thermal conductivity is requested.
      /// \param [out] derivedProperties On exit will contain the thermal conductivity property for the formation.
      /// \pre snapshot points to a valid snapshot age.
      /// \pre formation points to a valid formation.
      void calculateForBasement (       AbstractDerivedProperties::AbstractPropertyManager& propManager,
                                  const DataModel::AbstractSnapshot*                        snapshot,
                                  const DataModel::AbstractFormation*                       formation,
                                        AbstractDerivedProperties::FormationPropertyList&   derivedProperties ) const;

       /// \brief Determine if the property is computable for the specific combination of formation and snapshot.
      virtual bool isComputable ( const AbstractDerivedProperties::AbstractPropertyManager& propManager,
                                  const DataModel::AbstractSnapshot*                        snapshot,
                                  const DataModel::AbstractFormation*                       formation ) const;

       /// \brief Determine if the property is computable for the specific combination of basement formation and snapshot.
      virtual bool isComputableForBasement ( const AbstractDerivedProperties::AbstractPropertyManager& propManager,
                                             const DataModel::AbstractSnapshot*                        snapshot,
                                             const DataModel::AbstractFormation*                       formation ) const;

      virtual double calculateAtPosition( const GeoPhysics::GeoPhysicsFormation* formation,
                                          const GeoPhysics::CompoundLithology* lithology,
                                          const std::map<std::string, double>& dependentProperties) const override;

   private :

      double calculateThermalConductivity(  const GeoPhysics::CompoundLithology *lithology, const GeoPhysics::FluidType *fluid,
                                           const double porosity, const double temperature, const double porePressure ) const;
      const GeoPhysics::ProjectHandle& m_projectHandle;

   };


}

#endif // DERIVED_PROPERTIES__THERMAL_CONDUCTIVITY_CALCULATOR_H
