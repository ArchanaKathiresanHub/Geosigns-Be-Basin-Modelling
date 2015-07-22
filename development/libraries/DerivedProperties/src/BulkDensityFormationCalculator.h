#ifndef DERIVED_PROPERTIES__BULK_DENSITY_CALCULATOR_H
#define DERIVED_PROPERTIES__BULK_DENSITY_CALCULATOR_H

#include "FormationPropertyCalculator.h"
#include "GeoPhysicsProjectHandle.h"

namespace DerivedProperties {

   /// \brief Calculator for the bulk density of a layer.
   class BulkDensityFormationCalculator : public FormationPropertyCalculator {

   public :

      BulkDensityFormationCalculator ( const GeoPhysics::ProjectHandle* projectHandle );
 
      /// \brief Calculate the bulk density for the formation.
      /// 
      /// \param [in]  propManager The property manager.
      /// \param [in]  snapshot    The snapshot time for which the bulk density is requested.
      /// \param [in]  formation   The formation for which the bulk density is requested.
      /// \param [out] derivedProperties On exit will contain a single formation property, the bulk density in the layer.
      /// \pre snapshot points to a valid snapshot age.
      /// \pre formation points to a valid formation.
      virtual void calculate ( AbstractPropertyManager&            propManager,
                               const DataModel::AbstractSnapshot*  snapshot,
                               const DataModel::AbstractFormation* formation,
                                     FormationPropertyList&        derivedProperties ) const;

   private :

      const GeoPhysics::ProjectHandle* m_projectHandle;

      void computeBulkDensityBasementNonAlc ( DerivedProperties::AbstractPropertyManager& propertyManager,
                                              const DataModel::AbstractSnapshot*          snapshot,
                                              const GeoPhysics::Formation*                formation,
                                                    FormationPropertyList&                derivedProperties ) const;

      void computeBulkDensityBasementAlc ( DerivedProperties::AbstractPropertyManager& propertyManager,
                                           const DataModel::AbstractSnapshot*          snapshot,
                                           const GeoPhysics::Formation*                formation,
                                                 FormationPropertyList&                derivedProperties ) const;

      void computeBulkDensitySedimentsCoupled ( DerivedProperties::AbstractPropertyManager& propertyManager,
                                                const DataModel::AbstractSnapshot*          snapshot,
                                                const GeoPhysics::Formation*                formation,
                                                      FormationPropertyList&                derivedProperties ) const;

      void computeBulkDensitySedimentsHydrostatic ( DerivedProperties::AbstractPropertyManager& propertyManager,
                                                    const DataModel::AbstractSnapshot*          snapshot,
                                                    const GeoPhysics::Formation*                formation,
                                                          FormationPropertyList&                derivedProperties ) const;

      bool m_alcModeEnabled;
      bool m_coupledModeEnabled;

   };


}

#endif // DERIVED_PROPERTIES__BULK_DENSITY_CALCULATOR_H
