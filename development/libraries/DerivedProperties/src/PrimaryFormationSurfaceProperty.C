#include "PrimaryFormationSurfaceProperty.h"

#include "Interface/Property.h"
#include "Interface/Snapshot.h"
#include "Interface/Surface.h"
#include "Interface/Formation.h"
#include "Interface/GridMap.h"
#include "Interface/Grid.h"

DerivedProperties::PrimaryFormationSurfaceProperty::PrimaryFormationSurfaceProperty ( const DataAccess::Interface::PropertyValue* propertyValue ) : 
   FormationSurfaceProperty ( propertyValue->getProperty (), 
                              propertyValue->getSnapshot (),
                              propertyValue->getFormation (),
                              propertyValue->getSurface (),
                              propertyValue->getGridMap ()->getGrid () ),
   m_propertyValue ( propertyValue ),
   m_gridMap ( propertyValue->getGridMap ())
{
   m_gridMap->retrieveData ( true );
}

DerivedProperties::PrimaryFormationSurfaceProperty::~PrimaryFormationSurfaceProperty() {
   // Should check neither m_propertyValue nor m_gridMap at this point because the project handle may have deleted them already.
}

double DerivedProperties::PrimaryFormationSurfaceProperty::get ( unsigned int i,
                                                                 unsigned int j ) const {
   return m_gridMap->getValue ( i, j );
}


double DerivedProperties::PrimaryFormationSurfaceProperty::getUndefinedValue ( ) const {

   return m_gridMap->getUndefinedValue ( );
}

bool DerivedProperties::PrimaryFormationSurfaceProperty::isRetrieved () const {
   return m_gridMap->retrieved ();
}

void DerivedProperties::PrimaryFormationSurfaceProperty::retrieveData() const {
   m_gridMap->retrieveData();
}

void DerivedProperties::PrimaryFormationSurfaceProperty::restoreData() const {
   m_gridMap->restoreData();
}

const DataAccess::Interface::GridMap* DerivedProperties::PrimaryFormationSurfaceProperty::getGridMap() const {

   return m_gridMap;
}
