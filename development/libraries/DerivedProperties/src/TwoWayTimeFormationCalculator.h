//                                                                      
// Copyright (C) 2016-2018 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#ifndef DERIVED_PROPERTIES__TWOWAYTIME_FORMATION_CALCULATOR_H
#define DERIVED_PROPERTIES__TWOWAYTIME_FORMATION_CALCULATOR_H

#include "FormationPropertyCalculator.h"

namespace DerivedProperties {

   /// \brief Calculator for the two-way-(travel)-time of a layer.
   class TwoWayTimeFormationCalculator : public AbstractDerivedProperties::FormationPropertyCalculator {

   public :

      TwoWayTimeFormationCalculator();
 
      /// \brief Calculate the two-way-(travel)-time for the formation [ms].
      /// 
      /// \param [in]  propManager The property manager.
      /// \param [in]  snapshot    The snapshot time for which the two-way-time is requested.
      /// \param [in]  formation   The formation for which the two-way-time is requested.
      /// \param [out] derivedProperties On exit will contain a single formation property, the two-way-time in the layer.
      /// \pre snapshot points to a valid snapshot age.
      /// \pre formation points to a valid formation.
      virtual void calculate (       AbstractDerivedProperties::AbstractPropertyManager& propManager,
                               const DataModel::AbstractSnapshot*                        snapshot,
                               const DataModel::AbstractFormation*                       formation,
                                     AbstractDerivedProperties::FormationPropertyList&   derivedProperties ) const;

      /// \brief Determine if the property is computable for the specific combination of surface and snapshot.
      /// \details Age must be 0Ma and Depth,Temperature,Pressure,BulkDensity must be computable.
      /// \param [in]  propManager The property manager.
      /// \param [in]  snapshot    The snapshot time for which the two-way-time is requested.
      /// \param [in]  formation   The surface for which the two-way-time is requested.
      /// \pre snapshot points to a valid snapshot age or is null.
      /// \pre surface points to a valid surface or is null.
      virtual bool isComputable( const AbstractDerivedProperties::AbstractPropertyManager& propManager,
                                 const DataModel::AbstractSnapshot*                        snapshot,
                                 const DataModel::AbstractFormation*                       formation ) const;

   };


}

#endif // DERIVED_PROPERTIES__TWOWAYTIME_FORMATION_CALCULATOR_H
