//
// Copyright (C) 2016-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "PrimarySurfaceProperty.h"

#include "Property.h"
#include "Snapshot.h"
#include "Surface.h"
#include "GridMap.h"
#include "Grid.h"

using namespace AbstractDerivedProperties;

DerivedProperties::PrimarySurfaceProperty::PrimarySurfaceProperty ( const DataAccess::Interface::PropertyValue* propertyValue ) :
   SurfaceProperty ( propertyValue->getProperty (),
                     propertyValue->getSnapshot (),
                     propertyValue->getSurface (),
                     propertyValue->getGridMap ()->getGrid ()),
   m_gridMap ( propertyValue->getGridMap () )
{
   m_gridMap->retrieveData ( true );
}

DerivedProperties::PrimarySurfaceProperty::~PrimarySurfaceProperty() {
   // Should check neither m_propertyValue nor m_gridMap at this point because the project handle may have deleted them already.
}


double DerivedProperties::PrimarySurfaceProperty::get ( unsigned int i,
                                                        unsigned int j ) const {
   return m_gridMap->getValue ( i, j );
}

double DerivedProperties::PrimarySurfaceProperty::getUndefinedValue ( ) const {

   return m_gridMap->getUndefinedValue ( );
}


const DataAccess::Interface::GridMap* DerivedProperties::PrimarySurfaceProperty::getGridMap() const {

   return m_gridMap;
}

bool DerivedProperties::PrimarySurfaceProperty::isRetrieved () const {
   return m_gridMap->retrieved ();
}

void DerivedProperties::PrimarySurfaceProperty::retrieveData() const {
   m_gridMap->retrieveData();
}

void DerivedProperties::PrimarySurfaceProperty::restoreData() const {
   m_gridMap->restoreData();
}

double DerivedProperties::PrimarySurfaceProperty::getAverageValue() const {
   return m_gridMap->getAverageValue();
}
