#include "PrimaryProperty.h"

DerivedProperties::PrimaryProperty::PrimaryProperty ( const DataAccess::Interface::GridMap* aMap ) :
   m_gridMap ( aMap ) {

}

bool DerivedProperties::PrimaryProperty::retrieveData ( bool withGhosts ) const {
   return m_gridMap->retrieveData ( withGhosts );
}

bool DerivedProperties::PrimaryProperty::restoreData ( bool save, bool withGhosts ) const {
   return m_gridMap->restoreData ( save, withGhosts );
}

double DerivedProperties::PrimaryProperty::getUndefinedValue () const {
   return m_gridMap->getUndefinedValue ( );
}
