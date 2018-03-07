//
// Copyright (C) 2016-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "IndirectFormationProperty.h"
#include "Interface/Interface.h"

using namespace AbstractDerivedProperties;

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

DerivedProperties::IndirectFormationProperty::~IndirectFormationProperty() {}


double DerivedProperties::IndirectFormationProperty::getUndefinedValue () const
{
   return DataAccess::Interface::DefaultUndefinedMapValue;
}