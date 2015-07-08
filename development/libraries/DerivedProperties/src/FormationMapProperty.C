#include "FormationMapProperty.h"


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
