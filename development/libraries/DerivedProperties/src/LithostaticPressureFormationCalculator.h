#ifndef DERIVED_PROPERTIES__LITHOSTATIC_PRESSURE_FORMATION_CALCULATOR_H
#define DERIVED_PROPERTIES__LITHOSTATIC_PRESSURE_FORMATION_CALCULATOR_H

#include "FormationPropertyCalculator.h"

namespace DerivedProperties {

   /// \brief Calculator for the lithostatic-pressure for a layer.
   class LithostaticPressureFormationCalculator : public FormationPropertyCalculator {

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
      virtual void calculate ( AbstractPropertyManager&            propManager,
                               const DataModel::AbstractSnapshot*  snapshot,
                               const DataModel::AbstractFormation* formation,
                                     FormationPropertyList&        derivedProperties ) const;

   private :

      const GeoPhysics::ProjectHandle* m_projectHandle;

   };


}

#endif 
