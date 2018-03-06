#ifndef DERIVED_PROPERTIES__PERMEABITILY_CALCULATOR_H
#define DERIVED_PROPERTIES__PERMEABITILY_CALCULATOR_H

#include "FormationPropertyCalculator.h"
#include "GeoPhysicsProjectHandle.h"

namespace DerivedProperties {

   class PermeabilityFormationCalculator : public FormationPropertyCalculator {

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
      virtual void calculate ( AbstractPropertyManager&            propManager,
                               const DataModel::AbstractSnapshot*  snapshot,
                               const DataModel::AbstractFormation* formation,
                                     FormationPropertyList&        derivedProperties ) const;

   private :

      const GeoPhysics::ProjectHandle* m_projectHandle;
      bool m_chemicalCompactionRequired;
   };


}

#endif 
