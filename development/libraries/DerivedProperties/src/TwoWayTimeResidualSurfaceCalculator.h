//                                                                      
// Copyright (C) 2016-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#ifndef DERIVED_PROPERTIES__TWOWAYTIMERESIDUAL_SURFACE_CALCULATOR_H
#define DERIVED_PROPERTIES__TWOWAYTIMERESIDUAL_SURFACE_CALCULATOR_H

#include "SurfacePropertyCalculator.h"

namespace DerivedProperties {

   /// \brief Calculator for the two-way-(travel)-time residual of a surface.
   class TwoWayTimeResidualSurfaceCalculator : public SurfacePropertyCalculator {

   public :

      TwoWayTimeResidualSurfaceCalculator( const GeoPhysics::ProjectHandle* projectHandle);
 
      /// \brief Calculate the two-way-(travel)-time residual for the formation [ms].
      /// 
      /// \param [in]  propManager The property manager.
      /// \param [in]  snapshot    The snapshot time for which the two-way-time residual is requested.
      /// \param [in]  surface     The surface for which the two-way-time residual is requested.
      /// \param [out] derivedProperties On exit will contain a single surface property, the two-way-time residual at the surface.
      /// \pre snapshot points to present day 0.0Ma
      /// \pre surface  points to a valid surface.
      /// \pre surface has a initial two-way-(travel)-time linked to its stratigraphy
      virtual void calculate( AbstractPropertyManager&            propManager,
                              const DataModel::AbstractSnapshot*  snapshot,
                              const DataModel::AbstractSurface*   surface,
                                    SurfacePropertyList&          derivedProperties ) const;


      /// \brief Determine if the property is computable for the specific combination of surface and snapshot.
      /// \details Age must be 0Ma, intial input two-way-time must be linked to the surface in TwoWayTimeIoTbl
      ///    and cauldron output TwoWayTime must be computable.
      /// \param [in]  propManager The property manager.
      /// \param [in]  snapshot    The snapshot time for which the two-way-time residual is requested.
      /// \param [in]  surface     The surface for which the two-way-time residual is requested.
      /// \pre snapshot points to a valid snapshot age or is null.
      /// \pre surface points to a valid surface or is null.
      virtual bool isComputable( const AbstractPropertyManager&     propManager,
         const DataModel::AbstractSnapshot* snapshot,
         const DataModel::AbstractSurface*  surface ) const;

   private:

      const GeoPhysics::ProjectHandle* m_projectHandle;

   };


}

#endif // DERIVED_PROPERTIES__TWOWAYTIMERESIDUAL_SURFACE_CALCULATOR_H
