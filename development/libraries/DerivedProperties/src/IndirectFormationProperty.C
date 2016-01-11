#include "IndirectFormationProperty.h"
#include "Interface/Interface.h"

DerivedProperties::IndirectFormationProperty::IndirectFormationProperty(const DataModel::AbstractProperty * property,
                                                                              FormationPropertyPtr          propertyValues) :
   FormationProperty( property,
                      propertyValues->getSnapshot(),
                      propertyValues->getFormation(),
                      propertyValues->getGrid(),
                      propertyValues->lengthK() ),
   m_property(propertyValues)
{
}


double DerivedProperties::IndirectFormationProperty::getUndefinedValue () const
{
   return DataAccess::Interface::DefaultUndefinedMapValue;
}