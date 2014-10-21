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
   m_propertyValue ( propertyValue ),
   m_gridMap ( propertyValue->getGridMap ())
{
   m_gridMap->retrieveData ( true );
}

DerivedProperties::PrimaryFormationMapProperty::~PrimaryFormationMapProperty() {

   if ( m_propertyValue->hasGridMap ()) {
      m_gridMap->restoreData ( false, true );
   }

}


double DerivedProperties::PrimaryFormationMapProperty::get ( unsigned int i,
                                                             unsigned int j ) const {
   return m_gridMap->getValue ( i, j );
}


double DerivedProperties::PrimaryFormationMapProperty::getUndefinedValue ( ) const {

   return m_gridMap->getUndefinedValue ( );
}
