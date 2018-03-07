//
// Copyright (C) 2013-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "FormationSurfaceProperty.h"
#include "Interpolate2DProperty.h"

AbstractDerivedProperties::FormationSurfaceProperty::FormationSurfaceProperty ( const DataModel::AbstractProperty*  property,
                                                                                const DataModel::AbstractSnapshot*  snapshot,
                                                                                const DataModel::AbstractFormation* formation,
                                                                                const DataModel::AbstractSurface*   surface,
                                                                                const DataModel::AbstractGrid*      grid ) :
   m_property ( property ),
   m_snapshot ( snapshot ),
   m_surface ( surface ),
   m_formation ( formation ),
   m_grid ( grid )
{
}

double AbstractDerivedProperties::FormationSurfaceProperty::interpolate ( double i,
                                                                          double j ) const {

   Interpolate2DProperty<FormationSurfaceProperty> interpolator;
   return interpolator.interpolate ( *this, i, j );
}
