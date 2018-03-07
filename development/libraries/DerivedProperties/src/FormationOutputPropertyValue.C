//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "FormationOutputPropertyValue.h"

using namespace AbstractDerivedProperties;

DerivedProperties::FormationOutputPropertyValue::FormationOutputPropertyValue (       AbstractPropertyManager&      propertyManager,
                                                                                const DataModel::AbstractProperty*  property,
                                                                                const DataModel::AbstractSnapshot*  snapshot,
                                                                                const DataModel::AbstractFormation* formation ) :
   OutputPropertyValue ( property ) {

   if ( property != nullptr and snapshot != nullptr and formation != nullptr ) {
      m_formationProperty = propertyManager.getFormationProperty ( property, snapshot, formation );
   }

}

DerivedProperties::FormationOutputPropertyValue::FormationOutputPropertyValue ( FormationPropertyPtr& formationProperty ) :
   OutputPropertyValue ( formationProperty->getProperty ()), m_formationProperty ( formationProperty ) {
}

void DerivedProperties::FormationOutputPropertyValue::retrieveData () const {

   if ( m_formationProperty != nullptr ) {
      m_formationProperty->retrieveData();
   }
}

const string & DerivedProperties::FormationOutputPropertyValue::getName() const { 

   if( m_formationProperty != nullptr ) {
      return  m_formationProperty->getProperty()->getName(); 
   } else {
      return s_nullString;
   }
}

void DerivedProperties::FormationOutputPropertyValue::restoreData () const {

   if ( m_formationProperty != nullptr ) {
      m_formationProperty->restoreData();
   }
}


double DerivedProperties::FormationOutputPropertyValue::getValue ( const double i, const double j, const double k ) const {

   if ( m_formationProperty != nullptr ) {
      return m_formationProperty->interpolate ( i, j, k );
   } else {
      return DataAccess::Interface::DefaultUndefinedMapValue;
   }

}

unsigned int DerivedProperties::FormationOutputPropertyValue::getDepth () const {

   if ( m_formationProperty != nullptr ) {
      return m_formationProperty->lengthK ();
   } else {
      return 0;
   }

}

bool DerivedProperties::FormationOutputPropertyValue::isPrimary() const {
   if ( m_formationProperty != nullptr ) {
      return m_formationProperty->isPrimary ();
   } else {
      return false;
   }
}

const DataModel::AbstractGrid* DerivedProperties::FormationOutputPropertyValue::getGrid () const {

   if( m_formationProperty != nullptr ) {
      return m_formationProperty->getGrid();
   }
   return false;
}

bool DerivedProperties::FormationOutputPropertyValue::hasProperty() const {
   return m_formationProperty != nullptr;
}