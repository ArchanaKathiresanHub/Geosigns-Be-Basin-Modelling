//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef DERIVED_PROPERTIES__LITHOSTATIC_PRESSURE_FORMATION_CALCULATOR_H
#define DERIVED_PROPERTIES__LITHOSTATIC_PRESSURE_FORMATION_CALCULATOR_H

#include "FormationPropertyCalculator.h"
#include "DerivedFormationProperty.h"

namespace DerivedProperties {

   /// \brief Calculator for the lithostatic-pressure for a layer.
   class LithostaticPressureFormationCalculator : public AbstractDerivedProperties::FormationPropertyCalculator {

   public :

      LithostaticPressureFormationCalculator ( const GeoPhysics::ProjectHandle* projectHandle );

      /// \brief Calculate the lithostatic-pressure for the formation.
      /// 
      /// \param [in]  propManager The property manager.
      /// \param [in]  snapshot    The snapshot time for which the lithostatic-pressure is requested.
      /// \param [in]  formation   The formation for which the lithostatic-pressure is requested.
      /// \param [out] derivedProperties On exit will contain a single formation property, the lithostatic-pressure of the layer.
      /// \pre snapshot points to a valid snapshot age.
      /// \pre formation points to a valid formation.
      virtual void calculate (       AbstractDerivedProperties::AbstractPropertyManager& propManager,
                               const DataModel::AbstractSnapshot*                        snapshot,
                               const DataModel::AbstractFormation*                       formation,
                                     AbstractDerivedProperties::FormationPropertyList&   derivedProperties ) const;

      /// \brief Calculate the lithostatic-pressure for the basement formation.
      /// 
      /// \param [in]  propManager The property manager.
      /// \param [in]  snapshot    The snapshot time for which the lithostatic-pressure is requested.
      /// \param [in]  formation   The formation for which the lithostatic-pressure is requested.
      /// \param [out] derivedProperties On exit will contain a single formation property, the lithostatic-pressure of the layer.
      /// \pre snapshot points to a valid snapshot age.
      /// \pre formation points to a valid formation.
      virtual void calculateForBasement (       AbstractDerivedProperties::AbstractPropertyManager& propManager,
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
   private :

      /// \brief Compute the lithostatic pressure at the top of the domain.
      void computeLithostaticPressureAtSeaBottom ( const AbstractDerivedProperties::AbstractPropertyManager& propertyManager,
                                                   const double                                              snapshotAge,
                                                         DerivedFormationPropertyPtr&                        lithostaticPressure ) const;

      /// \brief Copy the lithostatic pressure from the formation directly above the surface.
      void copyLithostaticPressureFromLayerAbove (       AbstractDerivedProperties::AbstractPropertyManager& propertyManager,
                                                   const DataModel::AbstractProperty*                        lithostaticPressureProperty,
                                                   const DataModel::AbstractSnapshot*                        snapshot,
                                                   const DataModel::AbstractFormation*                       formationAbove,
                                                         DerivedFormationPropertyPtr&                        lithostaticPressure ) const;

      const GeoPhysics::ProjectHandle* m_projectHandle;

      bool m_hydrostaticDecompactionMode;
   };


}

#endif 
