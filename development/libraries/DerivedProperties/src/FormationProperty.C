#include "FormationProperty.h"

DerivedProperties::FormationProperty::FormationProperty ( const DataModel::AbstractProperty*  property,
                                                          const DataModel::AbstractSnapshot*  snapshot,
                                                          const DataModel::AbstractFormation* formation,
                                                          const DataModel::AbstractGrid*      grid,
                                                          const unsigned int                  nk ) :
   m_property ( property ),
   m_snapshot ( snapshot ),
   m_formation ( formation ),
   m_grid ( grid ),
   m_numberK ( nk )
{
}


