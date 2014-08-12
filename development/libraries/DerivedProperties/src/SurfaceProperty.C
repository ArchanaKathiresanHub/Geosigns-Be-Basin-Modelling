#include "SurfaceProperty.h"


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
