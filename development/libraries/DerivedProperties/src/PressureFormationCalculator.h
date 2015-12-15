#ifndef DERIVED_PROPERTIES__PRESSURE_FORMATION_CALCULATOR_H
#define DERIVED_PROPERTIES__PRESSURE_FORMATION_CALCULATOR_H

#include "FormationPropertyCalculator.h"
#include "GeoPhysicsProjectHandle.h"
#include "GeoPhysicsFluidType.h"

#include "AbstractPropertyManager.h"
#include "DerivedFormationProperty.h"

namespace DerivedProperties {

 
   /// \brief Calculates the pore pressure for a formation.
   class PressureFormationCalculator : public FormationPropertyCalculator {

   public :

      PressureFormationCalculator ( const GeoPhysics::ProjectHandle* projectHandle );

      /// \brief Calculate the pore pressure for the formation at the snapshot age.
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

      const GeoPhysics::ProjectHandle* m_projectHandle;

   };


}

#endif // DERIVED_PROPERTIES__PRESSURE_FORMATION_CALCULATOR_H
