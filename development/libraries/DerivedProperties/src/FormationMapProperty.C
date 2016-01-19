#include "FormationMapProperty.h"
#include "Interpolate2DProperty.h"


DerivedProperties::FormationMapProperty::FormationMapProperty ( const DataModel::AbstractProperty*  property,
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

double DerivedProperties::FormationMapProperty::interpolate ( double i,
                                                              double j ) const {

   Interpolate2DProperty<FormationMapProperty> interpolator;
   return interpolator.interpolate ( *this, i, j );
}

double DerivedProperties::FormationMapProperty::getA ( unsigned int i,
                                                       unsigned int j ) const {
   return get ( i, j );
}
