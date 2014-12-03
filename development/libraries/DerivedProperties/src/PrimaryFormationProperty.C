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
   m_propertyValue ( propertyValue ),
   m_gridMap ( propertyValue->getGridMap () )
{
   m_gridMap->retrieveData ( true );
}

DerivedProperties::PrimaryFormationProperty::~PrimaryFormationProperty() {

   if ( m_propertyValue->hasGridMap ()) {
      m_gridMap->restoreData ( false, true );
   }

}


double DerivedProperties::PrimaryFormationProperty::get ( unsigned int i,
                                                          unsigned int j,
                                                          unsigned int k ) const {
   return m_gridMap->getValue ( i, j, k );
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

