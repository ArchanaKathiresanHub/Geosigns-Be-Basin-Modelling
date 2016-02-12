#ifndef DERIVED_PROPERTIES__THERMAL_CONDUCTIVITY_CALCULATOR_H
#define DERIVED_PROPERTIES__THERMAL_CONDUCTIVITY_CALCULATOR_H

#include "AbstractSnapshot.h"
#include "AbstractFormation.h"

#include "FormationPropertyCalculator.h"
#include "GeoPhysicsProjectHandle.h"

namespace DerivedProperties {

   /// \brief Calculates the thermal conductivity for a formation.
   class ThermalConductivityFormationCalculator : public FormationPropertyCalculator {

   public :

      ThermalConductivityFormationCalculator ( const GeoPhysics::ProjectHandle* projectHandle );
 
      /// \brief Calculate the thermal conductivity for the formation.
      /// 
      /// \param [in]  propManager The property manager.
      /// \param [in]  snapshot    The snapshot time for which the thermal conductivity is requested.
      /// \param [in]  formation   The formation for which the thermal conductivity is requested.
      /// \param [out] derivedProperties On exit will contain the thermal conductivity property for the formation.
      /// \pre snapshot points to a valid snapshot age.
      /// \pre formation points to a valid formation.
      virtual void calculate ( AbstractPropertyManager&            propManager,
                               const DataModel::AbstractSnapshot*  snapshot,
                               const DataModel::AbstractFormation* formation,
                                     FormationPropertyList&        derivedProperties ) const;

      /// \brief Calculate the thermal conductivity for the basement formation.
      /// 
      /// \param [in]  propManager The property manager.
      /// \param [in]  snapshot    The snapshot time for which the thermal conductivity is requested.
      /// \param [in]  formation   The formation for which the thermal conductivity is requested.
      /// \param [out] derivedProperties On exit will contain the thermal conductivity property for the formation.
      /// \pre snapshot points to a valid snapshot age.
      /// \pre formation points to a valid formation.
      virtual void calculateForBasement ( AbstractPropertyManager&            propManager,
                                          const DataModel::AbstractSnapshot*  snapshot,
                                          const DataModel::AbstractFormation* formation,
                                          FormationPropertyList&        derivedProperties ) const;
      
       /// \brief Determine if the property is computable for the specific combination of formation and snapshot.
      virtual bool isComputable ( const AbstractPropertyManager&      propManager,
                                  const DataModel::AbstractSnapshot*  snapshot,
                                  const DataModel::AbstractFormation* formation ) const;

       /// \brief Determine if the property is computable for the specific combination of basement formation and snapshot.
      virtual bool isComputableForBasement ( const AbstractPropertyManager&      propManager,
                                             const DataModel::AbstractSnapshot*  snapshot,
                                             const DataModel::AbstractFormation* formation ) const;
   private :

      const GeoPhysics::ProjectHandle* m_projectHandle;

   };


}

#endif // DERIVED_PROPERTIES__THERMAL_CONDUCTIVITY_CALCULATOR_H
