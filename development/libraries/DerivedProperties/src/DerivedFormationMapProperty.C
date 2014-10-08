#include "DerivedFormationMapProperty.h"

DerivedProperties::DerivedFormationMapProperty::DerivedFormationMapProperty ( const DataModel::AbstractProperty*  property,
                                                                              const DataModel::AbstractSnapshot*  snapshot,
                                                                              const DataModel::AbstractFormation* formation,
                                                                              const DataModel::AbstractGrid*      grid ) :
   FormationMapProperty  ( property, snapshot, formation, grid ),
   m_values ( boost::extents [grid->lastI(true) - grid->firstI(true) + 1][grid->lastJ(true) - grid->firstJ(true) + 1])
{
}

double DerivedProperties::DerivedFormationMapProperty::get ( unsigned int i,
                                                             unsigned int j ) const {
   return m_values[ i - firstI(true)][ j - firstJ(true)];
}

void DerivedProperties::DerivedFormationMapProperty::set ( unsigned int i,
                                                           unsigned int j,
                                                           double   value ) {

   m_values[ i - firstI(true)][ j - firstJ(true)] = value;
}

double DerivedProperties::DerivedFormationMapProperty::getUndefinedValue () const {

   return DataAccess::Interface::DefaultUndefinedMapValue;
}
