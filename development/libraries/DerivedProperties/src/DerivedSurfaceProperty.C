//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include <algorithm>

#include "DerivedSurfaceProperty.h"
#include "Interface/Interface.h"

DerivedProperties::DerivedSurfaceProperty::DerivedSurfaceProperty ( const DataModel::AbstractProperty* property,
                                                                    const DataModel::AbstractSnapshot* snapshot,
                                                                    const DataModel::AbstractSurface*  surface,
                                                                    const DataModel::AbstractGrid*     grid ) :
   SurfaceProperty  ( property, snapshot, surface, grid ),
   m_values ( boost::extents [grid->lastI(true) - grid->firstI(true) + 1][grid->lastJ(true) - grid->firstJ(true) + 1])
{
}

DerivedProperties::DerivedSurfaceProperty::~DerivedSurfaceProperty () {
   m_values.resize( boost::extents[0][0] );
}

double DerivedProperties::DerivedSurfaceProperty::get ( unsigned int i,
                                                        unsigned int j ) const {
   return m_values[ i - firstI(true)][ j - firstJ(true)];
}

void DerivedProperties::DerivedSurfaceProperty::set ( unsigned int i,
                                                      unsigned int j,
                                                      double   value ) {

   m_values[ i - firstI(true)][ j - firstJ(true)] = value;
}

void DerivedProperties::DerivedSurfaceProperty::fill( const double value ) {
   std::fill( m_values.origin(), m_values.origin() + m_values.num_elements(), value );
}

double DerivedProperties::DerivedSurfaceProperty::getUndefinedValue () const {

   return DataAccess::Interface::DefaultUndefinedMapValue;
}
