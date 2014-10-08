#include "Interface/Interface.h"
#include "DerivedFormationProperty.h"

DerivedProperties::DerivedFormationProperty::DerivedFormationProperty ( const DataModel::AbstractProperty*  property,
                                                                        const DataModel::AbstractSnapshot*  snapshot,
                                                                        const DataModel::AbstractFormation* formation,
                                                                        const DataModel::AbstractGrid*      grid,
                                                                        const unsigned int                  nk ) :
   FormationProperty ( property, snapshot, formation, grid, nk ),
   m_values ( boost::extents [grid->lastI(true) - grid->firstI(true) + 1][grid->lastJ(true) - grid->firstJ(true) + 1][ nk ])
{
}

void DerivedProperties::DerivedFormationProperty::set ( unsigned int i,
                                                        unsigned int j,
                                                        unsigned int k,
                                                        double   value ) {
   m_values [ i - firstI(true)][ j - firstJ(true)][ k - firstK ()] = value;
}

double DerivedProperties::DerivedFormationProperty::getUndefinedValue () const {

   return DataAccess::Interface::DefaultUndefinedMapValue;
}
