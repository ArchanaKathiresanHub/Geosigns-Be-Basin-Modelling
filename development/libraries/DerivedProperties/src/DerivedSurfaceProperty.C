#include "DerivedSurfaceProperty.h"

DerivedProperties::DerivedSurfaceProperty::DerivedSurfaceProperty ( const DataModel::AbstractProperty* prop,
                                                                    const DataModel::AbstractSnapshot* ss,
                                                                    const DataModel::AbstractSurface*  surf,
                                                                    const DataModel::AbstractGrid*     g ) :
   SurfaceProperty  ( prop, ss, surf, g ),
   m_values ( boost::extents [g->lastI(true) - g->firstI(true) + 1][g->lastJ(true) - g->firstJ(true) + 1])
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
