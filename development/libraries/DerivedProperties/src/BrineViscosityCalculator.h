#ifndef DERIVED_PROPERTIES__BRINE_VISCOSITY_FORMATION_CALCULATOR_H
#define DERIVED_PROPERTIES__BRINE_VISCOSITY_FORMATION_CALCULATOR_H

#include "AbstractFormation.h"
#include "AbstractSnapshot.h"

#include "GeoPhysicsProjectHandle.h"

#include "AbstractPropertyManager.h"
#include "FormationPropertyCalculator.h"
#include "FormationProperty.h"

namespace DerivedProperties {

   /// \brief Calculator for the brine-viscosity for a layer.
   class BrineViscosityCalculator : public FormationPropertyCalculator {

   public :

      BrineViscosityCalculator ( const GeoPhysics::ProjectHandle* projectHandle );

      /// \brief Calculate the brine-viscosity for the formation.
      /// 
      /// \param [in]  propManager The property manager.
      /// \param [in]  snapshot    The snapshot time for which the brine-viscosity is requested.
      /// \param [in]  formation   The formation for which the brine-viscosity is requested.
      /// \param [out] derivedProperties On exit will contain a single formation property, the brine-viscosity of the layer.
      /// \pre snapshot points to a valid snapshot age.
      /// \pre formation points to a valid formation.
      virtual void calculate ( AbstractPropertyManager&            propManager,
                               const DataModel::AbstractSnapshot*  snapshot,
                               const DataModel::AbstractFormation* formation,
                                     FormationPropertyList&        derivedProperties ) const;

   private :

      const GeoPhysics::ProjectHandle* m_projectHandle;
      bool m_hydrostaticMode;

   };


}

#endif // DERIVED_PROPERTIES__BRINE_VISCOSITY_FORMATION_CALCULATOR_H
