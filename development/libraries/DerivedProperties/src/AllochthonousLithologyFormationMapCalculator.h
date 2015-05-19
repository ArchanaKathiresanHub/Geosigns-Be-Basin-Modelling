#ifndef DERIVED_PROPERTIES__ALLOCHTHONOUS_LITHOLOGY_FORMATION_MAP_CALCULATOR_H
#define DERIVED_PROPERTIES__ALLOCHTHONOUS_LITHOLOGY_FORMATION_MAP_CALCULATOR_H

#include <vector>
#include <string>

#include <boost/shared_ptr.hpp>

#include "AbstractSnapshot.h"
#include "AbstractFormation.h"
#include "AbstractPropertyManager.h"

#include "FormationMapProperty.h"
#include "FormationMapPropertyCalculator.h"

namespace DerivedProperties {

   /// \brief Calculates a derived property or set of properties.
   class AllochthonousLithologyFormationMapCalculator : public FormationMapPropertyCalculator {

   public :

      /// Constructor.
      AllochthonousLithologyFormationMapCalculator ();

      /// \brief Calculate the allochthonous lithology derived property.
      /// 
      /// \param [in]  propManager The property manager.
      /// \param [in]  snapshot    The snapshot time for which the allochthonous lithology property is requested.
      /// \param [in]  formation   The formation for which the allochthonous lithology property is requested.
      /// \param [out] derivedProperties On exit will contain a single formation-map property, the allochthonous lithology indicator of the layer.
      /// \pre snapshot points to a valid snapshot age.
      /// \pre formation points to a valid formation.
      ///
      /// The floating point number 1.0 indicates that the lithology at a position is defined as allochthonous, 0.0 if it is not.
      /// The undefined value indicated undefined regions.
      virtual void calculate ( AbstractPropertyManager&            propManager,
                               const DataModel::AbstractSnapshot*  snapshot,
                               const DataModel::AbstractFormation* formation,
                                     FormationMapPropertyList&     derivedProperties ) const;

   };

} // namespace DerivedProperties


#endif // DERIVED_PROPERTIES__ALLOCHTHONOUS_LITHOLOGY_FORMATION_MAP_CALCULATOR_H
