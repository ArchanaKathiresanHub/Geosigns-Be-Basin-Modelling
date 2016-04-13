//                                                                      
// Copyright (C) 2016-2016 Shell International Exploration & Production.
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
   class SonicFormationCalculator : public FormationPropertyCalculator {

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
      virtual void calculate ( AbstractPropertyManager&            propManager,
                               const DataModel::AbstractSnapshot*  snapshot,
                               const DataModel::AbstractFormation* formation,
                                     FormationPropertyList&        derivedProperties ) const;

   };


}

#endif // DERIVED_PROPERTIES__SONIC_CALCULATOR_H
