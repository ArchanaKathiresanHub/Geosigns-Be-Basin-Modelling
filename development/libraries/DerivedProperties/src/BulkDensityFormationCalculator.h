//                                                                      
// Copyright (C) 2015-2015 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 
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
                               FormationPropertyList&              derivedProperties ) const;

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

      /// \brief Calculate the bulk density in the basement when ALC mode has not been enabled.
      void computeBulkDensityBasementNonAlc ( DerivedProperties::AbstractPropertyManager& propertyManager,
                                              const DataModel::AbstractSnapshot*          snapshot,
                                              const GeoPhysics::Formation*                formation,
                                              FormationPropertyList&                      derivedProperties ) const;

      /// \brief Calculate the bulk density in the basement when ALC mode has been enabled.
      void computeBulkDensityBasementAlc ( DerivedProperties::AbstractPropertyManager& propertyManager,
                                           const DataModel::AbstractSnapshot*          snapshot,
                                           const GeoPhysics::Formation*                formation,
                                           FormationPropertyList&                      derivedProperties ) const;

      /// \brief Calculate the bulk density in the sediments when the fastcauldron simulation mode was coupled.
      void computeBulkDensitySedimentsCoupled ( DerivedProperties::AbstractPropertyManager& propertyManager,
                                                const DataModel::AbstractSnapshot*          snapshot,
                                                const GeoPhysics::Formation*                formation,
                                                FormationPropertyList&                      derivedProperties ) const;

      /// \brief Calculate the bulk density in the sediments when the fastcauldron simulation mode was hydrostatic.
      void computeBulkDensitySedimentsHydrostatic ( DerivedProperties::AbstractPropertyManager& propertyManager,
                                                    const DataModel::AbstractSnapshot*          snapshot,
                                                    const GeoPhysics::Formation*                formation,
                                                    FormationPropertyList&                      derivedProperties ) const;

      /// \brief Used to indicate whether ALC mode was enabled or not.
      bool m_alcModeEnabled;

      /// \brief Used to indicate whether the fastcauldron simulation mode was coupled or not (hydrostatic).
      bool m_coupledModeEnabled;

   };


}

#endif // DERIVED_PROPERTIES__BULK_DENSITY_CALCULATOR_H
