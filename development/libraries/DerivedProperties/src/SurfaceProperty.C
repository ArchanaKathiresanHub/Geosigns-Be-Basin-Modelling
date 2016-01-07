#include "SurfaceProperty.h"
#include "Interpolate2DProperty.h"

DerivedProperties::SurfaceProperty::SurfaceProperty ( const DataModel::AbstractProperty* property,
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

double DerivedProperties::SurfaceProperty::interpolate ( double i,
                                                         double j ) const {

   Interpolate2DProperty<SurfaceProperty> interpolator;
   return interpolator.interpolate ( *this, i, j );
}

double DerivedProperties::SurfaceProperty::getA ( unsigned int i,
                                                  unsigned int j ) const {
   return get ( i, j );
}
