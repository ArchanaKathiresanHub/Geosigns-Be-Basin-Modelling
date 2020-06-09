//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "SurfaceProperty.h"
#include "Interpolate2DProperty.h"

AbstractDerivedProperties::SurfaceProperty::SurfaceProperty ( const DataModel::AbstractProperty* property,
                                                              const DataModel::AbstractSnapshot* snapshot,
                                                              const DataModel::AbstractSurface*  surface,
                                                              const DataModel::AbstractGrid*     grid ) :
   m_property ( property ),
   m_snapshot ( snapshot ),
   m_surface ( surface ),
   m_grid ( grid )
{
   // Should check that they aint null.
}

double AbstractDerivedProperties::SurfaceProperty::interpolate ( double i,
                                                                 double j ) const {

   Interpolate2DProperty<SurfaceProperty> interpolator;
   return interpolator.interpolate ( *this, i, j );
}

