#include "PrimaryFormationProperty.h"

#include "Interface/Property.h"
#include "Interface/Snapshot.h"
#include "Interface/Formation.h"
#include "Interface/GridMap.h"
#include "Interface/Grid.h"

DerivedProperties::PrimaryFormationProperty::PrimaryFormationProperty ( const DataAccess::Interface::PropertyValue* propertyValue ) : 
   FormationProperty ( propertyValue->getProperty (), 
                       propertyValue->getSnapshot (),
                       propertyValue->getFormation (),
                       propertyValue->getGridMap ()->getGrid (),
                       propertyValue->getGridMap ()->lastK () + 1 ),
   m_gridMap ( propertyValue->getGridMap ())
{
}

double DerivedProperties::PrimaryFormationProperty::get ( unsigned int i,
                                                          unsigned int j,
                                                          unsigned int k ) const {
   return m_gridMap->getValue ( i, j, k );
}

