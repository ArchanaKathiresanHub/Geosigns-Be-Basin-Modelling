#include "PrimaryFormationMapProperty.h"

#include "Interface/Property.h"
#include "Interface/Snapshot.h"
#include "Interface/Formation.h"
#include "Interface/GridMap.h"
#include "Interface/Grid.h"

DerivedProperties::PrimaryFormationMapProperty::PrimaryFormationMapProperty ( const DataAccess::Interface::PropertyValue* propertyValue ) : 
   FormationMapProperty ( propertyValue->getProperty (), 
                          propertyValue->getSnapshot (),
                          propertyValue->getFormation (),
                          propertyValue->getGridMap ()->getGrid ()),
   m_gridMap ( propertyValue->getGridMap ())
{
}

double DerivedProperties::PrimaryFormationMapProperty::get ( unsigned int i,
                                                             unsigned int j ) const {
   return m_gridMap->getValue ( i, j );
}

