#include "FormationSurfaceProperty.h"
#include "Interpolate2DProperty.h"

DerivedProperties::FormationSurfaceProperty::FormationSurfaceProperty ( const DataModel::AbstractProperty*  property,
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

double DerivedProperties::FormationSurfaceProperty::interpolate ( double i,
                                                                  double j ) const {

   Interpolate2DProperty<FormationSurfaceProperty> interpolator;
   return interpolator.interpolate ( *this, i, j );
}

double DerivedProperties::FormationSurfaceProperty::getA ( unsigned int i,
                                                           unsigned int j ) const {
   return get ( i, j );
}
