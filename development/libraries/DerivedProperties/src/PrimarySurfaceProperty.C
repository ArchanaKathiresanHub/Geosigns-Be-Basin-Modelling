#include "PrimarySurfaceProperty.h"

#include "Interface/Property.h"
#include "Interface/Snapshot.h"
#include "Interface/Surface.h"
#include "Interface/GridMap.h"
#include "Interface/Grid.h"

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

   m_gridMap->restoreData ( false, true );
}


double DerivedProperties::PrimarySurfaceProperty::get ( unsigned int i,
                                                        unsigned int j ) const {
   return m_gridMap->getValue ( i, j );
}

double DerivedProperties::PrimarySurfaceProperty::getUndefinedValue ( ) const {

   return m_gridMap->getUndefinedValue ( );
}

