//
// Copyright (C) 2016-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "PrimaryFormationMapProperty.h"

#include "Property.h"
#include "Snapshot.h"
#include "Formation.h"
#include "Reservoir.h"
#include "GridMap.h"
#include "Grid.h"

using namespace AbstractDerivedProperties;

DerivedProperties::PrimaryFormationMapProperty::PrimaryFormationMapProperty ( const DataAccess::Interface::PropertyValue* propertyValue ) :
   FormationMapProperty ( propertyValue->getProperty (),
                          propertyValue->getSnapshot (),
                          propertyValue->getFormation (),
                          propertyValue->getGridMap ()->getGrid () ),
   m_gridMap ( propertyValue->getGridMap ())
{
   m_gridMap->retrieveData ( true );
}

DerivedProperties::PrimaryFormationMapProperty::~PrimaryFormationMapProperty() {
   // Should check neither m_propertyValue nor m_gridMap at this point because the project handle may have deleted them already.
}


double DerivedProperties::PrimaryFormationMapProperty::get ( unsigned int i,
                                                             unsigned int j ) const {
   return m_gridMap->getValue ( i, j );
}

double DerivedProperties::PrimaryFormationMapProperty::getUndefinedValue ( ) const {
   return m_gridMap->getUndefinedValue ( );
}

bool DerivedProperties::PrimaryFormationMapProperty::isRetrieved () const {
   return m_gridMap->retrieved ();
}

void DerivedProperties::PrimaryFormationMapProperty::retrieveData() const {
   m_gridMap->retrieveData();
}

void DerivedProperties::PrimaryFormationMapProperty::restoreData() const {
   m_gridMap->restoreData();
}

