#ifndef DERIVED_PROPERTIES__VELOCITY_CALCULATOR_H
#define DERIVED_PROPERTIES__VELOCITY_CALCULATOR_H

#include "FormationPropertyCalculator.h"

namespace DerivedProperties {

   /// \brief Calculator for the velocity of a layer.
   class VelocityFormationCalculator : public FormationPropertyCalculator {

   public :

      VelocityFormationCalculator ();
 
      /// \brief Calculate the velocity for the formation.
      /// 
      /// \param [in]  propManager The property manager.
      /// \param [in]  snapshot    The snapshot time for which the velocity is requested.
      /// \param [in]  formation   The formation for which the velocity is requested.
      /// \param [out] derivedProperties On exit will contain a single formation property, the velocity in the layer.
      /// \pre snapshot points to a valid snapshot age.
      /// \pre formation points to a valid formation.
      virtual void calculate ( AbstractPropertyManager&            propManager,
                               const DataModel::AbstractSnapshot*  snapshot,
                               const DataModel::AbstractFormation* formation,
                                     FormationPropertyList&        derivedProperties ) const;

   };


}

#endif // DERIVED_PROPERTIES__VELOCITY_CALCULATOR_H
