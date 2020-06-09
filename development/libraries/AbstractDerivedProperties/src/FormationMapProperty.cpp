//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "FormationMapProperty.h"
#include "Interpolate2DProperty.h"


AbstractDerivedProperties::FormationMapProperty::FormationMapProperty ( const DataModel::AbstractProperty*  property,
                                                                        const DataModel::AbstractSnapshot*  snapshot,
                                                                        const DataModel::AbstractFormation* formation,
                                                                        const DataModel::AbstractGrid*      grid ) :
   m_property ( property ),
   m_snapshot ( snapshot ),
   m_formation ( formation ),
   m_grid ( grid )
{
   // Should check that they aint null.
}

double AbstractDerivedProperties::FormationMapProperty::interpolate ( double i,
                                                                      double j ) const {

   Interpolate2DProperty<FormationMapProperty> interpolator;
   return interpolator.interpolate ( *this, i, j );
}
