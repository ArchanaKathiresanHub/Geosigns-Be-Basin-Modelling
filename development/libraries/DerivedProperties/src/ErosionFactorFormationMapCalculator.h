#ifndef DERIVED_PROPERTIES__EROSION_FACTOR_CALCULATOR_H
#define DERIVED_PROPERTIES__EROSION_FACTOR_CALCULATOR_H

#include "AbstractSnapshot.h"
#include "AbstractFormation.h"
#include "AbstractPropertyManager.h"

#include "FormationMapProperty.h"
#include "FormationMapPropertyCalculator.h"

namespace DerivedProperties {

   /// \brief Calculator for the erosion-factor of a layer.
   class ErosionFactorFormationMapCalculator : public FormationMapPropertyCalculator {

   public :

      ErosionFactorFormationMapCalculator ();
 
      /// \brief Calculate the velocity for the formation.
      /// 
      /// \param [in]  propManager The property manager.
      /// \param [in]  snapshot    The snapshot time for which the erosion-factor is requested.
      /// \param [in]  formation   The formation for which the erosion-factor is requested.
      /// \param [out] derivedProperties On exit will contain a single formation property, the erosion-factor in the layer.
      /// \pre snapshot points to a valid snapshot age.
      /// \pre formation points to a valid formation.
      virtual void calculate ( AbstractPropertyManager&            propManager,
                               const DataModel::AbstractSnapshot*  snapshot,
                               const DataModel::AbstractFormation* formation,
                                     FormationMapPropertyList&     derivedProperties ) const;

   };


}

#endif // DERIVED_PROPERTIES__EROSION_FACTOR_CALCULATOR_H
