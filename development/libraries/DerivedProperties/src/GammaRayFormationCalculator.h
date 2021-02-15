// 
// Copyright (C) 2017-2018 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef DERIVED_PROPERTIES__GAMMARAY_FORMATION_CALCULATOR_H
#define DERIVED_PROPERTIES__GAMMARAY_FORMATION_CALCULATOR_H

#include "FormationPropertyCalculator.h"

namespace DerivedProperties {

   /// \brief Calculator for the gamma ray of a layer.
   class GammaRayFormationCalculator : public AbstractDerivedProperties::FormationPropertyCalculator {

   public :

      GammaRayFormationCalculator( );
 
      /// \brief Calculate the gamma ray value for the formation [API].
      /// 
      /// \param [in]  propManager The property manager.
      /// \param [in]  snapshot    The snapshot time for which the gamma ray is requested.
      /// \param [in]  formation   The formation for which the gamma ray is requested.
      /// \param [out] derivedProperties On exit will contain a single formation property, the gamma ray in the layer.
      /// \pre snapshot points to a valid snapshot age.
      /// \pre formation points to a valid formation.
      void calculate ( AbstractDerivedProperties::AbstractPropertyManager& propManager,
                       const DataModel::AbstractSnapshot*                  snapshot,
                       const DataModel::AbstractFormation*                 formation,
                       AbstractDerivedProperties::FormationPropertyList&   derivedProperties ) const final;

      /// \brief Determine if the property is computable for the specific combination of surface and snapshot.
      /// \param [in]  propManager The property manager.
      /// \param [in]  snapshot    The snapshot time for which the gamma ray is requested.
      /// \param [in]  formation   The surface for which the gamma ray is requested.
      /// \pre snapshot points to a valid snapshot age or is null.
      /// \pre surface points to a valid surface or is null.
      bool isComputable( const AbstractDerivedProperties::AbstractPropertyManager& propManager,
                         const DataModel::AbstractSnapshot*                        snapshot,
                         const DataModel::AbstractFormation*                       formation ) const final;

      virtual double calculateAtPosition( const GeoPhysics::GeoPhysicsFormation* formation,
                                          const GeoPhysics::CompoundLithology* lithology,
                                          const std::map<std::string, double>& dependentProperties) const override;
   private:
      const double m_gammaRayScaleFactor=0.0158;
      const double m_gammaRayOffset=0.8;

      double calculateGammaRay(const double porosity, const double solidRadiogenicHeatProduction) const;
   };


}

#endif // DERIVED_PROPERTIES__GAMMARAY_FORMATION_CALCULATOR_H
