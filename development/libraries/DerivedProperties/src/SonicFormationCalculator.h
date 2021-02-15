//                                                                      
// Copyright (C) 2016-2018 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef DERIVED_PROPERTIES__SONIC_CALCULATOR_H
#define DERIVED_PROPERTIES__SONIC_CALCULATOR_H

#include "FormationPropertyCalculator.h"

namespace DerivedProperties {

   /// \brief Calculator for the velocity of a layer.
   class SonicFormationCalculator : public AbstractDerivedProperties::FormationPropertyCalculator {

   public :

      SonicFormationCalculator ();
 
      /// \brief Calculate the sonic slowness (Vp) for the formation.
      /// 
      /// \param [in]  propManager The property manager.
      /// \param [in]  snapshot    The snapshot time for which the sonic slowness is requested.
      /// \param [in]  formation   The formation for which the sonic slowness is requested.
      /// \param [out] derivedProperties On exit will contain a single formation property, the sonic slowness in the layer.
      /// \pre snapshot points to a valid snapshot age.
      /// \pre formation points to a valid formation.
      virtual void calculate (       AbstractDerivedProperties::AbstractPropertyManager& propManager,
                               const DataModel::AbstractSnapshot*                        snapshot,
                               const DataModel::AbstractFormation*                       formation,
                                     AbstractDerivedProperties::FormationPropertyList&   derivedProperties ) const;

      virtual double calculateAtPosition( const GeoPhysics::GeoPhysicsFormation* formation,
                                          const GeoPhysics::CompoundLithology* lithology,
                                          const std::map<std::string, double>& dependentProperties ) const override;

   private:
      double calculateSonic(double velocity) const;
   };


}

#endif // DERIVED_PROPERTIES__SONIC_CALCULATOR_H
