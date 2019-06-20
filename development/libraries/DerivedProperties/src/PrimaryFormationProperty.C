//
// Copyright (C) 2016-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "PrimaryFormationProperty.h"

#include "Property.h"
#include "Snapshot.h"
#include "Formation.h"
#include "GridMap.h"
#include "Grid.h"

using namespace AbstractDerivedProperties;

DerivedProperties::PrimaryFormationProperty::PrimaryFormationProperty ( const DataAccess::Interface::PropertyValue* propertyValue ) : 
   FormationProperty ( propertyValue->getProperty (), 
                       propertyValue->getSnapshot (),
                       propertyValue->getFormation (),
                       propertyValue->getGridMap ()->getGrid(),
                       propertyValue->getGridMap ()->lastK () + 1 ),
   m_propertyValue ( propertyValue ),
   m_gridMap ( propertyValue->getGridMap () )
{
   m_depth = 0;
   m_gridMap->retrieveData ( true );
   m_depth = m_gridMap->getDepth();
   m_storedInAscending = m_gridMap->isAscendingOrder();
}

DerivedProperties::PrimaryFormationProperty::~PrimaryFormationProperty() {
   // Should check neither m_propertyValue nor m_gridMap at this point because the project handle may have deleted them already.
}


double DerivedProperties::PrimaryFormationProperty::get ( unsigned int i,
                                                          unsigned int j,
                                                          unsigned int k ) const {
   return  m_gridMap->getValue (i, j, ( m_storedInAscending ? k : m_depth - 1 - k ));
}

double DerivedProperties::PrimaryFormationProperty::getUndefinedValue ( ) const {

   return m_gridMap->getUndefinedValue ( );
}

const DataAccess::Interface::GridMap* DerivedProperties::PrimaryFormationProperty::getGridMap() const {

   return m_gridMap;
}

bool DerivedProperties::PrimaryFormationProperty::isRetrieved () const {
   return m_gridMap->retrieved ();
}

void DerivedProperties::PrimaryFormationProperty::retrieveData() const {
   m_gridMap->retrieveData();
}

void DerivedProperties::PrimaryFormationProperty::restoreData() const {
   m_gridMap->restoreData();
}

