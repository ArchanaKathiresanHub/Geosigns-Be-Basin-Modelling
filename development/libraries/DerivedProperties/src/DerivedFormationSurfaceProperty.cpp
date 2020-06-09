//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "Interface.h"
#include "DerivedFormationSurfaceProperty.h"

using namespace AbstractDerivedProperties;

DerivedProperties::DerivedFormationSurfaceProperty::DerivedFormationSurfaceProperty ( const DataModel::AbstractProperty*  property,
                                                                                      const DataModel::AbstractSnapshot*  snapshot,
                                                                                      const DataModel::AbstractFormation* formation,
                                                                                      const DataModel::AbstractSurface*   surface,
                                                                                      const DataModel::AbstractGrid*      grid ) :
   FormationSurfaceProperty  ( property, snapshot, formation, surface, grid ),
   m_values ( boost::extents [grid->lastI(true) - grid->firstI(true) + 1][grid->lastJ(true) - grid->firstJ(true) + 1])
{
}

DerivedProperties::DerivedFormationSurfaceProperty::~DerivedFormationSurfaceProperty () {
   m_values.resize( boost::extents[0][0] );
}

double DerivedProperties::DerivedFormationSurfaceProperty::get ( unsigned int i,
                                                                 unsigned int j ) const {
   return m_values[ i - firstI(true)][ j - firstJ(true)];
}

void DerivedProperties::DerivedFormationSurfaceProperty::set ( unsigned int i,
                                                               unsigned int j,
                                                               double   value ) {
   m_values[ i - firstI(true)][ j - firstJ(true)] = value;
}

double DerivedProperties::DerivedFormationSurfaceProperty::getUndefinedValue () const {
   return DataAccess::Interface::DefaultUndefinedMapValue;
}

bool DerivedProperties::DerivedFormationSurfaceProperty::isPrimary() const {
   return false;
}
