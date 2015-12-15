#include "ReservoirProperty.h"

DerivedProperties::ReservoirProperty::ReservoirProperty ( const DataModel::AbstractProperty*  property,
                                                          const DataModel::AbstractSnapshot*  snapshot,
                                                          const DataModel::AbstractReservoir* reservoir,
                                                          const DataModel::AbstractGrid*      grid ) :
   FormationMapProperty ( property, snapshot, reservoir->getFormation (), grid ),
   m_reservoir ( reservoir )
{
   // Should check that they aint null.
}
