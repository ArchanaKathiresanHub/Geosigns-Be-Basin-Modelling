//
// Copyright (C) 2013-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "PropertyCalculator.h"

#include <algorithm>

void AbstractDerivedProperties::PropertyCalculator::addPropertyName ( const std::string& propertyName ) {

   if ( std::find ( m_propertyNames.begin (), m_propertyNames.end (), propertyName ) == m_propertyNames.end ()) {
      // Only add a property name if it is not already on the list.
      m_propertyNames.push_back ( propertyName );
   }

}

void AbstractDerivedProperties::PropertyCalculator::addDependentPropertyName ( const std::string& propertyName ) {

   if ( std::find ( m_dependentPropertyNames.begin (), m_dependentPropertyNames.end (), propertyName ) == m_dependentPropertyNames.end ()) {
      // Only add a property name if it is not already on the list.
      m_dependentPropertyNames.push_back ( propertyName );
   }

}

const std::vector<std::string>& AbstractDerivedProperties::PropertyCalculator::getPropertyNames () const {
   return m_propertyNames;
}

const std::vector<std::string>& AbstractDerivedProperties::PropertyCalculator::getDependentPropertyNames () const {
   return m_dependentPropertyNames;
}
