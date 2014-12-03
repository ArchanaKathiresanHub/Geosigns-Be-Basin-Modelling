#include "PropertyCalculator.h"

DerivedProperties::PropertyCalculator::PropertyCalculator () {
}

DerivedProperties::PropertyCalculator::~PropertyCalculator () {
}
      
void DerivedProperties::PropertyCalculator::addPropertyName ( const std::string& propertyName ) {

   if ( std::find ( m_propertyNames.begin (), m_propertyNames.end (), propertyName ) == m_propertyNames.end ()) {
      m_propertyNames.push_back ( propertyName );
   }

}

const std::vector<std::string>& DerivedProperties::PropertyCalculator::getPropertyNames () const {
   return m_propertyNames;
}

