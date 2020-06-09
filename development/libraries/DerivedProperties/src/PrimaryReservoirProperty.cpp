//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "PrimaryReservoirProperty.h"

#include "Property.h"
#include "Snapshot.h"
#include "Reservoir.h"
#include "GridMap.h"
#include "Grid.h"

using namespace AbstractDerivedProperties;

DerivedProperties::PrimaryReservoirProperty::PrimaryReservoirProperty ( const DataAccess::Interface::PropertyValue* propertyValue ) :
   ReservoirProperty ( propertyValue->getProperty (),
                       propertyValue->getSnapshot (),
                       propertyValue->getReservoir (),
                       propertyValue->getGridMap ()->getGrid ()),
   m_gridMap ( propertyValue->getGridMap ())
{
   m_gridMap->retrieveData ( true );
}

DerivedProperties::PrimaryReservoirProperty::~PrimaryReservoirProperty() {
   // Should check neither m_propertyValue nor m_gridMap at this point because the project handle may have deleted them already.
}

double DerivedProperties::PrimaryReservoirProperty::get ( unsigned int i,
                                                          unsigned int j ) const {
   return m_gridMap->getValue ( i, j );
}

double DerivedProperties::PrimaryReservoirProperty::getUndefinedValue ( ) const {
   return m_gridMap->getUndefinedValue ( );
}

bool DerivedProperties::PrimaryReservoirProperty::isRetrieved () const {
   return m_gridMap->retrieved ();
}

void DerivedProperties::PrimaryReservoirProperty::retrieveData() const {
   m_gridMap->retrieveData();
}

void DerivedProperties::PrimaryReservoirProperty::restoreData() const {
   m_gridMap->restoreData();
}

