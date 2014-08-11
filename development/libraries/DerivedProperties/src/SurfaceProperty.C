#include "SurfaceProperty.h"


DerivedProperties::SurfaceProperty::SurfaceProperty ( const DataModel::AbstractProperty* prop,
                                                      const DataModel::AbstractSnapshot* ss,
                                                      const DataModel::AbstractSurface*  surf,
                                                      const DataModel::AbstractGrid*     g ) :
   m_property ( prop ),
   m_snapshot ( ss ),
   m_surface ( surf ),
   m_grid ( g )
{
   // Should check that they aint null.
}
