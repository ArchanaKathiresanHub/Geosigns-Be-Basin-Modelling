#ifndef DERIVED_PROPERTIES__HYDROSTATIC_PRESSURE_FORMATION_CALCULATOR_H
#define DERIVED_PROPERTIES__HYDROSTATIC_PRESSURE_FORMATION_CALCULATOR_H

#include "FormationPropertyCalculator.h"
#include "GeoPhysicsProjectHandle.h"
#include "GeoPhysicsFluidType.h"

#include "AbstractPropertyManager.h"
#include "DerivedFormationProperty.h"

namespace DerivedProperties {

 
   /// \brief Calculates the hydrostatic pressure for a formation.
   class HydrostaticPressureFormationCalculator : public FormationPropertyCalculator {

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
      virtual void calculate ( AbstractPropertyManager&            propertyManager,
                               const DataModel::AbstractSnapshot*  snapshot,
                               const DataModel::AbstractFormation* formation,
                                     FormationPropertyList&        derivedProperties ) const;

   private :

      /// \brief Compute the hydrostatic pressure at the top of the domain.
      void computeHydrostaticPressureAtSeaBottom ( const AbstractPropertyManager&     propertyManager,
                                                   const double                       snapshotAge,
                                                   const GeoPhysics::FluidType*       fluid,
                                                         DerivedFormationPropertyPtr& hydrostaticPressure ) const;

      /// \brief Copy the hydrostatic pressure from the formation directly above the surface.
      void copyHydrostaticPressureFromLayerAbove ( AbstractPropertyManager&            propertyManager,
                                                   const DataModel::AbstractProperty*  hydrostaticPressureProperty,
                                                   const DataModel::AbstractSnapshot*  snapshot,
                                                   const DataModel::AbstractFormation* formationAbove,
                                                         DerivedFormationPropertyPtr&  hydrostaticPressure ) const;

      const GeoPhysics::ProjectHandle* m_projectHandle;

      bool m_hydrostaticDecompactionMode;
      bool m_hydrostaticMode;

   };


}

#endif // DERIVED_PROPERTIES__HYDROSTATIC_PRESSURE_FORMATION_CALCULATOR_H
