#include "PropertyCalculator.h"

DerivedProperties::PropertyCalculator::PropertyCalculator () {
}

DerivedProperties::PropertyCalculator::~PropertyCalculator () {
}
      
void DerivedProperties::PropertyCalculator::addPropertyName ( const std::string& propertyName ) {

   if ( std::find ( m_propertyNames.begin (), m_propertyNames.end (), propertyName ) == m_propertyNames.end ()) {
      // Only add a property name if it is not already on the list.
      m_propertyNames.push_back ( propertyName );
   }

}

void DerivedProperties::PropertyCalculator::addDependentPropertyName ( const std::string& propertyName ) {

   if ( std::find ( m_dependentPropertyNames.begin (), m_dependentPropertyNames.end (), propertyName ) == m_dependentPropertyNames.end ()) {
      // Only add a property name if it is not already on the list.
      m_dependentPropertyNames.push_back ( propertyName );
   }

}

const std::vector<std::string>& DerivedProperties::PropertyCalculator::getPropertyNames () const {
   return m_propertyNames;
}

const std::vector<std::string>& DerivedProperties::PropertyCalculator::getDependentPropertyNames () const {
   return m_dependentPropertyNames;
}
