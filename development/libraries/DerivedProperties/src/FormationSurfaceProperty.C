#include "FormationSurfaceProperty.h"

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
