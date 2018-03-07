//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef DERIVED_PROPERTIES__HYDROSTATIC_PRESSURE_FORMATION_CALCULATOR_H
#define DERIVED_PROPERTIES__HYDROSTATIC_PRESSURE_FORMATION_CALCULATOR_H

#include "FormationPropertyCalculator.h"
#include "GeoPhysicsProjectHandle.h"
#include "GeoPhysicsFluidType.h"

#include "AbstractPropertyManager.h"
#include "DerivedFormationProperty.h"

namespace DerivedProperties {

 
   /// \brief Calculates the hydrostatic pressure for a formation.
   class HydrostaticPressureFormationCalculator : public AbstractDerivedProperties::FormationPropertyCalculator {

   public :

      HydrostaticPressureFormationCalculator ( const GeoPhysics::ProjectHandle* projectHandle );

      /// \brief Calculate the hydrostatic pressure for the formation at the snapshot age.
      ///
      /// \param [in]  propertyManager   Manager for all derived properties.
      /// \param [in]  snapshot          The snapshot at which the property is to be calculated.
      /// \param [in]  formation         The formation for which the property is to be calculated.
      /// \param [out] derivedProperties On exit will contain the newly computed hydrostatic pressure.
      /// \pre snapshot is not null and is a valid snapshot age.
      /// \pre formation is not null and is a valid formation.
      virtual void calculate (       AbstractDerivedProperties::AbstractPropertyManager& propertyManager,
                               const DataModel::AbstractSnapshot*                        snapshot,
                               const DataModel::AbstractFormation*                       formation,
                                     AbstractDerivedProperties::FormationPropertyList&   derivedProperties ) const;

      virtual bool isComputable ( const AbstractDerivedProperties::AbstractPropertyManager& propManager,
                                  const DataModel::AbstractSnapshot*                        snapshot,
                                  const DataModel::AbstractFormation*                       formation ) const;
   private :

      /// \brief Compute hydrostatic pressure after a fastcauldron hydrostatic decompaction simulation mode.
      void computeHydrostaticPressureForDecomapctionMode (       AbstractDerivedProperties::AbstractPropertyManager& propertyManager,
                                                           const DataModel::AbstractSnapshot*                        snapshot,
                                                           const DataModel::AbstractFormation*                       formation,
                                                                 AbstractDerivedProperties::FormationPropertyList&   derivedProperties ) const;

      /// \brief Compute hydrostatic pressure after a fastcauldron hydrostatic temperature simulation mode.
      void computeHydrostaticPressureForHydrostaticMode (       AbstractDerivedProperties::AbstractPropertyManager& propertyManager,
                                                          const DataModel::AbstractSnapshot*                        snapshot,
                                                          const DataModel::AbstractFormation*                       formation,
                                                                AbstractDerivedProperties::FormationPropertyList&   derivedProperties ) const;

      /// \brief Compute hydrostatic pressure after a fastcauldron coupled simulation mode.
      void computeHydrostaticPressureForCoupledMode (       AbstractDerivedProperties::AbstractPropertyManager& propertyManager,
                                                      const DataModel::AbstractSnapshot*                        snapshot,
                                                      const DataModel::AbstractFormation*                       formation,
                                                            AbstractDerivedProperties::FormationPropertyList&   derivedProperties ) const;

      /// \brief Compute hydrostatic pressure for the basement formation ( set to 0)
      void computeForBasement (       AbstractDerivedProperties::AbstractPropertyManager& propertyManager,
                                const DataModel::AbstractSnapshot*                        snapshot,
                                const DataModel::AbstractFormation*                       formation,
                                      AbstractDerivedProperties::FormationPropertyList&   derivedProperties ) const;

      /// \brief Compute the hydrostatic pressure at the top of the domain.
      void computeHydrostaticPressureAtSeaBottom ( const AbstractDerivedProperties::AbstractPropertyManager& propertyManager,
                                                   const double                                              snapshotAge,
                                                   const GeoPhysics::FluidType*                              fluid,
                                                         DerivedFormationPropertyPtr&                        hydrostaticPressure ) const;

     /// \brief Compute the hydrostatic pressure at the top of the domain.
      void computeHydrostaticPressureAtSeaBottomForHydrostatic ( const AbstractDerivedProperties::AbstractPropertyManager& propertyManager,
                                                                 const double                                              snapshotAge,
                                                                 const GeoPhysics::FluidType*                              fluid,
                                                                       DerivedFormationPropertyPtr&                        hydrostaticPressure ) const;

      /// \brief Copy the hydrostatic pressure from the formation directly above the surface.
      void copyHydrostaticPressureFromLayerAbove (       AbstractDerivedProperties::AbstractPropertyManager& propertyManager,
                                                   const DataModel::AbstractProperty*                        hydrostaticPressureProperty,
                                                   const DataModel::AbstractSnapshot*                        snapshot,
                                                   const DataModel::AbstractFormation*                       formationAbove,
                                                         DerivedFormationPropertyPtr&                        hydrostaticPressure ) const;

      void computeEstimatedTemperature ( const double                                           snapshotAge,
                                         const AbstractDerivedProperties::FormationPropertyPtr& depth,
                                               DerivedFormationPropertyPtr&                     temperature ) const;

      const GeoPhysics::ProjectHandle* m_projectHandle;

      bool m_hydrostaticDecompactionMode;
      bool m_hydrostaticMode;
      bool m_opMode;
   };


}

#endif // DERIVED_PROPERTIES__HYDROSTATIC_PRESSURE_FORMATION_CALCULATOR_H
