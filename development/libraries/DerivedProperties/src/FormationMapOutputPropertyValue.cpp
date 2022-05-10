//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "FormationMapOutputPropertyValue.h"

using namespace AbstractDerivedProperties;

DerivedProperties::FormationMapOutputPropertyValue::FormationMapOutputPropertyValue ( AbstractPropertyManager&            propertyManager,
                                                                                      const DataModel::AbstractProperty*  property,
                                                                                      const DataModel::AbstractSnapshot*  snapshot,
                                                                                      const DataModel::AbstractFormation* formation ) : OutputPropertyValue ( property ) {

   if ( property != nullptr and snapshot != nullptr and formation != nullptr ) {
      m_formationMapProperty = propertyManager.getFormationMapProperty ( property, snapshot, formation );
   }

}

double DerivedProperties::FormationMapOutputPropertyValue::getValue ( const double i, const double j, const double k ) const 
{
   if ( m_formationMapProperty != nullptr ) 
   {
       auto value = m_formationMapProperty->interpolate(i, j);
       auto isNaN = OutputPropertyValue::checkForNANPropertyValue(value);
       value = isNaN ? DataAccess::Interface::DefaultUndefinedMapValue : value;
       return value;
   } 
   else 
   {
       return DataAccess::Interface::DefaultUndefinedMapValue;
   }
}

void DerivedProperties::FormationMapOutputPropertyValue::restoreData () const {
  if ( m_formationMapProperty != nullptr ) {
     m_formationMapProperty->restoreData();
  }
}

void DerivedProperties::FormationMapOutputPropertyValue::retrieveData () const {
  if ( m_formationMapProperty != nullptr ) {
     m_formationMapProperty->retrieveData();
  }
}

unsigned int DerivedProperties::FormationMapOutputPropertyValue::getDepth () const {

   if ( m_formationMapProperty != nullptr ) {
      return 1;
   } else {
      return 0;
   }
}

const std::string & DerivedProperties::FormationMapOutputPropertyValue::getName() const {

   if ( m_formationMapProperty != nullptr ) {
      return m_formationMapProperty->getProperty()->getName();
   } else {
      return s_nullString;
   }
}

const DataModel::AbstractGrid* DerivedProperties::FormationMapOutputPropertyValue::getGrid () const {

   if( m_formationMapProperty != nullptr ) {
      return m_formationMapProperty->getGrid();
   }
   return nullptr;
}

bool DerivedProperties::FormationMapOutputPropertyValue::isPrimary() const {

   if( m_formationMapProperty != nullptr ) {
      return m_formationMapProperty->isPrimary();
   } else {
      return false;
   }
}

bool DerivedProperties::FormationMapOutputPropertyValue::hasProperty() const {
   return m_formationMapProperty != nullptr;
}
