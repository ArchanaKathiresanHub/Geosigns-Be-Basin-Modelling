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
                              propertyValue->getGridMap ()->getGrid ()),
   m_gridMap ( propertyValue->getGridMap ())
{
}

double DerivedProperties::PrimaryFormationSurfaceProperty::get ( unsigned int i,
                                                                 unsigned int j ) const {
   return m_gridMap->getValue ( i, j );
}

