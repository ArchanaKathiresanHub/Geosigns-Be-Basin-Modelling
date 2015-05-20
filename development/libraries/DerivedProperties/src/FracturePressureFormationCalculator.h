#ifndef DERIVED_PROPERTIES__FRACTURE_PRESSURE_CALCULATOR_H
#define DERIVED_PROPERTIES__FRACTURE_PRESSURE_CALCULATOR_H

#include "AbstractSnapshot.h"
#include "AbstractFormation.h"

#include "FormationPropertyCalculator.h"
#include "GeoPhysicsProjectHandle.h"

namespace DerivedProperties {

   /// \brief Calculates the fracture pressure for a formation.
   class FracturePressureFormationCalculator : public FormationPropertyCalculator {

   public :

      FracturePressureFormationCalculator ( const GeoPhysics::ProjectHandle* projectHandle );
 
      /// \brief Calculate the fracture pressure for the formation.
      /// 
      /// \param [in]  propManager The property manager.
      /// \param [in]  snapshot    The snapshot time for which the fracture pressure is requested.
      /// \param [in]  formation   The formation for which the fracture pressure is requested.
      /// \param [out] derivedProperties On exit will the fracture pressure property for the layer.
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

#endif // DERIVED_PROPERTIES__FRACTURE_PRESSURE_CALCULATOR_H
