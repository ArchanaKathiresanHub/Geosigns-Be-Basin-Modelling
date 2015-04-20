#ifndef DERIVED_PROPERTIES__OVERPRESSURE_FORMATION_CALCULATOR_H
#define DERIVED_PROPERTIES__OVERPRESSURE_FORMATION_CALCULATOR_H

#include "FormationPropertyCalculator.h"

namespace DerivedProperties {

   /// \brief Calcualtor for the overpressure.
   class OverpressureFormationCalculator : public FormationPropertyCalculator {

   public :

      OverpressureFormationCalculator ();
 
      /// \brief Calculate the overpressure for the formation.
      /// 
      /// \param [in]  propManager The property manager.
      /// \param [in]  snapshot    The snapshot time for which the overpressure is requested.
      /// \param [in]  formation   The formation for which the overpressure is requested.
      /// \param [out] derivedProperties On exit will contain a single formation property, the overpressure of the layer.
      /// \pre snapshot points to a valid snapshot age.
      /// \pre formation points to a valid formation.
      virtual void calculate ( AbstractPropertyManager&            propManager,
                               const DataModel::AbstractSnapshot*  snapshot,
                               const DataModel::AbstractFormation* formation,
                                     FormationPropertyList&        derivedProperties ) const;

    };


}

#endif // DERIVED_PROPERTIES__OVERPRESSURE_FORMATION_CALCULATOR_H
