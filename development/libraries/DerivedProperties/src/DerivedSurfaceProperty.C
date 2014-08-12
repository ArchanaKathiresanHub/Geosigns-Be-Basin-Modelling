#include "DerivedSurfaceProperty.h"

DerivedProperties::DerivedSurfaceProperty::DerivedSurfaceProperty ( const DataModel::AbstractProperty* property,
                                                                    const DataModel::AbstractSnapshot* snapshot,
                                                                    const DataModel::AbstractSurface*  surface,
                                                                    const DataModel::AbstractGrid*     grid ) :
   SurfaceProperty  ( property, snapshot, surface, grid ),
   m_values ( boost::extents [grid->lastI(true) - grid->firstI(true) + 1][grid->lastJ(true) - grid->firstJ(true) + 1])
{
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
