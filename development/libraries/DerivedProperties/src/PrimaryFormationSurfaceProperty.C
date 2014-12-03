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
   m_propertyValue ( propertyValue ),
   m_gridMap ( propertyValue->getGridMap ())
{
   m_gridMap->retrieveData ( true );
}

DerivedProperties::PrimaryFormationSurfaceProperty::~PrimaryFormationSurfaceProperty() {

   if ( m_propertyValue->hasGridMap ()) {
      m_gridMap->restoreData ( false, true );
   }

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

