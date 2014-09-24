#include "Property.h"


bool DerivedProperties::Property::retrieveData ( bool withGhosts ) const {
   return true;
}

bool DerivedProperties::Property::restoreData ( bool save, bool withGhosts ) const {
   return true;
}

double DerivedProperties::Property::getUndefinedValue () const {
   return DataAccess::Interface::DefaultUndefinedMapValue;
}

  
